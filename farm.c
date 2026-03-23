#include "farm.h"
#include <stdio.h>

void init_game(GameState *gs) {
    gs->posX = (SIZE / 2) * TILE_SIZE;
    gs->posY = (SIZE / 2) * TILE_SIZE;
    gs->money = 100;
    gs->seeds = 10;
    gs->inventory = 0;
    gs->totalPlayTime = 0;
    gs->hours = 0;
    gs->minutes = 0;
    gs->seconds = 0;
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            gs->field[i][j].state = 0;
            gs->field[i][j].plantTime = 0;
        }
    }
}

void save_game(GameState *gs) {
    FILE *f = fopen("save.dat", "wb");
    if (f) { fwrite(gs, sizeof(GameState), 1, f); fclose(f); }
}

void load_game(GameState *gs) {
    FILE *f = fopen("save.dat", "rb");
    if (f) { fread(gs, sizeof(GameState), 1, f); fclose(f); }
    else init_game(gs);
}