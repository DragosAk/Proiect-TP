#include "raylib.h"
#include "farm.h"
#include <stdlib.h>
#include <time.h>

typedef enum { MENU, GAMEPLAY, PAUSE } GameScreen;

bool DrawButton(Rectangle bounds, const char *text) {
    bool clicked = false;
    Vector2 mousePoint = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePoint, bounds);

    if (isHovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        clicked = true;
    }

    Color rect = isHovered ? LIGHTGRAY : GRAY;
    DrawRectangleRec(bounds, rect);
    DrawRectangleLinesEx(bounds, 2, DARKGRAY);

    int textWidth = MeasureText(text, 20);
    DrawText(text, bounds.x + bounds.width / 2 - textWidth / 2, bounds.y + bounds.height / 2 - 10, 20, BLACK);

    return clicked;
}

int main() {
    srand(time(NULL));
    GameState gs;
    load_game(&gs);

    InitWindow(0, 0, "Farming Simulator");
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    SetWindowSize(screenW, screenH);
    ToggleFullscreen();
    SetTargetFPS(60);

    SetExitKey(KEY_NULL); // dezactivare ESC automat din raylib

    Texture2D playerTex = LoadTexture("90f32769-3507-4966-9f77-b52ff63627e5.jpeg");

    Camera2D camera = { 0 };
    camera.offset = (Vector2){ screenW / 2.0f, screenH / 2.0f };
    camera.zoom = 1.0f;

    float playerSpeed = 350.0f;
    GameScreen currentScreen = MENU;
    bool shouldExit = false;

    while (!WindowShouldClose() && !shouldExit) {
        float dt = GetFrameTime();
        float curTime = GetTime();

        // Update stari
        if (currentScreen == GAMEPLAY) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                currentScreen = PAUSE; 
            } else {
                // Logica matematica si input-ul din farm.c
                UpdateGameplay(&gs, dt, curTime, playerSpeed);
                camera.target = (Vector2){ gs.posX + TILE_SIZE/2, gs.posY + TILE_SIZE/2 };
            }
        }
        else if (currentScreen == PAUSE) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                currentScreen = GAMEPLAY; 
            }
        }

        // Randare
        BeginDrawing();
        ClearBackground(BLACK);

        if (currentScreen == MENU) {
            int titleWidth = MeasureText("FARMING SIMULATOR", 40);
            DrawText("FARMING SIMULATOR", screenW / 2 - titleWidth / 2, screenH / 3, 40, WHITE);

            if (DrawButton((Rectangle){ screenW / 2 - 100, screenH / 2, 200, 50 }, "Start Game")) currentScreen = GAMEPLAY;
            if (DrawButton((Rectangle){ screenW / 2 - 100, screenH / 2 + 70, 200, 50 }, "Quit")) shouldExit = true;
        }
        else if (currentScreen == GAMEPLAY) {
            // Logica grafica a mapei din farm.c
            DrawGameplay(&gs, camera, curTime, playerTex, screenW);
        }
        else if (currentScreen == PAUSE) {
            DrawGameplay(&gs, camera, curTime, playerTex, screenW);
            DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.6f));
            
            int pauseWidth = MeasureText("PAUSED", 40);
            DrawText("PAUSED", screenW / 2 - pauseWidth / 2, screenH / 3, 40, WHITE);

            if (DrawButton((Rectangle){ screenW / 2 - 100, screenH / 2, 200, 50 }, "Resume")) currentScreen = GAMEPLAY;
            if (DrawButton((Rectangle){ screenW / 2 - 100, screenH / 2 + 70, 200, 50 }, "Main Menu")) {
                currentScreen = MENU;
                save_game(&gs);
            }
        }

        EndDrawing();
    }

    UnloadTexture(playerTex);
    save_game(&gs);
    CloseWindow();
    return 0;
}