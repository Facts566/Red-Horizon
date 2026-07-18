#include <raylib.h>
#include <rlgl.h>
#include "player.h"
#include "map.h"
#include "light.h"
#include "level.h"

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

    float tileSize = 5.0f;
    float wallHeight = 20.0f;

    Shader shader = LoadLightShader();

    Level level = LoadLevel("map/map.txt", tileSize, wallHeight, texture, planksTex, wallTex, greenTex, shader);

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

    float shakeTime = 0.0f;
    const float SHAKE_DURATION = 0.3f;
    const float SHAKE_AMOUNT = 1.6f;

    while (!WindowShouldClose())
    {
        UpdatePlayer(&camera, &yaw, level);

        if (IsKeyPressed(KEY_F)) flashlightOn = !flashlightOn;

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            shakeTime = SHAKE_DURATION;

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

        EndMode3D();
        DrawTextureEx(gunTex, (Vector2){(float)GetScreenWidth()/2 - gunTex.width*gunScale/2, (float)GetScreenHeight() - gunTex.height * gunScale + gunKickY + 50.0f}, 0.0f, gunScale, WHITE);
        DrawFPS(10, 10);
        DrawCircle(GetScreenWidth()/2, GetScreenHeight()/2, 4, WHITE);
        EndDrawing();
    }

    rlDisableBackfaceCulling();
    EnableCursor();
    UnloadLevel(level);
    UnloadShader(shader);
    UnloadTexture(texture);
    UnloadTexture(wallTex);
    UnloadTexture(greenTex);
    UnloadTexture(planksTex);
    UnloadTexture(handTex);
    UnloadTexture(gunTex);
    CloseWindow();
    return 0;
}
