#pragma once
#include <raylib.h>

struct LevelModels {
    Model floor;
    Model wallN, wallS, wallW, wallE;
    Model greenN, greenS, greenW, greenE;
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

Level LoadLevel(const char *path, float tileSize, float wallHeight, Texture2D floorTex, Texture2D wallTex, Texture2D greenTex, Shader shader);
void DrawLevel(Level level);
void UnloadLevel(Level level);
