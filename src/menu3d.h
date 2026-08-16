#pragma once
#include <raylib.h>
#include "level.h"

struct Menu3D {
    Level level;
    Camera3D camera;
    Shader lightShader;
    Shader blurShader;
    RenderTexture2D sceneRT;
    RenderTexture2D blurRT;
    int blurTexelLoc;
    int blurDirLoc;
    Texture2D floorTex, planksTex, brickTex, whiteTex;
    float time;
    bool ready;
};

void InitMenu3D(Menu3D &m);
void DrawMenu3D(Menu3D &m, float dt);
void UnloadMenu3D(Menu3D &m);