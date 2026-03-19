#include "farm.h"
#include <stdio.h>
#include <stdlib.h>

void init_game(GameState *gs) {
    gs->pX = SIZE / 2; // incepe în mijlocul hărții
    gs->pY = SIZE / 2;
    gs->money = 100;
    gs->seeds = 10;
    gs->day = 1;
    for(int i = 0; i < SIZE; i++)
        for(int j = 0; j < SIZE; j++) gs->field[i][j] = 0;
}

void save_game(GameState *gs) {
    FILE *f = fopen("save.dat", "wb");
    if (f) {
        fwrite(gs, sizeof(GameState), 1, f);
        fclose(f);
    }
}

void load_game(GameState *gs) {
    FILE *f = fopen("save.dat", "rb");
    if (f) {
        fread(gs, sizeof(GameState), 1, f);
        fclose(f);
    } else {
        init_game(gs);
    }
}