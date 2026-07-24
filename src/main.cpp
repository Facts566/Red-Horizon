#include <raylib.h>
#include "game.h"
#include "config.h"

int main()
{
    InitWindow(GetScreenWidth(), GetScreenHeight(), "FactsEngine");

    Game game = { 0 };
    InitGame(game);

    while (!WindowShouldClose())
    {
        if (game.gameOver) {
            if (IsKeyPressed(KEY_R))
                ResetGame(game);

            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("YOU DIED", GetScreenWidth()/2 - MeasureText("YOU DIED", 80)/2, GetScreenHeight()/2 - 80, 80, RED);
            DrawText("Press R to restart", GetScreenWidth()/2 - MeasureText("Press R to restart", 30)/2, GetScreenHeight()/2 + 20, 30, GRAY);
            EndDrawing();
            continue;
        }

        UpdateGame(game);
        DrawGame(game);
    }

    UnloadGame(game);
    CloseWindow();
    return 0;
}
