#ifndef FARM_H
#define FARM_H

#include "raylib.h"

#define SIZE 50
#define TILE_SIZE 60
#define GROWTH_TIME 10.0f
#define SHOP_SIZE 3

#define STATE_EMPTY 0
#define STATE_SEED 1
#define STATE_GROWING 2
#define STATE_ALMOST 3
#define STATE_MATURE 4

typedef struct {
    int state;
    float plantTime;
} Tile;

typedef struct {
    Tile field[SIZE][SIZE];
    float posX, posY;
    int money, seeds, inventory, seconds, minutes, hours;
    float totalPlayTime;
} GameState;

void init_game(GameState *gs);
void save_game(GameState *gs);
void load_game(GameState *gs);

#endif