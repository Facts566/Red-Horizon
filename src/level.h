#pragma once
#include <raylib.h>
#include "config.h"

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
void DrawLevel(Level level, bool drawCeiling = true);
void UnloadLevel(Level level);
bool CheckWallCollision(Level level, float x, float z, float radius);
