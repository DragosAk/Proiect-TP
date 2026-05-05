#include "farm.h"
#include <stdio.h>
#include <stdlib.h>

// Date de configurare pentru plante
const float GROWTH_TIMES[MAX_CROP_TYPES] = { 10.0f, 20.0f, 30.0f };
const int SEED_PRICES[MAX_CROP_TYPES] = { 15, 25, 45 };
const int CROP_SELL_PRICES[MAX_CROP_TYPES] = { 40, 75, 150 };
const char* CROP_NAMES[MAX_CROP_TYPES] = { "GRAU", "MORCOV", "DOVLEAC" };

void init_game(GameState *gs) {
    gs->posX = (SIZE / 2) * TILE_SIZE;
    gs->posY = (SIZE / 2) * TILE_SIZE;
    gs->money = 100;
    gs->selected_seed = CROP_WHEAT;
    gs->lastActionTime = 0.0f;
    

    gs->seeds[0] = 5;
    gs->inventory[0] = 0;
    gs->seeds[1] = 3;
    gs->inventory[1] = 0;
    gs->seeds[2] = 2;
    gs->inventory[2] = 0;   
    
    gs->totalPlayTime = 0;
    gs->hours = 0;
    gs->minutes = 0;
    gs->seconds = 0;
    
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            gs->field[i][j].state = STATE_EMPTY;
            gs->field[i][j].plantTime = 0;
            gs->field[i][j].cropType = 0;
        }
    }
}

void save_game(GameState *gs) {
    FILE *f = fopen("save.dat", "wb");
    if (f != NULL) { 
        fwrite(gs, sizeof(GameState), 1, f); 
        fclose(f); 
    } else {
        printf("Eroare deschidere fisier pentru salvare!\n");
    }
}

void load_game(GameState *gs) {
    FILE *f = fopen("save.dat", "rb");
    if (f != NULL) { 
        fread(gs, sizeof(GameState), 1, f); 
        fclose(f); 
    } else {
        init_game(gs);
    }
}

void UpdateGameplay(GameState *gs, float dt, float curTime, float playerSpeed) {
    gs->totalPlayTime += dt;
    gs->seconds = ((int)curTime) % 60;
    gs->minutes = (((int)curTime) / 60) % 60;
    gs->hours = ((int)curTime) / 3600;

    // Selectie seminte (Tastele 1, 2, 3)
    if (IsKeyPressed(KEY_ONE)) gs->selected_seed = CROP_WHEAT;
    if (IsKeyPressed(KEY_TWO)) gs->selected_seed = CROP_CARROT;
    if (IsKeyPressed(KEY_THREE)) gs->selected_seed = CROP_PUMPKIN;

    // Coliziune si miscare
    float nextX = gs->posX;
    float nextY = gs->posY;

    if (IsKeyDown(KEY_W)) nextY -= playerSpeed * dt;
    if (IsKeyDown(KEY_S)) nextY += playerSpeed * dt;
    if (IsKeyDown(KEY_A)) nextX -= playerSpeed * dt;
    if (IsKeyDown(KEY_D)) nextX += playerSpeed * dt;

    Rectangle playerBox = { nextX, nextY, TILE_SIZE, TILE_SIZE };
    Rectangle shopBox = { 0, 0, SHOP_SIZE * TILE_SIZE, SHOP_SIZE * TILE_SIZE };

    bool hasCollision = false;
    // Coliziune cu magazinul
    if (CheckCollisionRecs(playerBox, shopBox)) hasCollision = true;
    // Coliziune cu marginile lumii
    if (nextX < 0 || nextX + TILE_SIZE > SIZE * TILE_SIZE) hasCollision = true;
    if (nextY < 0 || nextY + TILE_SIZE > SIZE * TILE_SIZE) hasCollision = true;

    if (!hasCollision) {
        gs->posX = nextX;
        gs->posY = nextY;
    }

    int tileX = (int)((gs->posX + TILE_SIZE/2) / TILE_SIZE);
    int tileY = (int)((gs->posY + TILE_SIZE/2) / TILE_SIZE);

    // Crestere plante
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            if (gs->field[y][x].state >= STATE_SEED && gs->field[y][x].state < STATE_MATURE) {
                float timeNeeded = GROWTH_TIMES[gs->field[y][x].cropType];
                float progress = (curTime - gs->field[y][x].plantTime) / timeNeeded;
                
                if (progress >= 1.0f) gs->field[y][x].state = STATE_MATURE;
                else if (progress >= 0.66f) gs->field[y][x].state = STATE_ALMOST;
                else if (progress >= 0.33f) gs->field[y][x].state = STATE_GROWING;
            }
        }
    }

    // Actiuni
    bool canAct = (curTime - gs->lastActionTime) > 0.15f; 

    // Plantare
    if (IsKeyDown(KEY_P) && canAct && gs->seeds[gs->selected_seed] > 0 && gs->field[tileY][tileX].state == STATE_EMPTY) {
        if (!(tileX < SHOP_SIZE && tileY < SHOP_SIZE)) {
            gs->field[tileY][tileX].state = STATE_SEED;
            gs->field[tileY][tileX].plantTime = curTime;
            gs->field[tileY][tileX].cropType = gs->selected_seed;
            gs->seeds[gs->selected_seed]--;
            gs->lastActionTime = curTime;
            save_game(gs);
        }
    }
    
    // Recoltare
    if (IsKeyDown(KEY_O) && canAct && gs->field[tileY][tileX].state == STATE_MATURE) {
        int harvestedType = gs->field[tileY][tileX].cropType;
        gs->field[tileY][tileX].state = STATE_EMPTY;
        gs->inventory[harvestedType]++;
        gs->seeds[harvestedType] += (rand() % 2) + 1; // 1 sau 2 seminte returnate
        gs->lastActionTime = curTime;
        save_game(gs);
    }
    
    bool inShop = (tileX < SHOP_SIZE + 1 && tileY < SHOP_SIZE + 1); // raza putin mai mare fiindca magazinul e solid
    
    // Vinde toate culturile din inventar
    if (inShop && IsKeyDown(KEY_V) && canAct) {
        bool soldSomething = false;
        for(int i = 0; i < MAX_CROP_TYPES; i++) {
            if (gs->inventory[i] > 0) {
                gs->money += gs->inventory[i] * CROP_SELL_PRICES[i];
                gs->inventory[i] = 0;
                soldSomething = true;
            }
        }
        if (soldSomething) {
            gs->lastActionTime = curTime;
            save_game(gs);
        }
    }
    
    // Cumpara seminte selectate
    if (inShop && IsKeyDown(KEY_B) && canAct) {
        int cost = SEED_PRICES[gs->selected_seed];
        if (gs->money >= cost) {
            gs->money -= cost;
            gs->seeds[gs->selected_seed]++;
            gs->lastActionTime = curTime;
            save_game(gs);
        }
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

                    float timeNeeded = GROWTH_TIMES[gs->field[y][x].cropType];
                    float p = (curTime - gs->field[y][x].plantTime) / timeNeeded;
                    if (p > 1.0f) p = 1.0f;

                    scale = 0.2f + (p * 0.7f);

                    // Diferentiere vizuala baza pe tipul culturii cand e matura
                    switch(gs->field[y][x].state) {
                        case STATE_SEED: plantCol = DARKGREEN; break;
                        case STATE_GROWING:  plantCol = LIME; break;
                        case STATE_ALMOST:   plantCol = YELLOW; break;
                        case STATE_MATURE:   
                            if (gs->field[y][x].cropType == CROP_WHEAT) plantCol = GOLD;
                            else if (gs->field[y][x].cropType == CROP_CARROT) plantCol = ORANGE;
                            else plantCol = RED; // Dovleac
                            scale = 0.95f; 
                            break;
                    }

                    float offset = (TILE_SIZE * (1.0f - scale)) / 2;
                    DrawRectangle(x * TILE_SIZE + offset, y * TILE_SIZE + offset, TILE_SIZE * scale, TILE_SIZE * scale, plantCol);
                }
            }
        }

        DrawText("SHOP", 10, 10, 40, WHITE);

        Rectangle sourceRec = { 0.0f, 0.0f, (float)playerTex.width, (float)playerTex.height };
        Rectangle destRec = { gs->posX, gs->posY, (float)TILE_SIZE, (float)TILE_SIZE };
        DrawTexturePro(playerTex, sourceRec, destRec, (Vector2){0,0}, 0.0f, WHITE);

    EndMode2D();

    // HUD
    DrawRectangle(0, 0, screenW, 110, Fade(BLACK, 0.8f));
    
    DrawText(TextFormat("BANI: $%d | SEMINTE SELECTATE: %s", gs->money, CROP_NAMES[gs->selected_seed]), 20, 10, 25, GREEN);
    
    DrawText(TextFormat("SEMINTE - Grau: %d | Morcov: %d | Dovleac: %d", gs->seeds[0], gs->seeds[1], gs->seeds[2]), 20, 45, 20, LIGHTGRAY);
    DrawText(TextFormat("INVENTAR - Grau: %d | Morcov: %d | Dovleac: %d", gs->inventory[0], gs->inventory[1], gs->inventory[2]), 20, 75, 20, WHITE);

    int tileX = (int)((gs->posX + TILE_SIZE/2) / TILE_SIZE);
    int tileY = (int)((gs->posY + TILE_SIZE/2) / TILE_SIZE);
    bool inShop = (tileX < SHOP_SIZE + 1 && tileY < SHOP_SIZE + 1);

    if (inShop) {
        int cost = SEED_PRICES[gs->selected_seed];
        DrawText(TextFormat("SHOP: [V] VINDE TOT | [B] CUMPARA %s ($%d)", CROP_NAMES[gs->selected_seed], cost), screenW - 600, 20, 20, SKYBLUE);
    } else {
        DrawText("[1,2,3] SCHIMBA SEMINTE | [P] PLANTA | [R] RECOLTA", screenW - 600, 20, 20, GRAY);
    }
}