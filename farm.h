#ifndef FARM_H
#define FARM_H

#include "raylib.h"
#include <stdbool.h>

#define MAP_SIZE 16
#define TILE_SIZE 64
#define DAY_DURATION 300.0f
#define MAX_OBJECTS 64
#define MAX_INVENTORY 20
#define MAX_PARTICLES 128
#define MAGIC_NUMBER 0xFA000001

typedef enum { CROP_NONE=0, CROP_WHEAT, CROP_CARROT, CROP_PUMPKIN } CropType;
typedef enum { STAGE_EMPTY=0, STAGE_SEED, STAGE_GROWING, STAGE_ALMOST, STAGE_MATURE } GrowthStage;
typedef enum { TOOL_HOE=0, TOOL_WATERCAN, TOOL_SEEDS, TOOL_HARVEST } Tool;
typedef enum { WEATHER_SUNNY, WEATHER_CLOUDY, WEATHER_RAINY, WEATHER_STORMY } Weather;
typedef enum { STATE_MENU, STATE_PLAYING, STATE_INVENTORY, STATE_PAUSE, STATE_SHOP, STATE_TUTORIAL } GameState;

typedef enum {
    ITEM_NONE=0, ITEM_SEED_WHEAT, ITEM_SEED_CARROT, ITEM_SEED_PUMPKIN,
    ITEM_CROP_WHEAT, ITEM_CROP_CARROT, ITEM_CROP_PUMPKIN, 
    ITEM_FERTILIZER, ITEM_SCARECROW, ITEM_FENCE
} ItemType;

typedef enum { OBJ_SCARECROW, OBJ_FENCE_V, OBJ_FENCE_H } ObjectType;

typedef struct {
    CropType type;
    GrowthStage stage;
    float growTimer;
    float growInterval;
    bool watered;
    bool tilled;
    int fertilizerApplied; // Memoria numarului de aplicari de ingrasamant
} Cell;

typedef struct {
    Cell cells[MAP_SIZE][MAP_SIZE];
    int width, height;
} Farm;

typedef struct {
    ItemType type;
    int qty;
} InventorySlot;

typedef struct {
    int x, y;
    ObjectType type;
    bool active;
} PlacedObject;

typedef struct {
    Vector2 position;
    float speed;
    int money;
    InventorySlot inventory[MAX_INVENTORY];
    Tool activeTool;
    int selectedItemIdx;
    
    int level;
    int xp;

    int direction; 
    bool isMoving;
    int currentFrame;
    float frameTimer;
} Player;

typedef struct {
    unsigned int magic;
    Farm farm;
    Player player;
    int currentDay;
    float timeOfDay;
    Weather weather;
    PlacedObject objects[MAX_OBJECTS];
    int dailySold[3]; 
    bool tutorialDone;
    int tutorialStep;
    bool rainInvoked;
} SaveBlock;

typedef struct {
    Vector2 pos, vel;
    Color color;
    float life, maxLife;
    float size;
    bool active;
} Particle;

void InitGame(Texture2D tileset, Texture2D bg, Texture2D playerTex);
void UpdateGame(void);
void DrawGame(void);
void CloseGame(void);

#endif