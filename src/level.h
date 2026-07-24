#pragma once
#include <raylib.h>

struct LevelModels {
    Model floor;
    Model planks;
    Model ceiling;
    Model wallN, wallS, wallW, wallE;
    Model greenN, greenS, greenW, greenE;
    Model whiteN, whiteS, whiteW, whiteE;
};

struct Level {
    int width;
    int height;
    float tileSize;
    float wallHeight;
    char *data;
    Vector3 playerStart;
    LevelModels models;
};

Level LoadLevel(const char *path, float tileSize, float wallHeight, Texture2D floorTex, Texture2D planksTex, Texture2D wallTex, Texture2D greenTex, Texture2D whiteTex, Shader shader);
void DrawLevel(Level level);
void UnloadLevel(Level level);
bool CheckWallCollision(Level level, float x, float z, float radius);

#define MAX_ZOMBIE_SPAWNS 64

struct ZombieSpawn {
    int col;
    int row;
    bool isMilitary;
};

int LoadZombieSpawns(const char *path, ZombieSpawn *spawns, int maxSpawns);

#define MAX_DOOR_SPAWNS 8

struct DoorSpawn {
    int col;
    int row;
    float rotation;
};

int LoadDoorSpawns(const char *path, DoorSpawn *spawns, int maxSpawns);
