#include "raylib.h"
#include "farm.h"

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1024, 768, "Farming Simulator Extended");
    
    // Initializam sistemul audio inainte sa se incarce fisierele!
    InitAudioDevice();
    
    SetExitKey(KEY_NULL); 
    
    SetTargetFPS(60);
    ToggleFullscreen();
    
    Texture2D tileset = LoadTexture("assets/tileset_stardew.png"); 
    Texture2D background = LoadTexture("assets/background_ferma.png");
    Texture2D playerTex = LoadTexture("assets/player.png"); 

    InitGame(tileset, background, playerTex);

    while (!WindowShouldClose()) {
        UpdateGame();
        DrawGame();
    }

    CloseGame();
    UnloadTexture(tileset);
    UnloadTexture(background);
    UnloadTexture(playerTex); 
    
    // Inchidem canalul hardware in mod curat
    CloseAudioDevice();
    
    CloseWindow();

    return 0;
}