#pragma once
#include <raylib.h>

#define MAX_BONUSES 64

enum BonusType { BONUS_HEALTH, BONUS_KEY };

struct BonusSpawn {
    int col;
    int row;
    BonusType type;
};

struct Bonus {
    Vector3 position;
    Texture2D texture;
    bool active;
    float bobTimer;
    BonusType type;
};

int LoadBonusSpawns(const char *path, BonusSpawn *spawns, int maxSpawns);
void InitBonuses(Bonus bonuses[], BonusSpawn spawns[], int count, Texture2D medicTex, Texture2D keyTex, float tileSize);
void UpdateBonuses(Bonus bonuses[], int count, Vector3 playerPos, float &health, int maxHealth, bool &hasKey);
void DrawBonuses(Bonus bonuses[], int count, Camera3D camera);
