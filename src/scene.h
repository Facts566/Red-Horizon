#pragma once
#include "props.h"
#include "level.h"

struct Scene {
    float tileSize;

    Sofa sofa;
    BoxCollider sofaBox;

    Lamp lamp;

    // === добавляй новые объекты сюда ===
};

void LoadScene(Scene &scene, Shader shader, float tileSize, Vector3 playerStart);
void DrawScene(Scene &scene);
BoxCollider GetSofaCollider(Scene &scene);
void UnloadScene(Scene &scene);
