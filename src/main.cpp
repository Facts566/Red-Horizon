#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <vector>
#include <cmath>
#include "player.h"
#include "map.h"
#include "light.h"
#include "level.h"
#include "door.h"

static bool RaycastWall(Level level, Vector3 origin, Vector3 dir, float maxDist, Vector3 &hitPos, Vector3 &hitNormal)
{
    float ts = level.tileSize;
    float wh = level.wallHeight;

    int col = (int)(origin.x / ts);
    int row = (int)(origin.z / ts);
    if (origin.x < 0) col--;
    if (origin.z < 0) row--;

    int stepX = (dir.x > 0) ? 1 : (dir.x < 0) ? -1 : 0;
    int stepZ = (dir.z > 0) ? 1 : (dir.z < 0) ? -1 : 0;

    float tMaxX = (dir.x != 0) ? ((col + (stepX > 0 ? 1 : 0)) * ts - origin.x) / dir.x : INFINITY;
    float tMaxZ = (dir.z != 0) ? ((row + (stepZ > 0 ? 1 : 0)) * ts - origin.z) / dir.z : INFINITY;
    if (tMaxX < 0) tMaxX = 0;
    if (tMaxZ < 0) tMaxZ = 0;

    float tDeltaX = (dir.x != 0) ? ts / fabsf(dir.x) : INFINITY;
    float tDeltaZ = (dir.z != 0) ? ts / fabsf(dir.z) : INFINITY;

    float t = 0;
    bool steppedX = false;

    for (int i = 0; i < 200; i++)
    {
        if (col >= 0 && col < level.width && row >= 0 && row < level.height)
        {
            char c = level.data[row * level.width + col];
            if (c == '&' || c == '@')
            {
                hitPos.x = origin.x + dir.x * t;
                hitPos.y = origin.y + dir.y * t;
                hitPos.z = origin.z + dir.z * t;

                if (hitPos.y < 0 || hitPos.y > wh)
                    return false;

                hitNormal = steppedX ? (Vector3){(float)-stepX, 0, 0} : (Vector3){0, 0, (float)-stepZ};
                return true;
            }
        }

        if (tMaxX < tMaxZ)
        {
            t = tMaxX;
            tMaxX += tDeltaX;
            col += stepX;
            steppedX = true;
        }
        else
        {
            t = tMaxZ;
            tMaxZ += tDeltaZ;
            row += stepZ;
            steppedX = false;
        }

        if (t > maxDist) break;
    }
    return false;
}

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
    Texture2D shotholeTex = LoadTexture("tex/shothole.png");
    Model decalModel = MakeWall(0.6f, 0.6f, 1.0f, 1.0f, shotholeTex);
    decalModel.materials[0].shader = shader;

    float shakeTime = 0.0f;
    float flashTime = 0.0f;
    float flashRotation = 0.0f;
    const float SHAKE_DURATION = 0.3f;
    const float SHAKE_AMOUNT = 1.6f;
    float fireCooldown = 0.0f;
    const float FIRE_RATE = 0.5f;

    int maxHealth = 100;
    int health = maxHealth;

    int maxAmmo = 7;
    int currentAmmo = maxAmmo;
    bool isReloading = false;
    float reloadTimer = 0.0f;
    const float RELOAD_TIME = 5.0f;

    struct BulletHole { Vector3 pos; Vector3 normal; };
    std::vector<BulletHole> bulletHoles;

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

            Vector3 dir = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
            Vector3 hitPos, hitNormal;
            if (RaycastWall(level, camera.position, dir, 100.0f, hitPos, hitNormal))
            {
                hitPos.x += hitNormal.x * 0.05f;
                hitPos.z += hitNormal.z * 0.05f;
                if (bulletHoles.size() >= 50)
                    bulletHoles.erase(bulletHoles.begin());
                bulletHoles.push_back({hitPos, hitNormal});
            }
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

        for (auto &bh : bulletHoles)
        {
            Vector3 p = bh.pos;
            p.y -= 0.3f;
            if (bh.normal.z < 0)
                DrawModelEx(decalModel, p, (Vector3){0,1,0}, 180.0f, (Vector3){1,1,1}, WHITE);
            else if (bh.normal.z > 0)
                DrawModel(decalModel, p, 1.0f, WHITE);
            else if (bh.normal.x < 0)
                DrawModelEx(decalModel, p, (Vector3){0,1,0}, -90.0f, (Vector3){1,1,1}, WHITE);
            else if (bh.normal.x > 0)
                DrawModelEx(decalModel, p, (Vector3){0,1,0}, 90.0f, (Vector3){1,1,1}, WHITE);
        }

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

        int hpBarWidth = 375;
        int hpBarHeight = 33;
        int hpBarX = 20;
        int hpBarY = GetScreenHeight() - 70;
        float hpPercent = (float)health / maxHealth;
        Color hpColor = hpPercent > 0.5f ? GREEN : (hpPercent > 0.25f ? ORANGE : RED);
        DrawRectangle(hpBarX - 3, hpBarY - 3, hpBarWidth + 6, hpBarHeight + 6, GRAY);
        DrawRectangle(hpBarX, hpBarY, hpBarWidth, hpBarHeight, ColorAlpha(BLACK, 0.7f));
        DrawRectangle(hpBarX, hpBarY, (int)(hpBarWidth * hpPercent), hpBarHeight, hpColor);
        DrawText(TextFormat("HP: %i / %i", health, maxHealth), hpBarX + 15, hpBarY + 4, 27, WHITE);

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
    UnloadTexture(shotholeTex);
    UnloadModel(decalModel);
    UnloadTexture(doorTexClosed);
    UnloadTexture(doorTexOpen);
    CloseWindow();
    return 0;
}
