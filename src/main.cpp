#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include "player.h"
#include "map.h"
#include "light.h"
#include "level.h"
#include "door.h"
#include "raycast.h"
#include "scene.h"
#include "weapon.h"

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

    Texture2D doorTexClosed = LoadTexture("tex/door/door_1.png");
    SetTextureFilter(doorTexClosed, TEXTURE_FILTER_POINT);
    SetTextureWrap(doorTexClosed, TEXTURE_WRAP_REPEAT);
    rlTextureParameters(doorTexClosed.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
    rlTextureParameters(doorTexClosed.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);

    Texture2D doorTexOpen = LoadTexture("tex/door/door_2.png");
    SetTextureFilter(doorTexOpen, TEXTURE_FILTER_POINT);
    SetTextureWrap(doorTexOpen, TEXTURE_WRAP_REPEAT);
    rlTextureParameters(doorTexOpen.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
    rlTextureParameters(doorTexOpen.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);

    Door doors[MAX_DOORS];
    int doorCount = 0;
    doors[doorCount++] = CreateDoor(
        (Vector3){12 * tileSize, 0, 9 * tileSize},
        (Vector3){0,1,0}, 0.0f,
        doorTexClosed, doorTexOpen, greenTex, wallTex, shader, LoadTexture("tex/shothole.png")
    );
    doors[doorCount++] = CreateDoor(
        (Vector3){22 * tileSize, 0, 17 * tileSize},
        (Vector3){0,1,0}, 270.0f,
        doorTexClosed, doorTexOpen, greenTex, wallTex, shader, LoadTexture("tex/shothole.png")
    );

    WeaponState weapon;
    LoadWeapon(weapon, shader);

    Camera3D camera = { 0 };
    camera.position = level.playerStart;
    camera.target = (Vector3){camera.position.x, camera.position.y, camera.position.z - 1};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Scene scene;
    LoadScene(scene, shader, tileSize, camera.position);

    SetLightUniforms(shader, camera.position, {1,1,1}, 80.0f, 0.05f);
    int lightRangeLoc = GetShaderLocation(shader, "lightRange");
    int lightAmbLoc = GetShaderLocation(shader, "ambientStrength");
    int lightPosLoc = GetShaderLocation(shader, "lightPosition");
    bool flashlightOn = true;

    DisableCursor();
    float yaw = 0.0f;

    rlDisableBackfaceCulling();

    int maxHealth = 100;
    int health = maxHealth;

    while (!WindowShouldClose())
    {
        UpdatePlayer(&camera, &yaw, level, doors, doorCount, GetSofaCollider(scene));

        if (IsKeyPressed(KEY_F)) flashlightOn = !flashlightOn;

        UpdateWeapon(weapon);
        ShootWeapon(weapon, camera, level, doors, doorCount, shader);

        float range = flashlightOn ? 80.0f : 0.0f;
        float ambient = 0.01f;
        SetShaderValue(shader, lightRangeLoc, &range, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, lightAmbLoc, &ambient, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, lightPosLoc, &camera.position, SHADER_UNIFORM_VEC3);

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
        DrawDoors(doors, doorCount);
        DrawScene(scene);
        DrawWeaponDecals(weapon, weapon.decalModel);

        EndMode3D();

        DrawWeaponHUD(weapon, health, maxHealth);

        EndDrawing();
    }

    rlDisableBackfaceCulling();
    EnableCursor();
    UnloadLevel(level);
    UnloadDoors();
    UnloadScene(scene);
    UnloadWeapon(weapon);
    UnloadShader(shader);
    UnloadTexture(texture);
    UnloadTexture(wallTex);
    UnloadTexture(greenTex);
    UnloadTexture(planksTex);
    UnloadTexture(doorTexClosed);
    UnloadTexture(doorTexOpen);
    CloseWindow();
    return 0;
}
