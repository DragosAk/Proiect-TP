#include "raylib.h"
#include "farm.h"
#include <stdlib.h>
#include <time.h>

int main() {
    // inițializare random și stare joc
    srand(time(NULL));
    GameState gs;
    load_game(&gs);

    // configurare fereastra fullscreen
    InitWindow(0, 0, "Farming Simulator V1");
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    SetWindowSize(screenW, screenH);
    ToggleFullscreen();
    SetTargetFPS(60);

    // configurare camera 2D
    Camera2D camera = { 0 };
    camera.target = (Vector2){ gs.pX * TILE_SIZE, gs.pY * TILE_SIZE };
    camera.offset = (Vector2){ screenW / 2.0f, screenH / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    while (!WindowShouldClose()) {
        //input miscare
        if (IsKeyPressed(KEY_W) && gs.pY > 0) gs.pY--;
        if (IsKeyPressed(KEY_S) && gs.pY < SIZE - 1) gs.pY++;
        if (IsKeyPressed(KEY_A) && gs.pX > 0) gs.pX--;
        if (IsKeyPressed(KEY_D) && gs.pX < SIZE - 1) gs.pX++;

        // ACTIUNI
        // plantare
        if (IsKeyPressed(KEY_P) && gs.seeds > 0 && gs.field[gs.pY][gs.pX] == 0) {
            gs.field[gs.pY][gs.pX] = 1;
            gs.seeds--;
            save_game(&gs);
        }

        // recoltare seminte cu seminte randomizate(1-3)
        if (IsKeyPressed(KEY_R) && gs.field[gs.pY][gs.pX] == 2) {
            gs.field[gs.pY][gs.pX] = 0;
            gs.money += 25;
            gs.seeds += (rand() % 3) + 1;
            save_game(&gs);
        }

        //avansare zi
        if (IsKeyPressed(KEY_N)) {
            gs.day++;
            for(int y=0; y<SIZE; y++)
                for(int x=0; x<SIZE; x++)
                    if(gs.field[y][x] == 1) gs.field[y][x] = 2;
            save_game(&gs);
        }

        // actualizare camera
        Vector2 playerWorldPos = { gs.pX * TILE_SIZE, gs.pY * TILE_SIZE };
        camera.target.x += (playerWorldPos.x - camera.target.x) * 0.1f;
        camera.target.y += (playerWorldPos.y - camera.target.y) * 0.1f;

        //interfata 2D
        BeginDrawing();
            ClearBackground(BLACK);

            BeginMode2D(camera);
                // afisare teren
                for (int y = 0; y < SIZE; y++) {
                    for (int x = 0; x < SIZE; x++) {
                        Color col = (Color){ 45, 30, 20, 255 }; // Pământ închis
                        if (gs.field[y][x] == 1) col = DARKGREEN;
                        if (gs.field[y][x] == 2) col = GOLD;

                        DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE - 2, TILE_SIZE - 2, col);
                    }
                }
                // player
                DrawRectangle(gs.pX * TILE_SIZE + 5, gs.pY * TILE_SIZE + 5, TILE_SIZE - 10, TILE_SIZE - 10, SKYBLUE);
            EndMode2D();

            // HUD
            DrawRectangle(0, 0, screenW, 150, Fade(BLACK, 0.8f));
            DrawText(TextFormat("ZIUA: %d  |  BANI: $%d  |  SEMINTE: %d", gs.day, gs.money, gs.seeds), 20, 20, 48, GREEN);
            DrawText("WASD: Mers | P: Plant | R: Recolta | N: Zi Noua | ESC: Iesire", 20, 100, 32, GRAY);

        EndDrawing();
    }

    save_game(&gs);
    CloseWindow();
    return 0;
}