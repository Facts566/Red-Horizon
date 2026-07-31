#include <raylib.h>
#include "game.h"
#include "config.h"

int main()
{
    InitWindow(GetScreenWidth(), GetScreenHeight(), "Red Horizon");
    InitAudioDevice();
    SetExitKey(KEY_NULL);

    Game game = {};
    game.state = GAME_MENU;
    game.exitGame = false;
    bool gameLoaded = false;

    while (!WindowShouldClose() && !game.exitGame)
    {
        if (game.state == GAME_MENU && IsKeyPressed(KEY_ESCAPE))
            game.exitGame = true;

        switch (game.state)
        {
            case GAME_MENU:
                DrawMenu(game);
                if (game.state == GAME_LEVEL_SELECT) {
                    if (!gameLoaded) {
                        InitGame(game);
                        gameLoaded = true;
                    }
                }
                break;

            case GAME_LEVEL_SELECT:
                DrawLevelSelect(game);
                if (game.state == GAME_PLAYING) {
                    LoadLevelByIndex(game, game.currentLevel);
                    DisableCursor();
                    gameLoaded = true;
                }
                break;

            case GAME_PLAYING:
                if (!game.gameOver)
                {
                    UpdateGame(game);
                    DrawGame(game);
                }
                else
                {
                    if (IsKeyPressed(KEY_R))
                        ResetGame(game);

                    if (IsKeyPressed(KEY_ESCAPE)) {
                        EnableCursor();
                        game.state = GAME_MENU;
                        game.menuSelection = 0;
                    }

                    BeginDrawing();
                    ClearBackground(BLACK);
                    DrawText("YOU DIED", GetScreenWidth()/2 - MeasureText("YOU DIED", 80)/2, GetScreenHeight()/2 - 100, 80, RED);
                    DrawText("Press R to restart", GetScreenWidth()/2 - MeasureText("Press R to restart", 30)/2, GetScreenHeight()/2 + 10, 30, GRAY);
                    DrawText("Press ESC for menu", GetScreenWidth()/2 - MeasureText("Press ESC for menu", 30)/2, GetScreenHeight()/2 + 50, 30, GRAY);
                    EndDrawing();
                }
                break;
        }
    }

    if (gameLoaded)
        UnloadGame(game);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
