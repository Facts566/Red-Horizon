#pragma once
#include <raylib.h>
#include "level.h"
#include "door.h"
#include "props.h"

#define ZOMBIE_MAX_PATH 256

struct Zombie {
    Vector3 position;
    float health;
    float speed;
    float radius;

    Vector2 path[ZOMBIE_MAX_PATH];
    int pathCount;
    float pathRecalcTimer;
    int pathIndex;

    Texture2D textureIdle;
    Texture2D textureWalk1;
    Texture2D textureWalk2;
    Texture2D textureDead;
    bool isWalking;
    float animTimer;
    bool animFrame;

    bool active;
};

void InitZombie(Zombie &zombie, Vector3 pos, Texture2D idle, Texture2D walk1, Texture2D walk2, Texture2D dead);
void UpdateZombie(Zombie &zombie, Level level, Door doors[], int doorCount, BoxCollider sofaBox, Vector3 playerPos, float dt);
void DrawZombie(Zombie &zombie, Camera3D camera);
void UnloadZombie(Zombie &zombie);
bool ZombieHitByRay(Zombie &zombie, Vector3 origin, Vector3 dir);
