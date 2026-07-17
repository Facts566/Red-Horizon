#include "raylib.h"
#include "rlgl.h"
#include <cstdlib>
#include <math.h>

const float PLAYER_SPEED = 20.0f;

void UpdatePlayer(Camera3D *camera, float *yaw)
{
    *yaw -= GetMouseDelta().x * 0.003f;

    float fx = sinf(*yaw);
    float fz = cosf(*yaw);
    float rx = cosf(*yaw);
    float rz = -sinf(*yaw);
    float dx = 0, dz = 0;

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    { dx += fx; dz += fz; }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))   { dx -= fx; dz -= fz; }
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))   { dx += rx; dz += rz; }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))  { dx -= rx; dz -= rz; }

    float len = sqrtf(dx * dx + dz * dz);
    if (len > 0) { dx /= len; dz /= len; }

    float dt = PLAYER_SPEED * GetFrameTime();
    camera->position.x += dx * dt;
    camera->position.z += dz * dt;
    camera->target.x = camera->position.x + fx;
    camera->target.y = camera->position.y;
    camera->target.z = camera->position.z + fz;
}

Model MakePlane(float w, float l, float tu, float tv, Texture2D tex)
{
    Mesh m = GenMeshPlane(w, l, 1, 1);
    for (int i = 0; i < m.vertexCount; i++) {
        m.texcoords[i*2]   *= tu;
        m.texcoords[i*2+1] *= tv;
    }
    free(m.vboId); m.vboId = NULL; m.vaoId = 0;
    UploadMesh(&m, false);
    Model mod = LoadModelFromMesh(m);
    mod.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex;
    return mod;
}

int main()
{
    InitWindow(GetScreenWidth(), GetScreenHeight(), "FactsEngine");

    Texture2D texture = LoadTexture("tex/floor.png");
    SetTextureFilter(texture, TEXTURE_FILTER_POINT);
    SetTextureWrap(texture, TEXTURE_WRAP_REPEAT);
    rlTextureParameters(texture.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
    rlTextureParameters(texture.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);

    float roomW = 150.0f;
    float roomD = 150.0f;
    float roomH = 20.0f;
    float tileSize = 5.0f;

    Model fl = MakePlane(roomW, roomD, roomW/tileSize, roomD/tileSize, texture);
    Model cl = MakePlane(roomW, roomD, roomW/tileSize, roomD/tileSize, texture);
    Model fw = MakePlane(roomW, roomH, roomW/tileSize, roomH/tileSize, texture);
    Model bw = MakePlane(roomW, roomH, roomW/tileSize, roomH/tileSize, texture);
    Model lw = MakePlane(roomH, roomD, roomH/tileSize, roomD/tileSize, texture);
    Model rw = MakePlane(roomH, roomD, roomH/tileSize, roomD/tileSize, texture);

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
        DrawModelEx(fw, (Vector3){0, roomH/2, roomD/2}, (Vector3){1,0,0}, -90.0f, (Vector3){1,1,1}, WHITE);
        DrawModelEx(bw, (Vector3){0, roomH/2, -roomD/2}, (Vector3){1,0,0}, 90.0f, (Vector3){1,1,1}, WHITE);
        DrawModelEx(lw, (Vector3){-roomW/2, roomH/2, 0}, (Vector3){0,0,1}, 90.0f, (Vector3){1,1,1}, WHITE);
        DrawModelEx(rw, (Vector3){roomW/2, roomH/2, 0}, (Vector3){0,0,1}, -90.0f, (Vector3){1,1,1}, WHITE);

        EndMode3D();
        DrawFPS(10, 10);
        EndDrawing();
    }

    rlEnableBackfaceCulling();
    EnableCursor();
    UnloadTexture(texture);
    CloseWindow();
    return 0;
}