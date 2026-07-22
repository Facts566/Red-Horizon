#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <cstdio>
#include "player.h"
#include "map.h"
#include "light.h"
#include "level.h"
#include "door.h"
#include "raycast.h"
#include "scene.h"
#include "weapon.h"
#include "zombie.h"

int main()
{
    InitWindow(GetScreenWidth(), GetScreenHeight(), "FactsEngine");

    Texture2D texture = LoadTexture("tex/floor.png");
    SetTextureFilter(texture, TEXTURE_FILTER_POINT);
    SetTextureWrap(texture, TEXTURE_WRAP_REPEAT);
    rlTextureParameters(texture.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
    rlTextureParameters(texture.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);

    Texture2D wallTex = LoadTexture("tex/bricks.png");
    SetTextureFilter(wallTex, TEXTURE_FILTER_POINT);
    SetTextureWrap(wallTex, TEXTURE_WRAP_REPEAT);
    rlTextureParameters(wallTex.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
    rlTextureParameters(wallTex.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);

    Texture2D greenTex = LoadTexture("tex/green_wall.png");
    SetTextureFilter(greenTex, TEXTURE_FILTER_POINT);
    SetTextureWrap(greenTex, TEXTURE_WRAP_REPEAT);
    rlTextureParameters(greenTex.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
    rlTextureParameters(greenTex.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);

    Texture2D planksTex = LoadTexture("tex/planks.png");
    SetTextureFilter(planksTex, TEXTURE_FILTER_POINT);
    SetTextureWrap(planksTex, TEXTURE_WRAP_REPEAT);
    rlTextureParameters(planksTex.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
    rlTextureParameters(planksTex.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);

    Texture2D zombiIdle = LoadTexture("tex/zombi/zombi.png");
    SetTextureFilter(zombiIdle, TEXTURE_FILTER_POINT);
    Texture2D zombiWalk1 = LoadTexture("tex/zombi/zombi_walk.png");
    SetTextureFilter(zombiWalk1, TEXTURE_FILTER_POINT);
    Texture2D zombiWalk2 = LoadTexture("tex/zombi/zombi_walk_1.png");
    SetTextureFilter(zombiWalk2, TEXTURE_FILTER_POINT);
    Texture2D zombiDead = LoadTexture("tex/zombi/zombi_kill.png");
    SetTextureFilter(zombiDead, TEXTURE_FILTER_POINT);

    Texture2D milIdle = LoadTexture("tex/zombie_military/zombie_military.png");
    SetTextureFilter(milIdle, TEXTURE_FILTER_POINT);

    Texture2D shotholeTex = LoadTexture("tex/shothole.png");
    SetTextureFilter(shotholeTex, TEXTURE_FILTER_POINT);

    float tileSize = 5.0f;
    float wallHeight = 20.0f;

    Texture2D whiteWallTex = LoadTexture("tex/white_wall.png");
    SetTextureFilter(whiteWallTex, TEXTURE_FILTER_POINT);
    SetTextureWrap(whiteWallTex, TEXTURE_WRAP_REPEAT);
    rlTextureParameters(whiteWallTex.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
    rlTextureParameters(whiteWallTex.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);

    Shader shader = LoadLightShader();
    Level level = LoadLevel("map/map.txt", tileSize, wallHeight, texture, planksTex, wallTex, greenTex, whiteWallTex, shader);

    WeaponState weapons[WEAPON_COUNT];
    LoadWeapon(weapons[0], shader, shotholeTex, "tex/gun.png");
    LoadMachineGun(weapons[1], shader, shotholeTex);
    int currentWeapon = 0;

    Camera3D camera = { 0 };
    camera.position = level.playerStart;
    camera.target = (Vector3){camera.position.x, camera.position.y, camera.position.z - 1};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Scene scene = { 0 };
    LoadScene(scene, shader, tileSize, camera.position, greenTex, wallTex, shotholeTex, whiteWallTex);

    InitZombieModel(shader);

    ZombieSpawn spawns[MAX_ZOMBIE_SPAWNS];
    int spawnCount = LoadZombieSpawns("map/enemy.txt", spawns, MAX_ZOMBIE_SPAWNS);
    if (spawnCount > SCENE_MAX_ZOMBIES) spawnCount = SCENE_MAX_ZOMBIES;
    scene.zombieCount = spawnCount;
    for (int i = 0; i < spawnCount; i++) {
        float wx = (float)spawns[i].col * tileSize + tileSize / 2.0f;
        float wz = (float)spawns[i].row * tileSize + tileSize / 2.0f;
        if (spawns[i].isMilitary) {
            InitZombie(scene.zombies[i], (Vector3){wx, 5.4f, wz}, milIdle, milIdle, milIdle, milIdle);
            scene.zombies[i].isMilitary = true;
        } else {
            InitZombie(scene.zombies[i], (Vector3){wx, 5.4f, wz}, zombiIdle, zombiWalk1, zombiWalk2, zombiDead);
        }
    }

    SetLightUniforms(shader, camera.position, {1,1,1}, 80.0f, 0.05f);
    int lightRangeLoc = GetShaderLocation(shader, "lightRange");
    int lightAmbLoc = GetShaderLocation(shader, "ambientStrength");
    int lightPosLoc = GetShaderLocation(shader, "lightPosition");

    DisableCursor();
    float yaw = 0.0f;

    rlDisableBackfaceCulling();

    int maxHealth = 100;
    float health = (float)maxHealth;

    float hitFlash = 0.0f;
    float hitShakeTime = 0.0f;

    bool gameOver = false;

    while (!WindowShouldClose())
    {
        if (gameOver) {
            if (IsKeyPressed(KEY_R)) {
                health = (float)maxHealth;
                camera.position = level.playerStart;
                camera.target = (Vector3){camera.position.x, camera.position.y, camera.position.z - 1};
                yaw = 0.0f;
                hitFlash = 0.0f;
                hitShakeTime = 0.0f;

                ZombieSpawn spawns2[MAX_ZOMBIE_SPAWNS];
                int sc2 = LoadZombieSpawns("map/enemy.txt", spawns2, MAX_ZOMBIE_SPAWNS);
                if (sc2 > SCENE_MAX_ZOMBIES) sc2 = SCENE_MAX_ZOMBIES;
                scene.zombieCount = sc2;
                for (int i = 0; i < sc2; i++) {
                    float wx = (float)spawns2[i].col * tileSize + tileSize / 2.0f;
                    float wz = (float)spawns2[i].row * tileSize + tileSize / 2.0f;
                    if (spawns2[i].isMilitary) {
                        InitZombie(scene.zombies[i], (Vector3){wx, 5.4f, wz}, milIdle, milIdle, milIdle, milIdle);
                        scene.zombies[i].isMilitary = true;
                    } else {
                        InitZombie(scene.zombies[i], (Vector3){wx, 5.4f, wz}, zombiIdle, zombiWalk1, zombiWalk2, zombiDead);
                    }
                }

                for (int i = 0; i < WEAPON_COUNT; i++) {
                    weapons[i].currentAmmo = weapons[i].maxAmmo;
                    weapons[i].isReloading = false;
                    weapons[i].reloadTimer = 0.0f;
                    weapons[i].fireCooldown = 0.0f;
                    weapons[i].bulletHoles.clear();
                }

                for (int i = 0; i < scene.doorCount; i++) {
                    scene.doors[i].isOpen = false;
                    scene.doors[i].bulletHoles.clear();
                }

                gameOver = false;
            }

            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("YOU DIED", GetScreenWidth()/2 - MeasureText("YOU DIED", 80)/2, GetScreenHeight()/2 - 80, 80, RED);
            DrawText("Press R to restart", GetScreenWidth()/2 - MeasureText("Press R to restart", 30)/2, GetScreenHeight()/2 + 20, 30, GRAY);
            EndDrawing();
            continue;
        }

        UpdatePlayer(&camera, &yaw, level, scene.doors, scene.doorCount, scene);

        if (IsKeyPressed(KEY_F)) {
            currentWeapon = (currentWeapon + 1) % WEAPON_COUNT;
        }

        UpdateWeapon(weapons[currentWeapon]);

        bool shotFired = (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_SPACE)) && weapons[currentWeapon].fireCooldown <= 0.0f && !weapons[currentWeapon].isReloading && weapons[currentWeapon].currentAmmo > 0;
        ShootWeapon(weapons[currentWeapon], camera, level, scene.doors, scene.doorCount, shader);

        {
            float dt = GetFrameTime();

            Vector3 doorPositions[SCENE_MAX_ZOMBIES + 1];
            int doorPosCount = 0;
            doorPositions[doorPosCount++] = camera.position;
            for (int i = 0; i < scene.zombieCount; i++) {
                if (scene.zombies[i].active)
                    doorPositions[doorPosCount++] = scene.zombies[i].position;
            }
            UpdateDoors(scene.doors, scene.doorCount, doorPositions, doorPosCount);

            for (int i = 0; i < scene.zombieCount; i++)
                UpdateZombie(scene.zombies[i], level, scene.doors, scene.doorCount, scene, camera.position, dt);

            if (shotFired) {
                Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
                float closestDist = 1e9f;
                int closestIdx = -1;
                for (int i = 0; i < scene.zombieCount; i++) {
                    if (!scene.zombies[i].active || scene.zombies[i].health <= 0.0f) continue;
                    if (!ZombieHitByRay(scene.zombies[i], camera.position, forward)) continue;
                    float dx = scene.zombies[i].position.x - camera.position.x;
                    float dz = scene.zombies[i].position.z - camera.position.z;
                    float dist = dx * dx + dz * dz;
                    if (dist < closestDist) {
                        closestDist = dist;
                        closestIdx = i;
                    }
                }
                if (closestIdx >= 0) {
                    Vector3 hitPos, hitNorm;
                    Vector3 closestZ = scene.zombies[closestIdx].position;
                    bool blocked = RaycastWall(level, camera.position, forward, sqrtf(closestDist), hitPos, hitNorm);
                    if (!blocked) {
                        scene.zombies[closestIdx].health -= 50.0f;
                        scene.zombies[closestIdx].hitTime = 0.15f;
                    }
                }
            }

            {
                static float touchTimer = 0.0f;
                bool touching = false;
                for (int i = 0; i < scene.zombieCount; i++) {
                    if (!scene.zombies[i].active || scene.zombies[i].health <= 0.0f) continue;
                    float dx = camera.position.x - scene.zombies[i].position.x;
                    float dz = camera.position.z - scene.zombies[i].position.z;
                    if (dx * dx + dz * dz < (PLAYER_RADIUS + scene.zombies[i].radius * 2) * (PLAYER_RADIUS + scene.zombies[i].radius * 2))
                        touching = true;
                }
                if (touching) {
                    touchTimer += dt;
                    while (touchTimer >= 0.3f) {
                        health -= 10.0f;
                        touchTimer -= 0.3f;
                        hitFlash = 0.2f;
                        hitShakeTime = 0.15f;
                    }
                } else {
                    touchTimer = 0.0f;
                }
            }
            for (int i = 0; i < scene.zombieCount; i++) {
                if (scene.zombies[i].wantsToShoot) {
                    health -= 10.0f;
                    scene.zombies[i].wantsToShoot = false;
                    hitFlash = 0.2f;
                    hitShakeTime = 0.15f;
                }
            }

            if (health < 0) health = 0;
            if (health <= 0.0f) gameOver = true;
        }

        float range = 80.0f;
        float ambient = 0.15f;
        SetShaderValue(shader, lightRangeLoc, &range, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, lightAmbLoc, &ambient, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, lightPosLoc, &camera.position, SHADER_UNIFORM_VEC3);

        int lampIdx = 0;
        for (int i = 0; i < scene.objectCount; i++) {
            if (!scene.objects[i].isLamp) continue;
            Vector3 pos = scene.objects[i].position;
            pos.y -= 5.0f;
            Vector3 color = {1.0f, 0.9f, 0.7f};
            float lr = 30.0f;
            char buf[32];

            snprintf(buf, sizeof(buf), "lampPos[%d]", lampIdx);
            SetShaderValue(shader, GetShaderLocation(shader, buf), &pos, SHADER_UNIFORM_VEC3);

            snprintf(buf, sizeof(buf), "lampColor[%d]", lampIdx);
            SetShaderValue(shader, GetShaderLocation(shader, buf), &color, SHADER_UNIFORM_VEC3);

            snprintf(buf, sizeof(buf), "lampRange[%d]", lampIdx);
            SetShaderValue(shader, GetShaderLocation(shader, buf), &lr, SHADER_UNIFORM_FLOAT);

            lampIdx++;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        Vector3 shakeOffset = {0};
        if (weapons[currentWeapon].shakeTime > 0.0f)
        {
            float t = weapons[currentWeapon].shakeTime / weapons[currentWeapon].shakeDuration;
            float intensity = weapons[currentWeapon].shakeAmount * t;
            shakeOffset.y = intensity;
            shakeOffset.x = ((float)GetRandomValue(0, 1000) / 500.0f - 1.0f) * intensity * 0.2f;
            shakeOffset.z = ((float)GetRandomValue(0, 1000) / 500.0f - 1.0f) * intensity * 0.2f;
            weapons[currentWeapon].shakeTime -= GetFrameTime();
        }
        if (hitShakeTime > 0.0f)
        {
            float intensity = 0.3f;
            shakeOffset.x += ((float)GetRandomValue(0, 1000) / 500.0f - 1.0f) * intensity;
            shakeOffset.y += ((float)GetRandomValue(0, 1000) / 500.0f - 1.0f) * intensity;
            hitShakeTime -= GetFrameTime();
        }

        Camera3D shakeCam = camera;
        shakeCam.position.x += shakeOffset.x;
        shakeCam.position.y += shakeOffset.y;
        shakeCam.position.z += shakeOffset.z;
        shakeCam.target.x += shakeOffset.x;
        shakeCam.target.y += shakeOffset.y;
        shakeCam.target.z += shakeOffset.z;

        BeginMode3D(shakeCam);

        DrawLevel(level);
        DrawScene(scene, shakeCam, shader);
        DrawWeaponDecals(weapons[currentWeapon], weapons[currentWeapon].decalModel);

        EndMode3D();

        DrawWeaponHUD(weapons[currentWeapon], (int)health, maxHealth);

        if (hitFlash > 0.0f)
        {
            int alpha = (int)(hitFlash / 0.15f * 100.0f);
            if (alpha > 100) alpha = 100;
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){180, 0, 0, (unsigned char)alpha});
            hitFlash -= GetFrameTime();
            if (hitFlash < 0.0f) hitFlash = 0.0f;
        }

        EndDrawing();
    }

    rlDisableBackfaceCulling();
    EnableCursor();
    UnloadLevel(level);
    UnloadScene(scene);
    for (int i = 0; i < WEAPON_COUNT; i++)
        UnloadWeapon(weapons[i]);
    UnloadShader(shader);
    UnloadTexture(texture);
    UnloadTexture(wallTex);
    UnloadTexture(greenTex);
    UnloadTexture(planksTex);
    UnloadTexture(whiteWallTex);
    UnloadTexture(zombiIdle);
    UnloadTexture(zombiWalk1);
    UnloadTexture(zombiWalk2);
    UnloadTexture(zombiDead);
    UnloadTexture(milIdle);
    UnloadTexture(shotholeTex);
    CloseWindow();
    return 0;
}
