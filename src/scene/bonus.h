#pragma once
#include <raylib.h>
#include "config.h"
#include "spawn.h"
#include "weapon.h"

struct Bonus {
    Vector3 position;
    Texture2D texture;
    bool active;
    float bobTimer;
    BonusType type;
    int weaponIndex;
    int keyId;
};

void InitBonuses(Bonus bonuses[], BonusSpawn spawns[], int count, Texture2D medicTex, Texture2D keyTex, Texture2D weaponTex, float tileSize);
void UpdateBonuses(Bonus bonuses[], int count, Vector3 playerPos, float &health, int maxHealth, bool hasKeys[], WeaponState weapons[], int weaponCount, Sound itemSound);
void DrawBonuses(Bonus bonuses[], int count, Camera3D camera);
