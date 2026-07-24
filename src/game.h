#pragma once
#include <raylib.h>
#include <vector>
#include "level.h"
#include "scene.h"
#include "weapon.h"
#include "door.h"
#include "spawn.h"
#include "bonus.h"

struct Game {
    Level level;
    Scene scene;
    Camera3D camera;
    Shader shader;

    WeaponState weapons[WEAPON_COUNT];
    int currentWeapon;
    std::vector<BulletHole> wallHoles;

    ZombieSpawn zombieSpawns[MAX_ZOMBIE_SPAWNS];
    int zombieSpawnCount;

    BonusSpawn bonusSpawns[MAX_BONUSES];
    Bonus bonuses[MAX_BONUSES];
    int bonusCount;

    bool hasKey;
    float yaw;

    int maxHealth;
    float health;
    float hitFlash;
    float hitShakeTime;
    float touchTimer;
    bool gameOver;
    bool showWeaponPanel;

    int lightRangeLoc;
    int lightAmbLoc;
    int lightPosLoc;

    Texture2D zombiIdle, zombiWalk1, zombiWalk2, zombiDead;
    Texture2D milIdle, milWalk1, milWalk2, milDead;
    Texture2D shotholeTex, medicTex, keyTex;
    Texture2D floorTex, wallTex, greenTex, planksTex, whiteWallTex;
};

void InitGame(Game &game);
void ResetGame(Game &game);
void UpdateGame(Game &game);
void DrawGame(Game &game);
void UnloadGame(Game &game);
