#pragma once
#include "config.h"

struct ZombieSpawn {
    int col;
    int row;
    bool isMilitary;
};

int LoadZombieSpawns(const char *path, ZombieSpawn *spawns, int maxSpawns);

enum BonusType { BONUS_HEALTH, BONUS_KEY };

struct BonusSpawn {
    int col;
    int row;
    BonusType type;
};

int LoadBonusSpawns(const char *path, BonusSpawn *spawns, int maxSpawns);
