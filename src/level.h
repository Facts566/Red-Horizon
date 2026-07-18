#pragma once
#include <raylib.h>

struct Level {
    int width;
    int height;
    float tileSize;
    float wallHeight;
    char *data;
    Vector3 playerStart;
};

Level LoadLevel(const char *path, float tileSize, float wallHeight);
void DrawLevel(Level level, Texture2D floorTex, Texture2D wallTex, Texture2D greenTex, Shader shader);
void UnloadLevel(Level level);
