#include "raylib.h"
#include "farm.h"
#include <stdlib.h>
#include <time.h>

typedef enum { MENU, GAMEPLAY, PAUSE } GameScreen;

// functie auxiliara pentru butoane UI
bool DrawButton(Rectangle bounds, const char *text) {
    bool clicked = false;
    Vector2 mousePoint = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePoint, bounds);

    if (isHovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        clicked = true;
    }

    Color rect;
    if (isHovered != 0)
        rect = LIGHTGRAY;
    else
        rect = GRAY;
    DrawRectangleRec(bounds, rect);
    DrawRectangleLinesEx(bounds, 2, DARKGRAY);

    int textWidth = MeasureText(text, 20);
    DrawText(text, bounds.x + bounds.width / 2 - textWidth / 2, bounds.y + bounds.height / 2 - 10, 20, BLACK);

    return clicked;
}

void UpdateGameplay(GameState *gs, float dt, float curTime, float playerSpeed) {
    gs->totalPlayTime += dt;
    gs->seconds = ((int)curTime) % 60;
    gs->minutes = (((int)curTime) / 60) % 60;
    gs->hours = ((int)curTime) / 3600;

    // miscare continua fara delay
    if (IsKeyDown(KEY_W)) gs->posY -= playerSpeed * dt;
    if (IsKeyDown(KEY_S)) gs->posY += playerSpeed * dt;
    if (IsKeyDown(KEY_A)) gs->posX -= playerSpeed * dt;
    if (IsKeyDown(KEY_D)) gs->posX += playerSpeed * dt;

    int tileX = (int)((gs->posX + TILE_SIZE/2) / TILE_SIZE);
    int tileY = (int)((gs->posY + TILE_SIZE/2) / TILE_SIZE);

    // limitare indici matrice
    if (tileX < 0) tileX = 0;
    if (tileX >= SIZE) tileX = SIZE - 1;
    if (tileY < 0) tileY = 0;
    if (tileY >= SIZE) tileY = SIZE - 1;

    // logica crestere plante
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            if (gs->field[y][x].state >= STATE_SEED && gs->field[y][x].state < STATE_MATURE) {
                float progress = (curTime - gs->field[y][x].plantTime) / GROWTH_TIME;
                if (progress >= 1.0f) gs->field[y][x].state = STATE_MATURE;
                else if (progress >= 0.66f) gs->field[y][x].state = STATE_ALMOST;
                else if (progress >= 0.33f) gs->field[y][x].state = STATE_GROWING;
            }
        }
    }

    // ACTIUNI
    // plantare
    if (IsKeyDown(KEY_P) && gs->seeds > 0 && gs->field[tileY][tileX].state == STATE_EMPTY) {
        if (!(tileX < SHOP_SIZE && tileY < SHOP_SIZE)) {
            gs->field[tileY][tileX].state = STATE_SEED;
            gs->field[tileY][tileX].plantTime = curTime;
            gs->seeds--;
            save_game(gs);
        }
    }
    
    // recoltare
    if (IsKeyPressed(KEY_R) && gs->field[tileY][tileX].state == STATE_MATURE) {
        gs->field[tileY][tileX].state = STATE_EMPTY;
        gs->inventory += 1;
        gs->seeds += (rand() % 2) + 1;
        save_game(gs);
    }
    
    bool inShop = (tileX < SHOP_SIZE && tileY < SHOP_SIZE);
    
    // vinde seminte 
    if (inShop && IsKeyPressed(KEY_V)) {
        if (gs->inventory > 0) {
            gs->money += gs->inventory * 40;
            gs->inventory = 0;
            save_game(gs);
        }
    }
    
    // cumpara seminte
    if (inShop && IsKeyPressed(KEY_B) && gs->money >= 15) {
        gs->money -= 15;
        gs->seeds += 1;
        save_game(gs);
    }
}

void DrawGameplay(GameState *gs, Camera2D camera, float curTime, Texture2D playerTex, int screenW) {
    BeginMode2D(camera);
        for (int y = 0; y < SIZE; y++) {
            for (int x = 0; x < SIZE; x++) {
                // desenare pamant/fundal parcela
                Color groundCol = (Color){ 35, 25, 15, 255 };
                if (x < SHOP_SIZE && y < SHOP_SIZE) groundCol = DARKBLUE;
                DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE - 2, TILE_SIZE - 2, groundCol);

                if (gs->field[y][x].state != STATE_EMPTY) {
                    Color plantCol;
                    float scale = 0.0f;

                    // calcul progres animatie
                    float p = (curTime - gs->field[y][x].plantTime) / GROWTH_TIME;
                    if (p > 1.0f) p = 1.0f;

                    // scalare marime plant in tile
                    scale = 0.2f + (p * 0.7f);

                    switch(gs->field[y][x].state) {
                        case STATE_SEED: plantCol = DARKGREEN; break;
                        case STATE_GROWING:  plantCol = LIME;      break;
                        case STATE_ALMOST:   plantCol = YELLOW;    break;
                        case STATE_MATURE:   plantCol = GOLD;      scale = 0.95f; break;
                    }

                    // desenare planta centrata
                    float offset = (TILE_SIZE * (1.0f - scale)) / 2;
                    DrawRectangle(x * TILE_SIZE + offset, y * TILE_SIZE + offset, TILE_SIZE * scale, TILE_SIZE * scale, plantCol);
                }
            }
        }

        DrawText("SHOP", 10, 10, 40, WHITE);

        // desenare avatar (imagine)
        Rectangle sourceRec = { 0.0f, 0.0f, (float)playerTex.width, (float)playerTex.height };
        Rectangle destRec = { gs->posX, gs->posY, (float)TILE_SIZE, (float)TILE_SIZE };
        DrawTexturePro(playerTex, sourceRec, destRec, (Vector2){0,0}, 0.0f, WHITE);

    EndMode2D();

    // interfata hud
    DrawRectangle(0, 0, screenW, 100, Fade(BLACK, 0.7f));
    DrawText(TextFormat("BANI: $%d | SEMINTE: %d | INVENTAR: %d | TIMP: %d:%d:%d", gs->money, gs->seeds, gs->inventory, gs->hours, gs->minutes, gs->seconds), 20, 20, 25, GREEN);

    int tileX = (int)((gs->posX + TILE_SIZE/2) / TILE_SIZE);
    int tileY = (int)((gs->posY + TILE_SIZE/2) / TILE_SIZE);
    bool inShop = (tileX < SHOP_SIZE && tileY < SHOP_SIZE);

    if (inShop) DrawText("SHOP: [V] VINDE ($40/buc) | [B] CUMPARA SEMINTE ($15)", 20, 60, 20, SKYBLUE);
    else DrawText("WASD: MISCARE | P: PLANTA | R: RECOLTA", 20, 60, 20, GRAY);
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

    // dezactivare ESC automat din raylib
    SetExitKey(KEY_NULL);

    // avatar
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

        // LOGICA DE UPDATE (Stari)
        if (currentScreen == GAMEPLAY) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                currentScreen = PAUSE; 
            } else {
                UpdateGameplay(&gs, dt, curTime, playerSpeed);
                camera.target = (Vector2){ gs.posX + TILE_SIZE/2, gs.posY + TILE_SIZE/2 };
            }
        }
        else if (currentScreen == PAUSE) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                currentScreen = GAMEPLAY; 
            }
        }

        // RANDARE GRAFICA (Stari)
        BeginDrawing();
        ClearBackground(BLACK);

        if (currentScreen == MENU) {
            int titleWidth = MeasureText("FARMING SIMULATOR", 40);
            DrawText("FARMING SIMULATOR", screenW / 2 - titleWidth / 2, screenH / 3, 40, WHITE);

            if (DrawButton((Rectangle){ screenW / 2 - 100, screenH / 2, 200, 50 }, "Start Game")) {
                currentScreen = GAMEPLAY;
            }
            if (DrawButton((Rectangle){ screenW / 2 - 100, screenH / 2 + 70, 200, 50 }, "Quit")) {
                shouldExit = true;
            }
        }
        else if (currentScreen == GAMEPLAY) {
            DrawGameplay(&gs, camera, curTime, playerTex, screenW);
        }
        else if (currentScreen == PAUSE) {
            DrawGameplay(&gs, camera, curTime, playerTex, screenW);

            // overlay transparent pentru meniul de pauza
            DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.6f));
            
            int pauseWidth = MeasureText("PAUSED", 40);
            DrawText("PAUSED", screenW / 2 - pauseWidth / 2, screenH / 3, 40, WHITE);

            if (DrawButton((Rectangle){ screenW / 2 - 100, screenH / 2, 200, 50 }, "Resume")) {
                currentScreen = GAMEPLAY;
            }
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