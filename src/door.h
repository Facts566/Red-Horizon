#pragma once
#include <raylib.h>

struct Door {
    Vector3 position;
    Vector3 rotationAxis;
    float rotationAngle;
    bool isOpen;
    float triggerRadius;
    Texture2D closedTex;
    Texture2D openTex;
};

Door CreateDoor(Vector3 position, Vector3 rotationAxis, float rotationAngle, Texture2D closedTex, Texture2D openTex, Texture2D capTex, Shader shader);
void UpdateDoor(Door *door, Vector3 playerPos);
void DrawDoor(Door door);
bool CheckDoorCollision(Door door, float x, float z, float radius);
void UnloadDoor();
