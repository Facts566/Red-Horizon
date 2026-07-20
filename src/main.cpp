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

    float tileSize = 5.0f;
    float wallHeight = 20.0f;

    Shader shader = LoadLightShader();
    Level level = LoadLevel("map/map.txt", tileSize, wallHeight, texture, planksTex, wallTex, greenTex, shader);

    WeaponState weapon;
    LoadWeapon(weapon, shader);

    Camera3D camera = { 0 };
    camera.position = level.playerStart;
    camera.target = (Vector3){camera.position.x, camera.position.y, camera.position.z - 1};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Scene scene = { 0 };
    LoadScene(scene, shader, tileSize, camera.position, greenTex, wallTex);

    scene.zombieCount = 10;
    InitZombie(scene.zombies[0], (Vector3){30 * tileSize + tileSize / 2.0f, 5.4f, 4 * tileSize + tileSize / 2.0f});
    InitZombie(scene.zombies[1], (Vector3){10 * tileSize + tileSize / 2.0f, 5.4f, 3 * tileSize + tileSize / 2.0f});
    InitZombie(scene.zombies[2], (Vector3){16 * tileSize + tileSize / 2.0f, 5.4f, 6 * tileSize + tileSize / 2.0f});
    InitZombie(scene.zombies[3], (Vector3){25 * tileSize + tileSize / 2.0f, 5.4f, 3 * tileSize + tileSize / 2.0f});
    InitZombie(scene.zombies[4], (Vector3){33 * tileSize + tileSize / 2.0f, 5.4f, 6 * tileSize + tileSize / 2.0f});
    InitZombie(scene.zombies[5], (Vector3){28 * tileSize + tileSize / 2.0f, 5.4f, 11 * tileSize + tileSize / 2.0f});
    InitZombie(scene.zombies[6], (Vector3){26 * tileSize + tileSize / 2.0f, 5.4f, 15 * tileSize + tileSize / 2.0f});
    InitZombie(scene.zombies[7], (Vector3){13 * tileSize + tileSize / 2.0f, 5.4f, 15 * tileSize + tileSize / 2.0f});
    InitZombie(scene.zombies[8], (Vector3){27 * tileSize + tileSize / 2.0f, 5.4f, 19 * tileSize + tileSize / 2.0f});
    InitZombie(scene.zombies[9], (Vector3){27 * tileSize + tileSize / 2.0f, 5.4f, 22 * tileSize + tileSize / 2.0f});

    int sofaIndex = 0;

    SetLightUniforms(shader, camera.position, {1,1,1}, 80.0f, 0.05f);
    int lightRangeLoc = GetShaderLocation(shader, "lightRange");
    int lightAmbLoc = GetShaderLocation(shader, "ambientStrength");
    int lightPosLoc = GetShaderLocation(shader, "lightPosition");

    DisableCursor();
    float yaw = 0.0f;

    rlDisableBackfaceCulling();

    int maxHealth = 100;
    float health = (float)maxHealth;

    while (!WindowShouldClose())
    {
        UpdatePlayer(&camera, &yaw, level, scene.doors, scene.doorCount, GetCollider(scene, sofaIndex));

        UpdateWeapon(weapon);

        bool shotFired = (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_SPACE)) && weapon.fireCooldown <= 0.0f && !weapon.isReloading && weapon.currentAmmo > 0;
        ShootWeapon(weapon, camera, level, scene.doors, scene.doorCount, shader);

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
                UpdateZombie(scene.zombies[i], level, scene.doors, scene.doorCount, GetCollider(scene, sofaIndex), camera.position, dt);

            if (shotFired) {
                Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
                for (int i = 0; i < scene.zombieCount; i++) {
                    if (ZombieHitByRay(scene.zombies[i], camera.position, forward)) {
                        scene.zombies[i].health -= 50.0f;
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
                    while (touchTimer >= 1.0f) {
                        health -= 10.0f;
                        touchTimer -= 1.0f;
                    }
                } else {
                    touchTimer = 0.0f;
                }
            }
            if (health < 0) health = 0;
        }

        float range = 80.0f;
        float ambient = 0.05f;
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
        if (weapon.shakeTime > 0.0f)
        {
            float t = weapon.shakeTime / weapon.shakeDuration;
            float intensity = weapon.shakeAmount * t;
            shakeOffset.y = intensity;
            shakeOffset.x = ((float)GetRandomValue(0, 1000) / 500.0f - 1.0f) * intensity * 0.2f;
            shakeOffset.z = ((float)GetRandomValue(0, 1000) / 500.0f - 1.0f) * intensity * 0.2f;
            weapon.shakeTime -= GetFrameTime();
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
        DrawScene(scene, shakeCam);
        DrawWeaponDecals(weapon, weapon.decalModel);

        EndMode3D();

        DrawWeaponHUD(weapon, (int)health, maxHealth);

        EndDrawing();
    }

    rlDisableBackfaceCulling();
    EnableCursor();
    UnloadLevel(level);
    UnloadScene(scene);
    UnloadWeapon(weapon);
    UnloadShader(shader);
    UnloadTexture(texture);
    UnloadTexture(wallTex);
    UnloadTexture(greenTex);
    UnloadTexture(planksTex);
    CloseWindow();
    return 0;
}
