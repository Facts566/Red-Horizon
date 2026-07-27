#include "game.h"
#include "config.h"
#include "player.h"
#include "map.h"
#include "light.h"
#include "raycast.h"
#include "zombie.h"
#include <rlgl.h>
#include <raymath.h>
#include <cmath>
#include <cstdio>

static Texture2D LoadTexRepeat(const char *path)
{
    Texture2D tex = LoadTexture(path);
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);
    SetTextureWrap(tex, TEXTURE_WRAP_REPEAT);
    rlTextureParameters(tex.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
    rlTextureParameters(tex.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);
    return tex;
}

static Texture2D LoadTexPoint(const char *path)
{
    Texture2D tex = LoadTexture(path);
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);
    return tex;
}

static void SpawnZombies(Game &game, const char *path)
{
    game.zombieSpawnCount = LoadZombieSpawns(path, game.zombieSpawns, MAX_ZOMBIE_SPAWNS);
    if (game.zombieSpawnCount > SCENE_MAX_ZOMBIES) game.zombieSpawnCount = SCENE_MAX_ZOMBIES;
    game.scene.zombieCount = game.zombieSpawnCount;
    for (int i = 0; i < game.zombieSpawnCount; i++) {
        float wx = (float)game.zombieSpawns[i].col * TILE_SIZE + TILE_SIZE / 2.0f;
        float wz = (float)game.zombieSpawns[i].row * TILE_SIZE + TILE_SIZE / 2.0f;
        if (game.zombieSpawns[i].isMilitary) {
            InitZombie(game.scene.zombies[i], (Vector3){wx, ZOMBIE_SPAWN_Y, wz},
                       game.milIdle, game.milWalk1, game.milWalk2, game.milDead);
            game.scene.zombies[i].isMilitary = true;
        } else {
            InitZombie(game.scene.zombies[i], (Vector3){wx, ZOMBIE_SPAWN_Y, wz},
                       game.zombiIdle, game.zombiWalk1, game.zombiWalk2, game.zombiDead);
        }
    }
}

static void SpawnBonuses(Game &game, const char *path)
{
    game.bonusCount = LoadBonusSpawns(path, game.bonusSpawns, MAX_BONUSES);
    InitBonuses(game.bonuses, game.bonusSpawns, game.bonusCount, game.medicTex, game.keyTex, TILE_SIZE);
}

static void ProcessShot(Game &game)
{
    bool shotFired = (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_SPACE)) &&
                     game.weapons[game.currentWeapon].fireCooldown <= 0.0f &&
                     !game.weapons[game.currentWeapon].isReloading &&
                     game.weapons[game.currentWeapon].currentAmmo > 0;

    ShootWeapon(game.weapons[game.currentWeapon], game.camera, game.level,
                game.scene.doors, game.scene.doorCount, game.shader, game.wallHoles);

    if (!shotFired) return;

    Vector3 forward = Vector3Normalize(Vector3Subtract(game.camera.target, game.camera.position));

    for (int i = 0; i < game.scene.objectCount; i++) {
        if (!game.scene.objects[i].active || !game.scene.objects[i].destructible) continue;
        Vector3 hitPos, hitNorm;
        if (RayBoxIntersect(game.scene.objects[i].collider, game.camera.position, forward, 100.0f, hitPos, hitNorm)) {
            SpawnBoxParticles(game.scene, hitPos, game.scene.objects[i].texture);
            game.scene.objects[i].active = false;
            game.scene.objects[i].addCollision = false;
            break;
        }
    }

    float closestDist = 1e9f;
    int closestIdx = -1;

    for (int i = 0; i < game.scene.zombieCount; i++) {
        if (!game.scene.zombies[i].active || game.scene.zombies[i].health <= 0.0f) continue;
        if (!ZombieHitByRay(game.scene.zombies[i], game.camera.position, forward)) continue;
        float dx = game.scene.zombies[i].position.x - game.camera.position.x;
        float dz = game.scene.zombies[i].position.z - game.camera.position.z;
        float dist = dx * dx + dz * dz;
        if (dist < closestDist) {
            closestDist = dist;
            closestIdx = i;
        }
    }

    if (closestIdx >= 0) {
        Vector3 hitPos, hitNorm;
        bool blocked = RaycastWall(game.level, game.camera.position, forward, sqrtf(closestDist), hitPos, hitNorm);
        if (!blocked) {
            float damage = game.currentWeapon == 2 ? 100.0f : (game.currentWeapon == 1 ? 50.0f : 25.0f);
            game.scene.zombies[closestIdx].health -= damage;
            game.scene.zombies[closestIdx].hitTime = ZOMBIE_HIT_TIME;
        }
    }
}

static void ProcessZombieTouchDamage(Game &game, float dt)
{
    bool touching = false;
    for (int i = 0; i < game.scene.zombieCount; i++) {
        if (!game.scene.zombies[i].active || game.scene.zombies[i].health <= 0.0f) continue;
        float dx = game.camera.position.x - game.scene.zombies[i].position.x;
        float dz = game.camera.position.z - game.scene.zombies[i].position.z;
        float touchDist = PLAYER_RADIUS + game.scene.zombies[i].radius * 2;
        if (dx * dx + dz * dz < touchDist * touchDist)
            touching = true;
    }

    if (touching) {
        game.touchTimer += dt;
        while (game.touchTimer >= TOUCH_INTERVAL) {
            game.health -= TOUCH_DAMAGE;
            game.touchTimer -= TOUCH_INTERVAL;
            game.hitFlash = HIT_FLASH_DURATION;
            game.hitShakeTime = HIT_SHAKE_DURATION;
        }
    } else {
        game.touchTimer = 0.0f;
    }

    for (int i = 0; i < game.scene.zombieCount; i++) {
        if (game.scene.zombies[i].wantsToShoot) {
            game.health -= TOUCH_DAMAGE;
            game.scene.zombies[i].wantsToShoot = false;
            game.hitFlash = HIT_FLASH_DURATION;
            game.hitShakeTime = HIT_SHAKE_DURATION;
        }
    }

    if (game.health < 0) game.health = 0;
    if (game.health <= 0.0f) game.gameOver = true;
}

static void UpdateLighting(Game &game)
{
    SetShaderValue(game.shader, game.lightRangeLoc, &LIGHT_RANGE, SHADER_UNIFORM_FLOAT);
    float ambient = LIGHT_AMBIENT;
    SetShaderValue(game.shader, game.lightAmbLoc, &ambient, SHADER_UNIFORM_FLOAT);
    SetShaderValue(game.shader, game.lightPosLoc, &game.camera.position, SHADER_UNIFORM_VEC3);

    int lampIdx = 0;
    for (int i = 0; i < game.scene.objectCount; i++) {
        if (!game.scene.objects[i].isLamp) continue;
        Vector3 pos = game.scene.objects[i].position;
        pos.y -= LAMP_Y_OFFSET;
        Vector3 color = {1.0f, 0.9f, 0.7f};
        float lr = LAMP_RANGE;
        char buf[32];

        snprintf(buf, sizeof(buf), "lampPos[%d]", lampIdx);
        SetShaderValue(game.shader, GetShaderLocation(game.shader, buf), &pos, SHADER_UNIFORM_VEC3);
        snprintf(buf, sizeof(buf), "lampColor[%d]", lampIdx);
        SetShaderValue(game.shader, GetShaderLocation(game.shader, buf), &color, SHADER_UNIFORM_VEC3);
        snprintf(buf, sizeof(buf), "lampRange[%d]", lampIdx);
        SetShaderValue(game.shader, GetShaderLocation(game.shader, buf), &lr, SHADER_UNIFORM_FLOAT);

        lampIdx++;
    }
}

static Camera3D ApplyShake(Game &game, Camera3D cam)
{
    Vector3 shakeOffset = {0};

    if (game.weapons[game.currentWeapon].shakeTime > 0.0f) {
        WeaponState &w = game.weapons[game.currentWeapon];
        float t = w.shakeTime / w.shakeDuration;
        float intensity = w.shakeAmount * t;
        shakeOffset.y = intensity;
        shakeOffset.x = ((float)GetRandomValue(0, 1000) / 500.0f - 1.0f) * intensity * 0.2f;
        shakeOffset.z = ((float)GetRandomValue(0, 1000) / 500.0f - 1.0f) * intensity * 0.2f;
        w.shakeTime -= GetFrameTime();
    }

    if (game.hitShakeTime > 0.0f) {
        float intensity = 0.3f;
        shakeOffset.x += ((float)GetRandomValue(0, 1000) / 500.0f - 1.0f) * intensity;
        shakeOffset.y += ((float)GetRandomValue(0, 1000) / 500.0f - 1.0f) * intensity;
        game.hitShakeTime -= GetFrameTime();
    }

    cam.position.x += shakeOffset.x;
    cam.position.y += shakeOffset.y;
    cam.position.z += shakeOffset.z;
    cam.target.x += shakeOffset.x;
    cam.target.y += shakeOffset.y;
    cam.target.z += shakeOffset.z;
    return cam;
}

static void DrawHitFlash(Game &game)
{
    if (game.hitFlash <= 0.0f) return;
    int alpha = (int)(game.hitFlash / HIT_FLASH_DURATION * 100.0f);
    if (alpha > 100) alpha = 100;
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){180, 0, 0, (unsigned char)alpha});
    game.hitFlash -= GetFrameTime();
    if (game.hitFlash < 0.0f) game.hitFlash = 0.0f;
}

void DrawMenu(Game &game)
{
    BeginDrawing();
    ClearBackground(BLACK);

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    const char *title = "RED HORIZON";
    int titleSize = 80;
    int titleW = MeasureText(title, titleSize);
    DrawText(title, sw/2 - titleW/2, sh/2 - 120, titleSize, RED);

    Rectangle playBtn = {(float)sw/2 - 120, (float)sh/2, 240, 60};
    Rectangle exitBtn = {(float)sw/2 - 120, (float)sh/2 + 80, 240, 60};

    Vector2 mouse = GetMousePosition();
    bool playHover = CheckCollisionPointRec(mouse, playBtn);
    bool exitHover = CheckCollisionPointRec(mouse, exitBtn);

    Color playColor = playHover ? DARKBLUE : DARKGRAY;
    Color exitColor = exitHover ? (Color){150, 30, 30, 255} : DARKGRAY;

    DrawRectangleRec(playBtn, playColor);
    DrawRectangleLinesEx(playBtn, 2, playHover ? YELLOW : WHITE);
    const char *playText = "PLAY";
    int playTextW = MeasureText(playText, 30);
    DrawText(playText, sw/2 - playTextW/2, sh/2 + 18, 30, WHITE);

    DrawRectangleRec(exitBtn, exitColor);
    DrawRectangleLinesEx(exitBtn, 2, exitHover ? YELLOW : WHITE);
    const char *exitText = "EXIT";
    int exitTextW = MeasureText(exitText, 30);
    DrawText(exitText, sw/2 - exitTextW/2, sh/2 + 98, 30, WHITE);

    if (playHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        EnableCursor();
        game.state = GAME_LEVEL_SELECT;
    }

    if (exitHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        game.exitGame = true;
    }

    EndDrawing();
}

void DrawLevelSelect(Game &game)
{
    BeginDrawing();
    ClearBackground(BLACK);

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    const char *title = "SELECT LEVEL";
    int titleSize = 60;
    int titleW = MeasureText(title, titleSize);
    DrawText(title, sw/2 - titleW/2, sh/2 - 200, titleSize, RED);

    int levelCount = 0;
    bool levelExists[8] = {};
    for (int i = 1; i <= 8; i++) {
        char path[64];
        snprintf(path, sizeof(path), "map/level_%d/map.txt", i);
        FILE *f = fopen(path, "r");
        if (f) {
            fclose(f);
            levelExists[i - 1] = true;
            levelCount++;
        }
    }

    Vector2 mouse = GetMousePosition();
    for (int i = 0; i < levelCount; i++) {
        Rectangle btn = {(float)sw/2 - 120, (float)sh/2 - 80 + i * 70, 240, 55};
        bool hover = CheckCollisionPointRec(mouse, btn);
        Color btnColor = hover ? DARKBLUE : DARKGRAY;
        DrawRectangleRec(btn, btnColor);
        DrawRectangleLinesEx(btn, 2, hover ? YELLOW : WHITE);
        char label[32];
        snprintf(label, sizeof(label), "Level %d", i + 1);
        int labelW = MeasureText(label, 30);
        DrawText(label, sw/2 - labelW/2, (int)btn.y + 16, 30, WHITE);

        if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            game.currentLevel = i + 1;
            game.state = GAME_PLAYING;
        }
    }

    Rectangle backBtn = {(float)sw/2 - 120, (float)sh/2 - 80 + levelCount * 70 + 20, 240, 55};
    bool backHover = CheckCollisionPointRec(mouse, backBtn);
    Color backColor = backHover ? (Color){150, 30, 30, 255} : DARKGRAY;
    DrawRectangleRec(backBtn, backColor);
    DrawRectangleLinesEx(backBtn, 2, backHover ? YELLOW : WHITE);
    const char *backText = "BACK";
    int backTextW = MeasureText(backText, 30);
    DrawText(backText, sw/2 - backTextW/2, (int)backBtn.y + 16, 30, WHITE);

    if (backHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        game.state = GAME_MENU;
    }

    EndDrawing();
}

void InitGame(Game &game)
{
    game.floorTex     = LoadTexRepeat("tex/map/floor.png");
    game.wallTex      = LoadTexRepeat("tex/map/bricks.png");
    game.greenTex     = LoadTexRepeat("tex/map/green_wall.png");
    game.planksTex    = LoadTexRepeat("tex/map/planks.png");
    game.whiteWallTex = LoadTexRepeat("tex/map/white_wall.png");

    game.zombiIdle  = LoadTexPoint("tex/zombi/zombi.png");
    game.zombiWalk1 = LoadTexPoint("tex/zombi/zombi_walk.png");
    game.zombiWalk2 = LoadTexPoint("tex/zombi/zombi_walk_1.png");
    game.zombiDead  = LoadTexPoint("tex/zombi/zombi_kill.png");

    game.milIdle  = LoadTexPoint("tex/zombie_military/zombie_military.png");
    game.milWalk1 = LoadTexPoint("tex/zombie_military/zombie_military_walk.png");
    game.milWalk2 = LoadTexPoint("tex/zombie_military/zombie_military_walk_1.png");
    game.milDead  = LoadTexPoint("tex/zombie_military/zombie_military_kill.png");

    game.shotholeTex = LoadTexPoint("tex/weapons/shothole.png");
    game.medicTex    = LoadTexPoint("tex/bonus/medic.png");
    game.keyTex      = LoadTexPoint("tex/bonus/key.png");

    game.shader = LoadLightShader();
    game.currentLevel = 1;

    LoadPistol(game.weapons[0], game.shader, game.shotholeTex);
    LoadWeapon(game.weapons[1], game.shader, game.shotholeTex, "tex/weapons/gun.png");
    LoadDoubleBarreledShotgun(game.weapons[2], game.shader, game.shotholeTex);
    for (int i = 0; i < WEAPON_COUNT; i++)
        game.weapons[i].unlocked = true;
    game.currentWeapon = 0;

    game.camera = { 0 };
    game.camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    game.camera.fovy = CAMERA_FOVY;
    game.camera.projection = CAMERA_PERSPECTIVE;

    game.maxHealth = HEALTH_MAX;
    game.health = (float)HEALTH_MAX;
    game.yaw = 0.0f;
    game.hitFlash = 0.0f;
    game.hitShakeTime = 0.0f;
    game.touchTimer = 0.0f;
    game.gameOver = false;
    game.showWeaponPanel = false;
    game.hasKey = false;
    game.transPhase = TRANS_NONE;
    game.transTimer = 0.0f;
    game.transNextLevel = 0;

    InitZombieModel(game.shader);

    SetLightUniforms(game.shader, game.camera.position, {1,1,1}, LIGHT_RANGE, LIGHT_AMBIENT);
    game.lightRangeLoc = GetShaderLocation(game.shader, "lightRange");
    game.lightAmbLoc   = GetShaderLocation(game.shader, "ambientStrength");
    game.lightPosLoc   = GetShaderLocation(game.shader, "lightPosition");

    rlDisableBackfaceCulling();
}

void LoadLevelByIndex(Game &game, int levelIndex)
{
    char mapPath[64];
    char enemyPath[64];
    snprintf(mapPath, sizeof(mapPath), "map/level_%d/map.txt", levelIndex);
    snprintf(enemyPath, sizeof(enemyPath), "map/level_%d/enemy.txt", levelIndex);

    UnloadLevel(game.level);
    UnloadScene(game.scene);

    game.level = LoadLevel(mapPath, TILE_SIZE, WALL_HEIGHT,
                           game.floorTex, game.planksTex, game.wallTex, game.greenTex, game.whiteWallTex, game.shader);

    game.camera.position = LoadPlayerSpawn(enemyPath, TILE_SIZE, WALL_HEIGHT);
    game.camera.target = (Vector3){game.camera.position.x, game.camera.position.y, game.camera.position.z - 1};
    game.yaw = 0.0f;

    LoadScene(game.scene, game.shader, TILE_SIZE,
              game.greenTex, game.wallTex, game.shotholeTex, game.whiteWallTex, levelIndex);

    SpawnZombies(game, enemyPath);
    SpawnBonuses(game, enemyPath);

    game.wallHoles.clear();
    game.hasKey = false;
    game.currentLevel = levelIndex;

    game.hitFlash = 0.0f;
    game.hitShakeTime = 0.0f;
    game.touchTimer = 0.0f;
    game.gameOver = false;

    for (int i = 0; i < WEAPON_COUNT; i++) {
        game.weapons[i].currentAmmo = game.weapons[i].maxAmmo;
        game.weapons[i].isReloading = false;
        game.weapons[i].reloadTimer = 0.0f;
        game.weapons[i].fireCooldown = 0.0f;
    }
}

void ResetGame(Game &game)
{
    LoadLevelByIndex(game, game.currentLevel);
    game.health = (float)game.maxHealth;
    game.currentWeapon = 0;
    for (int i = 0; i < WEAPON_COUNT; i++)
        game.weapons[i].unlocked = true;
}

constexpr float TRANS_FADE_DURATION = 0.4f;

void UpdateGame(Game &game)
{
    float dt = GetFrameTime();

    if (game.transPhase != TRANS_NONE)
    {
        game.transTimer += dt;
        if (game.transPhase == TRANS_FADE_OUT && game.transTimer >= TRANS_FADE_DURATION)
        {
            LoadLevelByIndex(game, game.transNextLevel);
            game.transPhase = TRANS_FADE_IN;
            game.transTimer = 0.0f;
        }
        else if (game.transPhase == TRANS_FADE_IN && game.transTimer >= TRANS_FADE_DURATION)
        {
            game.transPhase = TRANS_NONE;
            game.transTimer = 0.0f;
        }
        return;
    }

    UpdatePlayer(&game.camera, &game.yaw, game.level, game.scene.doors, game.scene.doorCount, game.scene);

    if (IsKeyPressed(KEY_F))
        game.showWeaponPanel = !game.showWeaponPanel;

    if (game.showWeaponPanel) {
        if (IsKeyPressed(KEY_ONE) && game.weapons[0].unlocked)   { game.currentWeapon = 0; game.showWeaponPanel = false; }
        if (IsKeyPressed(KEY_TWO) && game.weapons[1].unlocked)   { game.currentWeapon = 1; game.showWeaponPanel = false; }
        if (IsKeyPressed(KEY_THREE) && game.weapons[2].unlocked) { game.currentWeapon = 2; game.showWeaponPanel = false; }
        if (IsKeyPressed(KEY_ESCAPE)) game.showWeaponPanel = false;
    }

    UpdateWeapon(game.weapons[game.currentWeapon]);

    bool isMoving = IsKeyDown(KEY_W) || IsKeyDown(KEY_S) || IsKeyDown(KEY_A) || IsKeyDown(KEY_D) ||
                    IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT);
    bool isSprinting = isMoving && IsKeyDown(KEY_LEFT_SHIFT);
    UpdateWeaponBob(game.weapons[game.currentWeapon], isMoving, isSprinting);

    ProcessShot(game);

    Vector3 doorPositions[SCENE_MAX_ZOMBIES + 1];
    int doorPosCount = 0;
    doorPositions[doorPosCount++] = game.camera.position;
    for (int i = 0; i < game.scene.zombieCount; i++) {
        if (game.scene.zombies[i].active)
            doorPositions[doorPosCount++] = game.scene.zombies[i].position;
    }
    UpdateDoors(game.scene.doors, game.scene.doorCount, doorPositions, doorPosCount, game.hasKey);

    if (CheckExitDoorTrigger(game.scene.doors, game.scene.doorCount, game.camera.position))
    {
        char nextMapPath[64];
        snprintf(nextMapPath, sizeof(nextMapPath), "map/level_%d/map.txt", game.currentLevel + 1);
        FILE *f = fopen(nextMapPath, "r");
        if (f)
        {
            fclose(f);
            game.transPhase = TRANS_FADE_OUT;
            game.transTimer = 0.0f;
            game.transNextLevel = game.currentLevel + 1;
        }
        else
        {
            game.state = GAME_MENU;
            EnableCursor();
        }
        return;
    }

    for (int i = 0; i < game.scene.zombieCount; i++)
        UpdateZombie(game.scene.zombies[i], game.level, game.scene.doors, game.scene.doorCount,
                     game.scene, game.camera.position, dt);

    ProcessZombieTouchDamage(game, dt);
    UpdateBonuses(game.bonuses, game.bonusCount, game.camera.position, game.health, game.maxHealth, game.hasKey);
    UpdateParticles(game.scene, dt);
    UpdateLighting(game);
}

void DrawGame(Game &game)
{
    BeginDrawing();
    ClearBackground(BLACK);

    Camera3D shakeCam = ApplyShake(game, game.camera);

    BeginMode3D(shakeCam);
        DrawLevel(game.level);
        DrawScene(game.scene, shakeCam, game.shader, game.bonuses, game.bonusCount);
        DrawParticles(game.scene, shakeCam);
        DrawWeaponDecals(game.wallHoles, game.weapons[game.currentWeapon].decalModel);
    EndMode3D();

    DrawWeaponHUD(game.weapons[game.currentWeapon], (int)game.health, game.maxHealth);

    if (game.showWeaponPanel)
        DrawWeaponPanel(game.weapons, WEAPON_COUNT, game.currentWeapon);

    DrawHitFlash(game);

    if (game.transPhase != TRANS_NONE)
    {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        float t;
        if (game.transPhase == TRANS_FADE_OUT)
            t = game.transTimer / TRANS_FADE_DURATION;
        else
            t = 1.0f - game.transTimer / TRANS_FADE_DURATION;
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        unsigned char alpha = (unsigned char)(t * 255);
        DrawRectangle(0, 0, sw, sh, (Color){0, 0, 0, alpha});
    }

    EndDrawing();
}

void UnloadGame(Game &game)
{
    rlDisableBackfaceCulling();
    EnableCursor();
    UnloadLevel(game.level);
    UnloadScene(game.scene);
    for (int i = 0; i < WEAPON_COUNT; i++)
        UnloadWeapon(game.weapons[i]);
    UnloadShader(game.shader);
    UnloadTexture(game.floorTex);
    UnloadTexture(game.wallTex);
    UnloadTexture(game.greenTex);
    UnloadTexture(game.planksTex);
    UnloadTexture(game.whiteWallTex);
    UnloadTexture(game.zombiIdle);
    UnloadTexture(game.zombiWalk1);
    UnloadTexture(game.zombiWalk2);
    UnloadTexture(game.zombiDead);
    UnloadTexture(game.milIdle);
    UnloadTexture(game.milWalk1);
    UnloadTexture(game.milWalk2);
    UnloadTexture(game.milDead);
    UnloadTexture(game.shotholeTex);
    UnloadTexture(game.medicTex);
    UnloadTexture(game.keyTex);
}
