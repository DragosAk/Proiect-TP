#include "farm.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

static SaveBlock save_data;
static GameState current_state = STATE_MENU;
static Particle particles[MAX_PARTICLES];

static Texture2D texTiles;
static Texture2D texBg;
static Texture2D texPlayer;
static Camera2D cam;

static Sound sndHoe;
static Sound sndWater;
static Sound sndPlant;
static Sound sndHarvest;
static Sound sndBuy;
static Sound sndSell;
static Sound sndStep;

static Sound sndClick;
static Music musMenu;

static int basePrices[] = {10, 25, 60}; 
static int seedPrices[] = {5, 12, 30};

static float levelUpAnimTimer = 0.0f;
static float hoverAlphaTime = 0.0f;

static int currentFenceRotation = 0; 

static const char* GetItemName(ItemType t) {
    switch(t) {
        case ITEM_SEED_WHEAT: return "Sem.\nGrau";
        case ITEM_SEED_CARROT: return "Sem.\nMorcov";
        case ITEM_SEED_PUMPKIN: return "Sem.\nDovleac";
        case ITEM_CROP_WHEAT: return "Grau";
        case ITEM_CROP_CARROT: return "Morcov";
        case ITEM_CROP_PUMPKIN: return "Dovleac";
        case ITEM_SCARECROW: return "Sperie\ntoare";
        case ITEM_FENCE: return "Gard";
        case ITEM_FERTILIZER: return "Ingrasa-\nmant";
        default: return "?";
    }
}

static const char* tutTexts[] = {
    "Miscari: WASD",
    "Selecteaza Sapa (tasteaza 1) si da click pentru a ara pamantul",
    "Selecteaza Stropitoarea (tasteaza 2) si uda pamantul o singura data",
    "Deschide Inventarul (TAB) si planteaza seminte (tasta 3)",
    "Asteapta sa creasca si recolteaza cu Mana Libera (tasta 4)",
    "Du-te in Magazin (Podeaua de lemn) si apasa [E] pentru a accesa."
};

static void SaveDataToFile(void);
static void LoadDataFromFile(void);
static void AddXP(int amount);
static void SpawnParticles(float x, float y, int count, Color c);
static void UpdateDayWeather(float dt);
static void AddItem(ItemType type, int qty);
static void InitNewGame(void);
static void DrawUI(void);
static bool HasScarecrowNear(int gridX, int gridY);

void InitGame(Texture2D tileset, Texture2D bg, Texture2D playerTex) {
    texTiles = tileset;
    texBg = bg;
    texPlayer = playerTex;
    
    sndHoe = LoadSound("wavs/hoe.wav");
    sndWater = LoadSound("wavs/water.wav");
    sndPlant = LoadSound("wavs/plant.wav");
    sndHarvest = LoadSound("wavs/harvest.wav");
    sndBuy = LoadSound("wavs/buy.wav");
    sndSell = LoadSound("wavs/sell.wav");
    sndStep = LoadSound("wavs/step.wav");
    sndClick = LoadSound("wavs/click.wav");
    musMenu = LoadMusicStream("menu.mp3");
    
    cam.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
    cam.target = (Vector2){ 0, 0 };
    cam.rotation = 0.0f;
    cam.zoom = 1.0f;
    
    for(int i=0; i<MAX_PARTICLES; i++) particles[i].active = false;
    
    FILE *f = fopen("savegame.dat", "rb");
    if(f) {
        fclose(f);
    } else {
        InitNewGame();
    }
    
    PlayMusicStream(musMenu);
}

static void InitNewGame(void) {
    memset(&save_data, 0, sizeof(SaveBlock));
    save_data.magic = MAGIC_NUMBER;
    save_data.farm.width = MAP_SIZE;
    save_data.farm.height = MAP_SIZE;
    save_data.player.position = (Vector2){ MAP_SIZE*TILE_SIZE/2.0f, MAP_SIZE*TILE_SIZE/2.0f };
    save_data.player.speed = 300.0f;
    save_data.player.money = 100;
    save_data.player.level = 1;
    save_data.player.xp = 0;
    save_data.player.activeTool = TOOL_HOE;
    
    save_data.player.direction = 0; 
    save_data.player.isMoving = false;
    save_data.player.currentFrame = 0;
    save_data.player.frameTimer = 0.0f;
    
    AddItem(ITEM_SEED_WHEAT, 5);
    AddItem(ITEM_SEED_CARROT, 2);
    AddItem(ITEM_SCARECROW, 1);
    
    save_data.currentDay = 1;
    save_data.timeOfDay = 0.0f; 
    save_data.weather = WEATHER_SUNNY;
    save_data.tutorialDone = false;
    save_data.tutorialStep = 0;
}

static void SaveDataToFile(void) {
    FILE *f = fopen("savegame.dat", "wb");
    if(f) {
        fwrite(&save_data, sizeof(SaveBlock), 1, f);
        fclose(f);
    }
}

static void LoadDataFromFile(void) {
    FILE *f = fopen("savegame.dat", "rb");
    if(f) {
        SaveBlock temp;
        fread(&temp, sizeof(SaveBlock), 1, f);
        if(temp.magic == MAGIC_NUMBER) {
            save_data = temp;
        }
        fclose(f);
    }
}

static void AddItem(ItemType type, int qty) {
    for(int i=0; i<MAX_INVENTORY; i++) {
        if(save_data.player.inventory[i].type == type) {
            save_data.player.inventory[i].qty += qty;
            return;
        }
    }
    for(int i=0; i<MAX_INVENTORY; i++) {
        if(save_data.player.inventory[i].type == ITEM_NONE) {
            save_data.player.inventory[i].type = type;
            save_data.player.inventory[i].qty = qty;
            return;
        }
    }
}

static void RemoveItem(int slotIdx, int qty) {
    if(slotIdx < 0 || slotIdx >= MAX_INVENTORY) return;
    save_data.player.inventory[slotIdx].qty -= qty;
    if(save_data.player.inventory[slotIdx].qty <= 0) {
        save_data.player.inventory[slotIdx].type = ITEM_NONE;
        save_data.player.inventory[slotIdx].qty = 0;
    }
}

static void AddXP(int amount) {
    save_data.player.xp += amount;
    int req = save_data.player.level * save_data.player.level * 50;
    if(save_data.player.xp >= req) {
        save_data.player.level++;
        save_data.player.xp -= req;
        levelUpAnimTimer = 2.0f;
        SpawnParticles(save_data.player.position.x, save_data.player.position.y, 20, GOLD);
    }
}

static void SpawnParticles(float x, float y, int count, Color c) {
    for(int i=0; i<MAX_PARTICLES && count>0; i++) {
        if(!particles[i].active) {
            particles[i].active = true;
            particles[i].pos = (Vector2){ x + GetRandomValue(-10,10), y + GetRandomValue(-10,10) };
            particles[i].vel = (Vector2){ GetRandomValue(-50,50), GetRandomValue(-50,50) };
            particles[i].color = c;
            particles[i].maxLife = (float)GetRandomValue(5, 15) / 10.0f;
            particles[i].life = particles[i].maxLife;
            particles[i].size = GetRandomValue(2, 6);
            count--;
        }
    }
}

static bool HasScarecrowNear(int gx, int gy) {
    for(int i=0; i<MAX_OBJECTS; i++) {
        if(save_data.objects[i].active && save_data.objects[i].type == OBJ_SCARECROW) {
            int dx = abs(save_data.objects[i].x - gx);
            int dy = abs(save_data.objects[i].y - gy);
            if(dx <= 3 && dy <= 3) return true;
        }
    }
    return false;
}

static void UpdateDayWeather(float dt) {
    save_data.timeOfDay += dt;
    if(save_data.timeOfDay >= DAY_DURATION) {
        save_data.timeOfDay = 0.0f;
        save_data.currentDay++;
        save_data.rainInvoked = false;
        
        int r = GetRandomValue(1, 100);
        if(r <= 40) save_data.weather = WEATHER_SUNNY;
        else if(r <= 70) save_data.weather = WEATHER_CLOUDY;
        else if(r <= 95) save_data.weather = WEATHER_RAINY;
        else save_data.weather = WEATHER_STORMY;
        
        for(int y=0; y<MAP_SIZE; y++) {
            for(int x=0; x<MAP_SIZE; x++) {
                if(save_data.weather == WEATHER_RAINY || save_data.weather == WEATHER_STORMY) {
                    if(save_data.farm.cells[y][x].tilled) save_data.farm.cells[y][x].watered = true;
                }
                
                if(save_data.weather == WEATHER_STORMY && save_data.farm.cells[y][x].stage == STAGE_MATURE) {
                    if(!HasScarecrowNear(x, y) && GetRandomValue(1,100) <= 20) {
                        save_data.farm.cells[y][x].stage = STAGE_SEED; 
                    }
                }
            }
        }
        SaveDataToFile();
    }
}

static void UpdatePlayerInteractions(float mx, float my) {
    int gx = (int)(mx / TILE_SIZE);
    int gy = (int)(my / TILE_SIZE);
    
    if(save_data.player.activeTool == TOOL_SEEDS) {
        int sel = save_data.player.selectedItemIdx;
        if(save_data.player.inventory[sel].type == ITEM_FENCE && IsKeyPressed(KEY_R)) {
            currentFenceRotation = !currentFenceRotation;
        }
    }
    
    if(gx < 0 || gx >= MAP_SIZE || gy < 0 || gy >= MAP_SIZE) return;
    if(gx < 3 && gy < 3) return; 
    
    Cell *c = &save_data.farm.cells[gy][gx];
    
    float px = save_data.player.position.x + TILE_SIZE/2.0f;
    float py = save_data.player.position.y + TILE_SIZE/2.0f;
    float dist = sqrtf(powf(mx-px,2) + powf(my-py,2));
    if(dist > TILE_SIZE * 2.0f) return; 

    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        
        if(save_data.player.activeTool == TOOL_HARVEST) {
            bool objectPicked = false;
            for(int i=0; i<MAX_OBJECTS; i++) {
                if(save_data.objects[i].active && save_data.objects[i].x == gx && save_data.objects[i].y == gy) {
                    ItemType itemToGive = ITEM_NONE;
                    if (save_data.objects[i].type == OBJ_SCARECROW) itemToGive = ITEM_SCARECROW;
                    else if (save_data.objects[i].type == OBJ_FENCE_V || save_data.objects[i].type == OBJ_FENCE_H) itemToGive = ITEM_FENCE;
                    
                    AddItem(itemToGive, 1);
                    save_data.objects[i].active = false;
                    PlaySound(sndHarvest); 
                    SpawnParticles(mx, my, 5, GRAY);
                    objectPicked = true;
                    break;
                }
            }
            if(objectPicked) return; 
        }

        if(save_data.player.activeTool == TOOL_HOE && !c->tilled) {
            c->tilled = true;
            PlaySound(sndHoe);
            AddXP(2);
            if(save_data.tutorialStep == 1) save_data.tutorialStep++;
        }
        else if(save_data.player.activeTool == TOOL_WATERCAN && c->tilled && !c->watered) {
            c->watered = true;
            PlaySound(sndWater);
            AddXP(1);
            if(save_data.tutorialStep == 2) save_data.tutorialStep++;
        }
        else if(save_data.player.activeTool == TOOL_SEEDS) {
            int sel = save_data.player.selectedItemIdx;
            ItemType itm = save_data.player.inventory[sel].type;
            if (itm == ITEM_NONE) return;
            
            bool hasObj = false;
            for(int i=0; i<MAX_OBJECTS; i++) {
                if(save_data.objects[i].active && save_data.objects[i].x == gx && save_data.objects[i].y == gy) {
                    hasObj = true; break;
                }
            }

            if(itm == ITEM_SEED_WHEAT || itm == ITEM_SEED_CARROT || itm == ITEM_SEED_PUMPKIN) {
                if(c->tilled && c->stage == STAGE_EMPTY && !hasObj) {
                    c->stage = STAGE_SEED;
                    c->growTimer = 0.0f;
                    c->fertilizerApplied = 0; 
                    
                    float speedMod = (save_data.player.level >= 3) ? 0.75f : 1.0f;
                    c->growInterval = 10.0f * speedMod; 
                    
                    if(itm == ITEM_SEED_WHEAT) c->type = CROP_WHEAT;
                    if(itm == ITEM_SEED_CARROT) c->type = CROP_CARROT;
                    if(itm == ITEM_SEED_PUMPKIN) c->type = CROP_PUMPKIN;
                    
                    RemoveItem(sel, 1);
                    PlaySound(sndPlant);
                    SpawnParticles(mx, my, 3, DARKBROWN);
                    AddXP(5);
                    if(save_data.tutorialStep == 3) save_data.tutorialStep++;
                }
            }
            else if(itm == ITEM_SCARECROW || itm == ITEM_FENCE) {
                if(!hasObj && c->stage == STAGE_EMPTY) {
                    
                    Rectangle pRect = { save_data.player.position.x + 16, save_data.player.position.y + 32, 32, 32 };
                    Rectangle objRect = {0};
                    
                    if(itm == ITEM_SCARECROW) {
                        objRect = (Rectangle){ gx * TILE_SIZE + 16, gy * TILE_SIZE + 10, 32, 50 };
                    } else {
                        if (currentFenceRotation == 0) objRect = (Rectangle){ gx * TILE_SIZE + 20, gy * TILE_SIZE, 24, TILE_SIZE };
                        else objRect = (Rectangle){ gx * TILE_SIZE, gy * TILE_SIZE + 20, TILE_SIZE, 24 };
                    }
                    
                    if (CheckCollisionRecs(pRect, objRect)) {
                        return; 
                    }

                    for(int i=0; i<MAX_OBJECTS; i++) {
                        if(!save_data.objects[i].active) {
                            save_data.objects[i].active = true;
                            if(itm == ITEM_SCARECROW) {
                                save_data.objects[i].type = OBJ_SCARECROW;
                            } else {
                                save_data.objects[i].type = (currentFenceRotation == 0) ? OBJ_FENCE_V : OBJ_FENCE_H;
                            }
                            
                            save_data.objects[i].x = gx;
                            save_data.objects[i].y = gy;
                            RemoveItem(sel, 1);
                            PlaySound(sndPlant);
                            SpawnParticles(mx, my, 5, GRAY);
                            break;
                        }
                    }
                }
            }
            else if(itm == ITEM_FERTILIZER) {
                if(c->stage > STAGE_EMPTY && c->stage < STAGE_MATURE && c->watered) {
                    c->fertilizerApplied++; 
                    
                    int requiredFertilizers = 1;
                    if(c->type == CROP_CARROT) requiredFertilizers = 2;
                    if(c->type == CROP_PUMPKIN) requiredFertilizers = 3;
                    
                    RemoveItem(sel, 1);
                    PlaySound(sndPlant); 
                    SpawnParticles(mx, my, 10, MAGENTA);
                    AddXP(2);
                    
                    if(c->fertilizerApplied >= requiredFertilizers) {
                        c->stage = STAGE_MATURE; 
                    }
                }
            }
        }
        else if(save_data.player.activeTool == TOOL_HARVEST && c->stage == STAGE_MATURE) {
            ItemType drop = ITEM_NONE;
            ItemType seedDrop = ITEM_NONE;
            int xpGained = 0;
            int seedQty = 0;
            
            if(c->type == CROP_WHEAT) { 
                drop = ITEM_CROP_WHEAT; 
                xpGained = 15; 
                seedDrop = ITEM_SEED_WHEAT;
                seedQty = GetRandomValue(1, 3);
            }
            if(c->type == CROP_CARROT) { 
                drop = ITEM_CROP_CARROT; 
                xpGained = 25; 
                seedDrop = ITEM_SEED_CARROT;
                seedQty = GetRandomValue(1, 2);
            }
            if(c->type == CROP_PUMPKIN) { 
                drop = ITEM_CROP_PUMPKIN; 
                xpGained = 50; 
                seedDrop = ITEM_SEED_PUMPKIN;
                seedQty = GetRandomValue(1, 2);
            }
            
            // Adaugare recolta in inventar
            AddItem(drop, 1);
            // Adaugare stocastica de seminte recuperate
            if(seedQty > 0) AddItem(seedDrop, seedQty);
            
            c->stage = STAGE_EMPTY;
            c->tilled = false; 
            c->watered = false; 
            c->fertilizerApplied = 0; 
            c->type = CROP_NONE;
            
            PlaySound(sndHarvest);
            SpawnParticles(mx, my, 8, LIME);
            AddXP(xpGained);
            if(save_data.tutorialStep == 4) save_data.tutorialStep++;
        }
    }
}

static void UpdateMenu(void) {
    float sw = GetScreenWidth();
    float sh = GetScreenHeight();
    
    Rectangle btnNew = { sw/2 - 100, sh/2 - 60, 200, 50 };
    Rectangle btnCont = { sw/2 - 100, sh/2 + 10, 200, 50 };
    Rectangle btnQuit = { sw/2 - 100, sh/2 + 80, 200, 50 };
    
    Vector2 mp = GetMousePosition();
    bool hasSave = false;
    FILE *f = fopen("savegame.dat", "rb");
    if(f) { hasSave = true; fclose(f); }

    if(CheckCollisionPointRec(mp, btnNew) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        PlaySound(sndClick);
        StopMusicStream(musMenu); 
        InitNewGame();
        current_state = STATE_PLAYING;
    }
    if(hasSave && CheckCollisionPointRec(mp, btnCont) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        PlaySound(sndClick);
        StopMusicStream(musMenu); 
        LoadDataFromFile();
        current_state = STATE_PLAYING;
    }
    if(CheckCollisionPointRec(mp, btnQuit) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        PlaySound(sndClick);
        CloseWindow();
        exit(0);
    }
}

static void UpdatePause(void) {
    float sw = GetScreenWidth();
    float sh = GetScreenHeight();
    
    Rectangle btnResume = { sw/2 - 100, sh/2 - 90, 200, 50 };
    Rectangle btnTutorial = { sw/2 - 100, sh/2 - 20, 200, 50 };
    Rectangle btnMenu = { sw/2 - 100, sh/2 + 50, 200, 50 };
    Rectangle btnQuit = { sw/2 - 100, sh/2 + 120, 200, 50 };
    
    Vector2 mp = GetMousePosition();

    if(CheckCollisionPointRec(mp, btnResume) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        PlaySound(sndClick);
        current_state = STATE_PLAYING;
    }
    if(CheckCollisionPointRec(mp, btnTutorial) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        PlaySound(sndClick);
        current_state = STATE_TUTORIAL;
    }
    if(CheckCollisionPointRec(mp, btnMenu) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        PlaySound(sndClick);
        SaveDataToFile();
        current_state = STATE_MENU;
        PlayMusicStream(musMenu); 
    }
    if(CheckCollisionPointRec(mp, btnQuit) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        PlaySound(sndClick);
        SaveDataToFile();
        CloseWindow();
        exit(0);
    }
}

static void UpdateTutorial(void) {
    float sw = GetScreenWidth();
    float sh = GetScreenHeight();
    
    Rectangle btnBack = { sw/2 - 100, sh - 120, 200, 50 };
    Vector2 mp = GetMousePosition();
    
    if(CheckCollisionPointRec(mp, btnBack) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        PlaySound(sndClick);
        current_state = STATE_PAUSE;
    }
}

static void UpdateShop(void) {
    bool bought = false;
    
    if(IsKeyPressed(KEY_ONE) && save_data.player.money >= seedPrices[0]) {
        save_data.player.money -= seedPrices[0]; AddItem(ITEM_SEED_WHEAT, 1); bought = true;
    }
    else if(IsKeyPressed(KEY_TWO) && save_data.player.money >= seedPrices[1]) {
        save_data.player.money -= seedPrices[1]; AddItem(ITEM_SEED_CARROT, 1); bought = true;
    }
    else if(IsKeyPressed(KEY_THREE) && save_data.player.money >= seedPrices[2]) {
        save_data.player.money -= seedPrices[2]; AddItem(ITEM_SEED_PUMPKIN, 1); bought = true;
    }
    else if(IsKeyPressed(KEY_FOUR) && save_data.player.money >= 50) {
        save_data.player.money -= 50; AddItem(ITEM_SCARECROW, 1); bought = true;
    }
    else if(IsKeyPressed(KEY_FIVE) && save_data.player.money >= 10) {
        save_data.player.money -= 10; AddItem(ITEM_FENCE, 1); bought = true;
    }
    else if(IsKeyPressed(KEY_SIX) && save_data.player.money >= 20) {
        save_data.player.money -= 20; AddItem(ITEM_FERTILIZER, 1); bought = true;
    }
    
    if (bought) PlaySound(sndBuy); 

    if(IsKeyPressed(KEY_V)) {
        bool soldSomething = false;
        int totalEarned = 0;
        
        for(int i=0; i<MAX_INVENTORY; i++) {
            ItemType it = save_data.player.inventory[i].type;
            int qty = save_data.player.inventory[i].qty;
            if(qty <= 0) continue;

            int cropIdx = -1;
            if(it == ITEM_CROP_WHEAT) cropIdx = 0;
            else if(it == ITEM_CROP_CARROT) cropIdx = 1;
            else if(it == ITEM_CROP_PUMPKIN) cropIdx = 2;

            if(cropIdx != -1) {
                int price = basePrices[cropIdx];
                int earningsForThisCrop = price * qty; 
                
                totalEarned += earningsForThisCrop;
                save_data.player.inventory[i].qty = 0;
                save_data.player.inventory[i].type = ITEM_NONE;
                soldSomething = true;
            }
        }
        if(soldSomething) {
            if(save_data.player.level >= 6) {
                totalEarned = (int)(totalEarned * 1.2f);
            }
            save_data.player.money += totalEarned;
            AddXP(totalEarned / 10); 
            PlaySound(sndSell); 
            SpawnParticles(save_data.player.position.x, save_data.player.position.y, 15, GOLD);
            
            if(save_data.tutorialStep == 5) {
                save_data.tutorialStep++;
                save_data.tutorialDone = true;
            }
        }
    }
}

void UpdateGame(void) {
    if(current_state == STATE_MENU) {
        UpdateMusicStream(musMenu);
    }

    if(IsKeyPressed(KEY_F11)) {
        ToggleFullscreen();
    }
    if(IsWindowResized()) {
        cam.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
    }

    if(IsKeyPressed(KEY_ESCAPE)) {
        if(current_state == STATE_INVENTORY || current_state == STATE_SHOP) {
            current_state = STATE_PLAYING;
        } else if(current_state == STATE_PLAYING) {
            current_state = STATE_PAUSE;
        } else if(current_state == STATE_PAUSE) {
            current_state = STATE_PLAYING;
        } else if(current_state == STATE_TUTORIAL) {
            current_state = STATE_PAUSE;
        }
    }

    if(current_state == STATE_MENU) { UpdateMenu(); return; }
    if(current_state == STATE_PAUSE) { UpdatePause(); return; }
    if(current_state == STATE_TUTORIAL) { UpdateTutorial(); return; }
    if(current_state == STATE_SHOP) { UpdateShop(); return; }
    
    if(IsKeyPressed(KEY_TAB)) {
        if(current_state == STATE_PLAYING) current_state = STATE_INVENTORY;
        else if(current_state == STATE_INVENTORY) current_state = STATE_PLAYING;
    }
    
    if(current_state == STATE_INVENTORY) return;
    
    float dt = GetFrameTime();
    
    UpdateDayWeather(dt);
    if(save_data.player.level >= 9 && IsKeyPressed(KEY_R) && !save_data.rainInvoked) {
        save_data.weather = WEATHER_RAINY;
        save_data.rainInvoked = true;
    }
    
    cam.zoom += GetMouseWheelMove() * 0.1f;
    if(cam.zoom < 0.5f) cam.zoom = 0.5f;
    if(cam.zoom > 2.0f) cam.zoom = 2.0f;
    
    cam.target.x += (save_data.player.position.x - cam.target.x) * 0.08f;
    cam.target.y += (save_data.player.position.y - cam.target.y) * 0.08f;
    
    if(IsKeyPressed(KEY_ONE)) save_data.player.activeTool = TOOL_HOE;
    if(IsKeyPressed(KEY_TWO)) save_data.player.activeTool = TOOL_WATERCAN;
    if(IsKeyPressed(KEY_THREE)) save_data.player.activeTool = TOOL_SEEDS;
    if(IsKeyPressed(KEY_FOUR)) save_data.player.activeTool = TOOL_HARVEST;
    
    save_data.player.isMoving = false;
    Vector2 nextPos = save_data.player.position;
    
    if(IsKeyDown(KEY_W)) { nextPos.y -= save_data.player.speed * dt; save_data.player.direction = 1; save_data.player.isMoving = true; }
    if(IsKeyDown(KEY_S)) { nextPos.y += save_data.player.speed * dt; save_data.player.direction = 0; save_data.player.isMoving = true; }
    if(IsKeyDown(KEY_A)) { nextPos.x -= save_data.player.speed * dt; save_data.player.direction = 3; save_data.player.isMoving = true; }
    if(IsKeyDown(KEY_D)) { nextPos.x += save_data.player.speed * dt; save_data.player.direction = 2; save_data.player.isMoving = true; }
    
    if(save_data.player.isMoving) {
        save_data.player.frameTimer += dt;
        if(save_data.player.frameTimer > 0.15f) {
            save_data.player.frameTimer = 0.0f;
            save_data.player.currentFrame++;
            if(save_data.player.currentFrame > 3) save_data.player.currentFrame = 0; 
            
            if(save_data.player.currentFrame == 1 || save_data.player.currentFrame == 3) {
                PlaySound(sndStep);
            }
        }
    } else {
        save_data.player.currentFrame = 0; 
    }
    
    if(save_data.tutorialStep == 0 && save_data.player.isMoving) {
        save_data.tutorialStep++;
    }
    
    if(nextPos.x < 0) nextPos.x = 0;
    if(nextPos.y < 0) nextPos.y = 0;
    if(nextPos.x > MAP_SIZE*TILE_SIZE - TILE_SIZE) nextPos.x = MAP_SIZE*TILE_SIZE - TILE_SIZE;
    if(nextPos.y > MAP_SIZE*TILE_SIZE - TILE_SIZE) nextPos.y = MAP_SIZE*TILE_SIZE - TILE_SIZE;
    
    bool col = false;
    Rectangle pRect = { nextPos.x + 16, nextPos.y + 32, 32, 32 };
    
    for(int i=0; i<MAX_OBJECTS; i++) {
        if(save_data.objects[i].active) {
            int ox = save_data.objects[i].x * TILE_SIZE;
            int oy = save_data.objects[i].y * TILE_SIZE;
            Rectangle fRect = {0};
            
            if(save_data.objects[i].type == OBJ_FENCE_V) {
                fRect = (Rectangle){ ox + 20, oy, 24, TILE_SIZE };
            }
            else if(save_data.objects[i].type == OBJ_FENCE_H) {
                fRect = (Rectangle){ ox, oy + 20, TILE_SIZE, 24 };
            }
            else if(save_data.objects[i].type == OBJ_SCARECROW) {
                fRect = (Rectangle){ ox + 16, oy + 10, 32, 50 };
            }
            
            if(fRect.width > 0 && CheckCollisionRecs(pRect, fRect)) { 
                col = true; 
                break; 
            }
        }
    }
    if(!col) save_data.player.position = nextPos;
    
    Vector2 wMouse = GetScreenToWorld2D(GetMousePosition(), cam);
    UpdatePlayerInteractions(wMouse.x, wMouse.y);

    int px_tile = save_data.player.position.x / TILE_SIZE;
    int py_tile = save_data.player.position.y / TILE_SIZE;
    
    if(px_tile < 3 && py_tile < 3 && IsKeyPressed(KEY_E)) {
        current_state = STATE_SHOP;
    }
    
    for(int y=0; y<MAP_SIZE; y++) {
        for(int x=0; x<MAP_SIZE; x++) {
            Cell *c = &save_data.farm.cells[y][x];
            if(c->stage > STAGE_EMPTY && c->stage < STAGE_MATURE && c->watered) {
                c->growTimer += dt;
                if(c->growTimer >= c->growInterval) {
                    c->growTimer = 0.0f;
                    c->stage++;
                }
            }
        }
    }
    
    for(int i=0; i<MAX_PARTICLES; i++) {
        if(particles[i].active) {
            particles[i].pos.x += particles[i].vel.x * dt;
            particles[i].pos.y += particles[i].vel.y * dt;
            particles[i].life -= dt;
            if(particles[i].life <= 0.0f) particles[i].active = false;
            
            if(save_data.weather == WEATHER_STORMY && particles[i].color.r == GRAY.r) {
                if(particles[i].life <= 0) SpawnParticles(cam.target.x + GetRandomValue(-400,400), cam.target.y - 300, 1, GRAY);
            }
        }
    }
    if(save_data.weather == WEATHER_STORMY && GetRandomValue(1,10)>5) {
        SpawnParticles(cam.target.x + GetRandomValue(-400,400), cam.target.y - 300, 2, GRAY);
    }
    
    if(levelUpAnimTimer > 0) levelUpAnimTimer -= dt;
    hoverAlphaTime += dt * 5.0f;
}

static void DrawCell(int x, int y) {
    Cell c = save_data.farm.cells[y][x];
    Rectangle dest = { x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE };
    
    Rectangle srcBase = { 0, 0, 64, 64 }; 
    if (x < 3 && y < 3) {
        srcBase = (Rectangle){ 192, 0, 64, 64 }; 
    }
    DrawTexturePro(texTiles, srcBase, dest, (Vector2){0,0}, 0.0f, WHITE);
    
    if(c.tilled) {
        Rectangle srcTill = { 64, 0, 64, 64 };
        Color tColor = c.watered ? DARKBROWN : WHITE;
        DrawTexturePro(texTiles, srcTill, dest, (Vector2){0,0}, 0.0f, tColor);
    }
    
    if(c.stage > STAGE_EMPTY) {
        int row = 0;
        if(c.type == CROP_WHEAT) row = 1;
        if(c.type == CROP_CARROT) row = 2;
        if(c.type == CROP_PUMPKIN) row = 3;
        
        int col = c.stage - 1; 
        Rectangle cSrc = { col*64, row*64, 64, 64 };
        DrawTexturePro(texTiles, cSrc, dest, (Vector2){0,0}, 0.0f, WHITE);
    }
}

static void DrawParticles(void) {
    for(int i=0; i<MAX_PARTICLES; i++) {
        if(particles[i].active) {
            float alpha = particles[i].life / particles[i].maxLife;
            Color c = particles[i].color;
            c.a = (unsigned char)(255 * alpha);
            DrawCircleV(particles[i].pos, particles[i].size, c);
        }
    }
}

static void DrawUI(void) {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    
    DrawRectangle(0, sh - 80, sw, 80, Fade(BLACK, 0.7f));
    
    const char* wStr = "INSORIT";
    Color wCol = YELLOW;
    if(save_data.weather == WEATHER_CLOUDY) { wStr="INNORAT"; wCol=LIGHTGRAY; }
    if(save_data.weather == WEATHER_RAINY) { wStr="PLOUA"; wCol=BLUE; }
    if(save_data.weather == WEATHER_STORMY) { wStr="FURTUNA"; wCol=PURPLE; }
    
    DrawText(TextFormat("ZIUA %d", save_data.currentDay), 20, sh-60, 20, WHITE);
    DrawText(wStr, 120, sh-60, 20, wCol);
    DrawRectangle(100, sh-60, 15, 15, wCol);
    
    DrawText(TextFormat("BANI: %dG", save_data.player.money), sw - 200, sh-60, 20, GOLD);
    
    int req = save_data.player.level * save_data.player.level * 50;
    float xpPerc = (float)save_data.player.xp / (float)req;
    DrawRectangle(250, sh-50, 400, 20, DARKGRAY);
    DrawRectangle(250, sh-50, (int)(400*xpPerc), 20, GREEN);
    DrawText(TextFormat("LVL %d", save_data.player.level), 660, sh-50, 20, WHITE);
    
    if(levelUpAnimTimer > 0) {
        DrawText("LEVEL UP!", sw/2 - 100, sh/2 - 100, 40, Fade(GOLD, levelUpAnimTimer/2.0f));
    }
    
    if(!save_data.tutorialDone && save_data.tutorialStep < 6) {
        DrawText(tutTexts[save_data.tutorialStep], sw/2 - MeasureText(tutTexts[save_data.tutorialStep], 20)/2, sh-120, 20, ORANGE);
    }
    
    const char* tools[] = {"Sapa", "Stropitoare", "Seminte/Obj", "Mana Goala (Recolta)"};
    DrawText(TextFormat("Tool: %s", tools[save_data.player.activeTool]), 20, 20, 20, RAYWHITE);
    
    if(save_data.player.activeTool == TOOL_SEEDS) {
        int sel = save_data.player.selectedItemIdx;
        if(save_data.player.inventory[sel].type == ITEM_FENCE) {
            DrawText("[R] Roteste Gard", 20, 80, 15, ORANGE);
        }
    }
    
    DrawText("[TAB] Inventar  [ESC] Meniu Pauza", 20, 50, 15, GRAY);
}

static void DrawTranslucentButton(Rectangle r, const char* text) {
    bool hover = CheckCollisionPointRec(GetMousePosition(), r);
    Color bColor = hover ? (Color){10, 10, 10, 220} : (Color){30, 30, 30, 150};
    DrawRectangleRounded(r, 0.5f, 10, bColor);
    
    int tW = MeasureText(text, 20);
    DrawText(text, r.x + r.width/2 - tW/2, r.y + 15, 20, WHITE);
}

void DrawGame(void) {
    BeginDrawing();
    float sw = GetScreenWidth();
    float sh = GetScreenHeight();
    
    if(current_state == STATE_MENU) {
        ClearBackground(BLACK);
        
        DrawTexturePro(texBg, 
            (Rectangle){0, 0, texBg.width, texBg.height}, 
            (Rectangle){0, 0, sw, sh}, 
            (Vector2){0,0}, 0.0f, WHITE);
            
        DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.3f));
        
        DrawText("FARMING SIMULATOR", sw/2 - MeasureText("FARMING SIMULATOR", 40)/2, 100, 40, WHITE);
        DrawText("[F11] Toggle Fullscreen", 20, 20, 20, LIGHTGRAY);
        
        Rectangle btnNew = { sw/2 - 100, sh/2 - 60, 200, 50 };
        Rectangle btnCont = { sw/2 - 100, sh/2 + 10, 200, 50 };
        Rectangle btnQuit = { sw/2 - 100, sh/2 + 80, 200, 50 };
        
        DrawTranslucentButton(btnNew, "NEW GAME");
        
        FILE *f = fopen("savegame.dat", "rb");
        if(f) {
            fclose(f);
            DrawTranslucentButton(btnCont, "CONTINUE");
        } else {
            DrawRectangleRounded(btnCont, 0.5f, 10, (Color){50, 50, 50, 100});
            DrawText("CONTINUE", btnCont.x + btnCont.width/2 - MeasureText("CONTINUE", 20)/2, btnCont.y + 15, 20, GRAY);
        }
        
        DrawTranslucentButton(btnQuit, "QUIT");
        
        EndDrawing();
        return;
    }
    
    ClearBackground(BLACK);
    
    float bgScale = (float)GetScreenHeight() / (float)texBg.height;
    float scaledW = texBg.width * bgScale;
    float scaledH = texBg.height * bgScale;
    
    float parallaxFactor = 0.15f; 
    float pX = -cam.target.x * parallaxFactor;
    float pY = -cam.target.y * parallaxFactor;
    
    pX = fmodf(pX, scaledW);
    pY = fmodf(pY, scaledH);
    
    if (pX > 0.0f) pX -= scaledW;
    if (pY > 0.0f) pY -= scaledH;
    
    Color bgTint = Fade(WHITE, 0.45f);
    
    int cols = (sw / scaledW) + 2;
    int rows = (sh / scaledH) + 2;
    
    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < rows; j++) {
            DrawTextureEx(texBg, (Vector2){ pX + i * scaledW, pY + j * scaledH }, 0.0f, bgScale, bgTint);
        }
    }

    BeginMode2D(cam);
        for(int y=0; y<MAP_SIZE; y++) {
            for(int x=0; x<MAP_SIZE; x++) {
                DrawCell(x, y);
            }
        }
        
        DrawText("MAGAZIN\n[E] ACCES", 30, 60, 20, WHITE);
        
        for(int i=0; i<MAX_OBJECTS; i++) {
            if(save_data.objects[i].active) {
                int px = save_data.objects[i].x * TILE_SIZE;
                int py = save_data.objects[i].y * TILE_SIZE;
                
                if(save_data.objects[i].type == OBJ_FENCE_V) {
                    DrawRectangle(px+20, py, 24, TILE_SIZE, DARKBROWN); 
                }
                else if(save_data.objects[i].type == OBJ_FENCE_H) {
                    DrawRectangle(px, py+20, TILE_SIZE, 24, DARKBROWN); 
                }
                else if(save_data.objects[i].type == OBJ_SCARECROW) {
                    DrawRectangle(px+24, py+10, 16, 50, BROWN); 
                    float armOff = sinf((float)GetTime() * 5.0f) * 10.0f;
                    DrawLineEx((Vector2){px+32, py+30}, (Vector2){px+10, py+30+armOff}, 4, BROWN);
                    DrawLineEx((Vector2){px+32, py+30}, (Vector2){px+54, py+30-armOff}, 4, BROWN);
                }
            }
        }
        
        Rectangle pSrc = { 
            save_data.player.currentFrame * 64.0f, 
            save_data.player.direction * 64.0f, 
            64.0f, 64.0f 
        };
        DrawTextureRec(texPlayer, pSrc, save_data.player.position, WHITE);
        
        Vector2 m = GetScreenToWorld2D(GetMousePosition(), cam);
        int hx = (int)(m.x / TILE_SIZE);
        int hy = (int)(m.y / TILE_SIZE);
        
        if(hx >= 0 && hx < MAP_SIZE && hy >= 0 && hy < MAP_SIZE) {
            Color hc = YELLOW;
            hc.a = (unsigned char)(128 + sinf(hoverAlphaTime)*127);
            DrawRectangleLinesEx((Rectangle){hx*TILE_SIZE, hy*TILE_SIZE, TILE_SIZE, TILE_SIZE}, 3, hc);
            
            if(save_data.player.activeTool == TOOL_SEEDS) {
                int sel = save_data.player.selectedItemIdx;
                if(save_data.player.inventory[sel].type == ITEM_FENCE) {
                    Color ghostCol = Fade(DARKBROWN, 0.5f);
                    if (currentFenceRotation == 0) {
                        DrawRectangle(hx*TILE_SIZE + 20, hy*TILE_SIZE, 24, TILE_SIZE, ghostCol);
                    } else {
                        DrawRectangle(hx*TILE_SIZE, hy*TILE_SIZE + 20, TILE_SIZE, 24, ghostCol);
                    }
                }
            }
        }
        
        if(save_data.weather == WEATHER_RAINY || save_data.weather == WEATHER_STORMY) {
            for(int i=0; i<100; i++) {
                int rx = GetRandomValue((int)(cam.target.x - sw), (int)(cam.target.x + sw));
                int ry = GetRandomValue((int)(cam.target.y - sh), (int)(cam.target.y + sh));
                DrawLine(rx, ry, rx-5, ry+10, Fade(BLUE, 0.4f));
            }
        }
        
        DrawParticles();
    EndMode2D();
    
    float t = save_data.timeOfDay;
    if(t < 75.0f) DrawRectangle(0,0,sw,sh, (Color){255,150,0,60}); 
    else if(t > 150.0f && t <= 225.0f) DrawRectangle(0,0,sw,sh, (Color){200,50,0,80}); 
    else if(t > 225.0f) DrawRectangle(0,0,sw,sh, (Color){0,0,50,150}); 
    
    DrawUI();
    
    if(current_state == STATE_INVENTORY) {
        DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.8f));
        int startX = sw/2 - 200;
        int startY = sh/2 - 160;
        DrawText("INVENTAR (Apasa ESC pt a iesi)", startX, startY-40, 20, WHITE);
        
        for(int i=0; i<20; i++) {
            int rx = startX + (i%5) * 80;
            int ry = startY + (i/5) * 80;
            
            Rectangle slot = { rx, ry, 70, 70 };
            bool hov = CheckCollisionPointRec(GetMousePosition(), slot);
            
            DrawRectangleRec(slot, hov ? LIGHTGRAY : GRAY);
            if(hov || save_data.player.selectedItemIdx == i) DrawRectangleLinesEx(slot, 3, YELLOW);
            
            if(save_data.player.inventory[i].qty > 0) {
                DrawText(GetItemName(save_data.player.inventory[i].type), rx+4, ry+8, 10, BLACK);
                DrawText(TextFormat("x%d", save_data.player.inventory[i].qty), rx+5, ry+45, 20, WHITE);
            }
            
            if(hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlaySound(sndClick);
                save_data.player.selectedItemIdx = i;
                save_data.player.activeTool = TOOL_SEEDS;
            }
        }
    }

    if(current_state == STATE_PAUSE) {
        DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.6f)); 
        DrawText("JOC IN PAUZA", sw/2 - MeasureText("JOC IN PAUZA", 40)/2, sh/2 - 160, 40, WHITE);
        
        Rectangle btnResume = { sw/2 - 100, sh/2 - 90, 200, 50 };
        Rectangle btnTutorial = { sw/2 - 100, sh/2 - 20, 200, 50 };
        Rectangle btnMenu = { sw/2 - 100, sh/2 + 50, 200, 50 };
        Rectangle btnQuit = { sw/2 - 100, sh/2 + 120, 200, 50 };
        
        DrawTranslucentButton(btnResume, "RESUME");
        DrawTranslucentButton(btnTutorial, "TUTORIAL");
        DrawTranslucentButton(btnMenu, "MAIN MENU");
        DrawTranslucentButton(btnQuit, "QUIT DESKTOP");
    }

    if(current_state == STATE_TUTORIAL) {
        DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.9f));
        DrawText("CUM SE JOACA", sw/2 - MeasureText("CUM SE JOACA", 30)/2, 80, 30, GOLD);
        
        for(int i=0; i<6; i++) {
            DrawText(tutTexts[i], sw/2 - 350, 150 + i*40, 20, WHITE);
        }
        
        Rectangle btnBack = { sw/2 - 100, sh - 120, 200, 50 };
        DrawTranslucentButton(btnBack, "INAPOI");
    }
    
    if(current_state == STATE_SHOP) {
        DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.85f));
        DrawText("MAGAZIN LOCAL (Apasa ESC pt a iesi)", sw/2 - MeasureText("MAGAZIN LOCAL (Apasa ESC pt a iesi)", 30)/2, 80, 30, WHITE);
        DrawText(TextFormat("BANI CURENTI: %dG", save_data.player.money), sw/2 - 100, 130, 20, GOLD);
        
        DrawText(TextFormat("[1] Cumpara Seminte Grau - %dG", seedPrices[0]), sw/2 - 200, 200, 20, WHITE);
        DrawText(TextFormat("[2] Cumpara Seminte Morcov - %dG", seedPrices[1]), sw/2 - 200, 240, 20, WHITE);
        DrawText(TextFormat("[3] Cumpara Seminte Dovleac - %dG", seedPrices[2]), sw/2 - 200, 280, 20, WHITE);
        DrawText("[4] Cumpara Sperietoare - 50G", sw/2 - 200, 320, 20, WHITE);
        DrawText("[5] Cumpara Gard - 10G", sw/2 - 200, 360, 20, WHITE);
        DrawText("[6] Cumpara Ingrasamant - 20G", sw/2 - 200, 400, 20, WHITE);
        
        DrawText("[V] VINDE TOATE RECOLTELE DIN INVENTAR", sw/2 - 200, 450, 20, GREEN);
        DrawText(TextFormat("Preturi vanzare: Grau %dG | Morcov %dG | Dovleac %dG", basePrices[0], basePrices[1], basePrices[2]), sw/2 - 200, 480, 15, GRAY);
    }
    
    EndDrawing();
}

void CloseGame(void) {
    if(current_state != STATE_MENU) SaveDataToFile();
    
    UnloadSound(sndHoe);
    UnloadSound(sndWater);
    UnloadSound(sndPlant);
    UnloadSound(sndHarvest);
    UnloadSound(sndBuy);
    UnloadSound(sndSell);
    UnloadSound(sndStep);
    UnloadSound(sndClick);
    UnloadMusicStream(musMenu);
}