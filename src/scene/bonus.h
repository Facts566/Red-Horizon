#pragma once
#include <raylib.h>
#include "config.h"
#include "spawn.h"

struct Bonus {
    Vector3 position;
    Texture2D texture;
    bool active;
    float bobTimer;
    BonusType type;
};

void InitBonuses(Bonus bonuses[], BonusSpawn spawns[], int count, Texture2D medicTex, Texture2D keyTex, float tileSize);
void UpdateBonuses(Bonus bonuses[], int count, Vector3 playerPos, float &health, int maxHealth, bool &hasKey);
void DrawBonuses(Bonus bonuses[], int count, Camera3D camera);
