#pragma once
#include <raylib.h>
#include "level.h"

constexpr int MENU_MAX_ZOMBIES = 4;

struct MenuZombie {
    Vector3 position;
    Vector3 target;
    float yaw;
    float speed;
    float animTimer;
    float stateTimer;
    int animFrame;
    bool moving;
};

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
    Texture2D zombiIdle, zombiWalk1, zombiWalk2;
    MenuZombie zombies[MENU_MAX_ZOMBIES];
    int zombieCount;
    float time;
    bool ready;
};

void InitMenu3D(Menu3D &m);
void DrawMenu3D(Menu3D &m, float dt);
void UnloadMenu3D(Menu3D &m);