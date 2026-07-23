#pragma once
#include <raylib.h>

#define MAX_BONUSES 64

struct BonusSpawn {
    int col;
    int row;
};

struct Bonus {
    Vector3 position;
    Texture2D texture;
    bool active;
    float bobTimer;
};

int LoadBonusSpawns(const char *path, BonusSpawn *spawns, int maxSpawns);
void InitBonuses(Bonus bonuses[], BonusSpawn spawns[], int count, Texture2D medicTex, float tileSize);
void UpdateBonuses(Bonus bonuses[], int count, Vector3 playerPos, float &health, int maxHealth);
void DrawBonuses(Bonus bonuses[], int count, Camera3D camera);
