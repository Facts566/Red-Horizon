#include <raylib.h>
#include <rlgl.h>
#include "player.h"
#include "map.h"
#include "light.h"

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

    float roomW = 150.0f;
    float roomD = 150.0f;
    float roomH = 20.0f;
    float tileSize = 5.0f;

    Model fl = MakePlane(roomW, roomD, roomW/tileSize, roomD/tileSize, texture);
    Model cl = MakePlane(roomW, roomD, roomW/tileSize, roomD/tileSize, texture);
    Model fw = MakeWall(roomW, roomH, roomW/tileSize, roomH/tileSize, wallTex);
    Model bw = MakeWall(roomW, roomH, roomW/tileSize, roomH/tileSize, wallTex);
    Model lw = MakeWall(roomD, roomH, roomD/tileSize, roomH/tileSize, wallTex);
    Model rw = MakeWall(roomD, roomH, roomD/tileSize, roomH/tileSize, wallTex);

    Shader shader = LoadLightShader();
    SetLightUniforms(shader, {0, roomH - 1.0f, 0}, {1,1,1}, 60.0f, 0.02f);

    fl.materials[0].shader = shader;
    cl.materials[0].shader = shader;
    fw.materials[0].shader = shader;
    bw.materials[0].shader = shader;
    lw.materials[0].shader = shader;
    rw.materials[0].shader = shader;

    Camera3D camera = { 0 };
    camera.position = (Vector3){0, roomH/2, roomD/4};
    camera.target = (Vector3){0, roomH/2, 0};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    DisableCursor();
    float yaw = 0.0f;

    rlDisableBackfaceCulling();

    while (!WindowShouldClose())
    {
        UpdatePlayer(&camera, &yaw);

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera);

        DrawModel(fl, (Vector3){0, 0, 0}, 1.0f, WHITE);
        DrawModel(cl, (Vector3){0, roomH, 0}, 1.0f, WHITE);
        DrawModel(fw, (Vector3){0, 0, roomD/2}, 1.0f, WHITE);
        DrawModelEx(bw, (Vector3){0, 0, -roomD/2}, (Vector3){0,1,0}, 180.0f, (Vector3){1,1,1}, WHITE);
        DrawModelEx(lw, (Vector3){-roomW/2, 0, 0}, (Vector3){0,1,0}, -90.0f, (Vector3){1,1,1}, WHITE);
        DrawModelEx(rw, (Vector3){roomW/2, 0, 0}, (Vector3){0,1,0}, 90.0f, (Vector3){1,1,1}, WHITE);

        EndMode3D();
        DrawFPS(10, 10);
        DrawCircle(GetScreenWidth()/2, GetScreenHeight()/2, 4, WHITE);
        EndDrawing();
    }

    rlEnableBackfaceCulling();
    EnableCursor();
    UnloadShader(shader);
    UnloadTexture(texture);
    UnloadTexture(wallTex);
    CloseWindow();
    return 0;
}
