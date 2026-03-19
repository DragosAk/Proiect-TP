#ifndef FARM_H
#define FARM_H
#include "raylib.h"

#define SIZE 50          // marime matrice patratica
#define TILE_SIZE 60      // dimensiune vizuala unitate

typedef struct {
    int field[SIZE][SIZE];
    int pX, pY;
    int money, seeds, day;
} GameState;

void init_game(GameState *gs);
void save_game(GameState *gs);
void load_game(GameState *gs);

#endif