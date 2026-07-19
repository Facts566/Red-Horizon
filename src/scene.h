#pragma once
#include "props.h"
#include "level.h"
#include "zombie.h"

#define SCENE_MAX_ZOMBIES 8

struct Scene {
    float tileSize;

    Sofa sofa;
    BoxCollider sofaBox;

    Lamp lamp;

    Zombie zombies[SCENE_MAX_ZOMBIES];
    int zombieCount;

    // === добавляй новые объекты сюда ===
};

void LoadScene(Scene &scene, Shader shader, float tileSize, Vector3 playerStart);
void DrawScene(Scene &scene, Camera3D camera);
BoxCollider GetSofaCollider(Scene &scene);
void UnloadScene(Scene &scene);
