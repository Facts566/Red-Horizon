#include <raylib.h>
#include <rlgl.h>
#include "player.h"
#include "map.h"
#include "light.h"
#include "level.h"
#include "door.h"

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

    Texture2D handTex = LoadTexture("tex/hand_with_flashligh.png");
    Texture2D gunTex = LoadTexture("tex/gun.png");
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

    float tileSize = 5.0f;
    float wallHeight = 20.0f;

    Shader shader = LoadLightShader();

    Level level = LoadLevel("map/map.txt", tileSize, wallHeight, texture, planksTex, wallTex, greenTex, shader);

    Door door = CreateDoor(
        (Vector3){12 * tileSize, 0, 8 * tileSize},
        (Vector3){0,1,0}, 0.0f,
        doorTexClosed, doorTexOpen, shader
    );

    Camera3D camera = { 0 };
    camera.position = level.playerStart;
    camera.target = (Vector3){camera.position.x, camera.position.y, camera.position.z - 1};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetLightUniforms(shader, camera.position, {1,1,1}, 80.0f, 0.05f);
    int lightRangeLoc = GetShaderLocation(shader, "lightRange");
    int lightAmbLoc = GetShaderLocation(shader, "ambientStrength");
    int lightPosLoc = GetShaderLocation(shader, "lightPosition");
    bool flashlightOn = true;

    DisableCursor();
    float yaw = 0.0f;

    rlDisableBackfaceCulling();

    // Muzzle flash configuration
    const float FLASH_OFFSET_X = 170.0f;
    const float FLASH_OFFSET_Y = 70.0f;
    const float FLASH_SCALE = 2.0f;
    const float FLASH_DURATION = 0.08f;

    Texture2D muzzleTex = LoadTexture("tex/Muzzle.png");

    float shakeTime = 0.0f;
    float flashTime = 0.0f;
    float flashRotation = 0.0f;
    const float SHAKE_DURATION = 0.3f;
    const float SHAKE_AMOUNT = 1.6f;
    float fireCooldown = 0.0f;
    const float FIRE_RATE = 0.5f;

    int maxAmmo = 7;
    int currentAmmo = maxAmmo;
    bool isReloading = false;
    float reloadTimer = 0.0f;
    const float RELOAD_TIME = 5.0f;

    while (!WindowShouldClose())
    {
        UpdatePlayer(&camera, &yaw, level, &door);

        if (fireCooldown > 0.0f)
            fireCooldown -= GetFrameTime();

        if (IsKeyPressed(KEY_F)) flashlightOn = !flashlightOn;

        if (IsKeyPressed(KEY_R) && !isReloading && currentAmmo < maxAmmo)
        {
            isReloading = true;
            reloadTimer = RELOAD_TIME;
        }

        if (isReloading)
        {
            reloadTimer -= GetFrameTime();
            if (reloadTimer <= 0.0f)
            {
                currentAmmo = maxAmmo;
                isReloading = false;
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && fireCooldown <= 0.0f && !isReloading && currentAmmo > 0)
        {
            currentAmmo--;
            fireCooldown = FIRE_RATE;
            shakeTime = SHAKE_DURATION;
            flashTime = FLASH_DURATION;
            flashRotation = (float)GetRandomValue(0, 3600) / 10.0f;
        }

        Vector3 shakeOffset = {0};
        float gunKickY = 0.0f;
        float gunScale = 5.0f;
        if (shakeTime > 0.0f)
        {
            float t = shakeTime / SHAKE_DURATION;
            float intensity = SHAKE_AMOUNT * t;
            shakeOffset.y = intensity;
            shakeOffset.x = ((float)GetRandomValue(0, 1000) / 500.0f - 1.0f) * intensity * 0.2f;
            shakeOffset.z = ((float)GetRandomValue(0, 1000) / 500.0f - 1.0f) * intensity * 0.2f;
            gunKickY = -intensity * 12.0f;
            gunScale = 5.0f + 0.2f;
            shakeTime -= GetFrameTime();
        }

        if (flashTime > 0.0f)
            flashTime -= GetFrameTime();

        float range = flashlightOn ? 80.0f : 0.0f;
        float ambient = 0.01f;
        SetShaderValue(shader, lightRangeLoc, &range, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, lightAmbLoc, &ambient, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, lightPosLoc, &camera.position, SHADER_UNIFORM_VEC3);

        BeginDrawing();
        ClearBackground(BLACK);

        Camera3D shakeCam = camera;
        shakeCam.position.x += shakeOffset.x;
        shakeCam.position.y += shakeOffset.y;
        shakeCam.position.z += shakeOffset.z;
        shakeCam.target.x += shakeOffset.x;
        shakeCam.target.y += shakeOffset.y;
        shakeCam.target.z += shakeOffset.z;

        BeginMode3D(shakeCam);

        DrawLevel(level);
        DrawDoor(door);

        EndMode3D();

        Vector2 gunPos = {
            (float)GetScreenWidth()/2 - gunTex.width*gunScale/2,
            (float)GetScreenHeight() - gunTex.height*gunScale + gunKickY + 50.0f
        };
        if (flashTime > 0.0f)
        {
            Vector2 flashCenter = {
                gunPos.x + FLASH_OFFSET_X * gunScale,
                gunPos.y + FLASH_OFFSET_Y * gunScale
            };
            float fw = (float)muzzleTex.width * FLASH_SCALE;
            float fh = (float)muzzleTex.height * FLASH_SCALE;
            Rectangle flashDest = {flashCenter.x - fw/2, flashCenter.y - fh/2, fw, fh};
            DrawTexturePro(muzzleTex, (Rectangle){0, 0, (float)muzzleTex.width, (float)muzzleTex.height}, flashDest, (Vector2){fw/2, fh/2}, flashRotation, WHITE);
        }

        DrawTextureEx(gunTex, gunPos, 0.0f, gunScale, WHITE);

        const char *ammoText = TextFormat("%i / %i", currentAmmo, maxAmmo);
        int ammoFontSize = 40;
        int ammoTextWidth = MeasureText(ammoText, ammoFontSize);
        DrawText(ammoText, GetScreenWidth() / 2 - ammoTextWidth / 2, GetScreenHeight() - 120, ammoFontSize, WHITE);

        if (isReloading)
        {
            const char *reloadText = "RELOADING...";
            int reloadFontSize = 24;
            int reloadTextWidth = MeasureText(reloadText, reloadFontSize);
            int reloadY = GetScreenHeight() - 80;
            DrawText(reloadText, GetScreenWidth() / 2 - reloadTextWidth / 2, reloadY, reloadFontSize, YELLOW);

            float reloadProgress = 1.0f - reloadTimer / RELOAD_TIME;
            int barWidth = 200;
            int barHeight = 8;
            int barX = GetScreenWidth() / 2 - barWidth / 2;
            int barY = reloadY + 30;
            DrawRectangle(barX, barY, barWidth, barHeight, DARKGRAY);
            DrawRectangle(barX, barY, (int)(barWidth * reloadProgress), barHeight, YELLOW);
        }

        DrawFPS(10, 10);
        EndDrawing();
    }

    rlDisableBackfaceCulling();
    EnableCursor();
    UnloadLevel(level);
    UnloadDoor();
    UnloadShader(shader);
    UnloadTexture(texture);
    UnloadTexture(wallTex);
    UnloadTexture(greenTex);
    UnloadTexture(planksTex);
    UnloadTexture(handTex);
    UnloadTexture(gunTex);
    UnloadTexture(muzzleTex);
    UnloadTexture(doorTexClosed);
    UnloadTexture(doorTexOpen);
    CloseWindow();
    return 0;
}
