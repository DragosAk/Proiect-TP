#ifndef FARM_H
#define FARM_H

#include "raylib.h"

#define SIZE 50
#define TILE_SIZE 60
#define SHOP_SIZE 3

#define STATE_EMPTY 0
#define STATE_SEED 1
#define STATE_GROWING 2
#define STATE_ALMOST 3
#define STATE_MATURE 4

#define MAX_CROP_TYPES 3
#define CROP_WHEAT 0
#define CROP_CARROT 1
#define CROP_PUMPKIN 2

typedef struct {
    int state;
    float plantTime;
    int cropType; // memoreaza tipul de planta din aceasta celula
} Tile;

typedef struct {
    Tile field[SIZE][SIZE];
    float posX, posY;
    int money;
    int selected_seed;
    int seeds[MAX_CROP_TYPES];
    int inventory[MAX_CROP_TYPES];
    float totalPlayTime;
    int seconds, minutes, hours;
    float lastActionTime; // timer pentru a modera IsKeyDown
} GameState;

void init_game(GameState *gs);
void save_game(GameState *gs);
void load_game(GameState *gs);

// Functiile de logica originare din main.c
void UpdateGameplay(GameState *gs, float dt, float curTime, float playerSpeed);
void DrawGameplay(GameState *gs, Camera2D camera, float curTime, Texture2D playerTex, int screenW);

#endif