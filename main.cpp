#include "raylib.h"
#include "rlgl.h"
#include <cstdlib>

int main()
{
    InitWindow(GetScreenWidth(), GetScreenHeight(), "FactsEngine");

    Texture2D texture = LoadTexture("tex/floor.png");
    SetTextureFilter(texture, TEXTURE_FILTER_POINT);
    SetTextureWrap(texture, TEXTURE_WRAP_REPEAT);
    rlTextureParameters(texture.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
    rlTextureParameters(texture.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);

    float platformW = 50.0f;
    float platformD = 50.0f;
    float tileSize = 2.0f;

    Mesh mesh = GenMeshPlane(platformW, platformD, 1, 1);

    for (int i = 0; i < mesh.vertexCount; i++) {
        mesh.texcoords[i * 2]     *= platformW / tileSize;
        mesh.texcoords[i * 2 + 1] *= platformD / tileSize;
    }

    free(mesh.vboId);
    mesh.vboId = NULL;
    mesh.vaoId = 0;
    UploadMesh(&mesh, false);

    Model model = LoadModelFromMesh(mesh);
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    Camera3D camera = { 0 };
    camera.position = (Vector3){10.0f, 10.0f, 10.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    DisableCursor();

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_FIRST_PERSON);

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera);

        DrawModel(model, (Vector3){0,0,0}, 1.0f, WHITE);
        DrawCubeWires((Vector3){0,0,0}, 2, 2, 2, RED);
        DrawGrid(10, 10);

        EndMode3D();
        DrawFPS(10, 10);
        EndDrawing();
    }

    EnableCursor();
    UnloadTexture(texture);
    CloseWindow();
    return 0;
}