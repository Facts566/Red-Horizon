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

    Texture2D handTex = LoadTexture("tex/hand_with_flashligh.png");

    float tileSize = 5.0f;
    float wallHeight = 20.0f;

    Level level = LoadLevel("map.txt", tileSize, wallHeight);

    Shader shader = LoadLightShader();

    Camera3D camera = { 0 };
    camera.position = level.playerStart;
    camera.target = (Vector3){camera.position.x, camera.position.y, camera.position.z - 1};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetLightUniforms(shader, camera.position, {1,1,1}, 80.0f, 0.05f);
    int lightRangeLoc = GetShaderLocation(shader, "lightRange");
    int lightAmbLoc = GetShaderLocation(shader, "ambientStrength");
    bool flashlightOn = true;

    DisableCursor();
    float yaw = 0.0f;

    rlDisableBackfaceCulling();

    while (!WindowShouldClose())
    {
        UpdatePlayer(&camera, &yaw);

        if (IsKeyPressed(KEY_F)) flashlightOn = !flashlightOn;
        float range = flashlightOn ? 80.0f : 0.0f;
        float ambient = 0.05f;
        SetShaderValue(shader, lightRangeLoc, &range, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, lightAmbLoc, &ambient, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "lightPosition"), &camera.position, SHADER_UNIFORM_VEC3);

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera);

        DrawLevel(level, texture, wallTex, shader);

        EndMode3D();
        if (flashlightOn)
            DrawTextureEx(handTex, (Vector2){20, (float)GetScreenHeight() - handTex.height * 20 - 20}, 0.0f, 20.0f, WHITE);
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
    UnloadTexture(handTex);
    CloseWindow();
    return 0;
}
