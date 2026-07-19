#pragma once
#include <raylib.h>

struct Sofa {
    Model model;
    Texture2D texture;
    Vector3 position;
    float scale;
};

struct Lamp {
    Model model;
    Texture2D texture;
    Vector3 position;
    float scale;
};

struct BoxCollider {
    Vector3 min;
    Vector3 max;
};

void LoadSofa(Sofa &sofa, Shader shader, float tileSize);
void DrawSofa(Sofa &sofa);
void UnloadSofa(Sofa &sofa);

void LoadLamp(Lamp &lamp, Shader shader, float tileSize);
void DrawLamp(Lamp &lamp);
void UnloadLamp(Lamp &lamp);

BoxCollider MakeSofaCollider(Sofa &sofa);
bool CheckBoxCollision(BoxCollider box, float x, float z, float radius);
