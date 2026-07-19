#pragma once
#include <raylib.h>

struct Sofa {
    Model model;
    Texture2D texture;
    Vector3 position;
    float scale;
};

void LoadSofa(Sofa &sofa, Shader shader, float tileSize);
void DrawSofa(Sofa &sofa);
void UnloadSofa(Sofa &sofa);
