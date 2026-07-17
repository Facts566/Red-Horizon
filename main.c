#include <raylib.h>

int main()
{
    InitWindow(GetScreenWidth(), GetScreenHeight(), "FactsEngine");
    Camera3D camera = { 0 };
    camera.position = (Vector3){10.0f, 10.0f, 10.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera);

        DrawCube((Vector3){0,0,0}, 2, 2, 2, WHITE);
        DrawCubeWires((Vector3){0,0,0}, 2, 2, 2, RED);
        DrawGrid(10, 10);

        EndMode3D();
        DrawFPS(10, 10);
        EndDrawing();
        UpdateCamera(&camera, CAMERA_FREE);
    }
    CloseWindow();
    return 0;
}