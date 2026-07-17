#include <raylib.h>

int main()
{
    InitWindow(GetScreenWidth(), GetScreenHeight(), "FactsEngine");

    Texture2D texture = LoadTexture("tex/floor.png");
    SetTextureFilter(texture, TEXTURE_FILTER_POINT);

    Mesh mesh = GenMeshCube(2.0f, 2.0f, 2.0f);
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
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera);

        DrawModel(model, (Vector3){0,0,0}, 1.0f, WHITE);
        DrawCubeWires((Vector3){0,0,0}, 2, 2, 2, RED);
        DrawGrid(10, 10);

        EndMode3D();
        DrawFPS(10, 10);
        EndDrawing();
        UpdateCamera(&camera, CAMERA_FIRST_PERSON);
    }
    EnableCursor();
    UnloadTexture(texture);
    CloseWindow();
    return 0;
}