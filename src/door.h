#pragma once
#include <raylib.h>
#include <vector>

struct BulletHole {
    Vector3 pos;
    Vector3 normal;
};

struct Door {
    Vector3 position;
    Vector3 rotationAxis;
    float rotationAngle;
    bool isOpen;
    float triggerRadius;
    Texture2D closedTex;
    Texture2D openTex;
    std::vector<BulletHole> bulletHoles;
};

Door CreateDoor(Vector3 position, Vector3 rotationAxis, float rotationAngle, Texture2D closedTex, Texture2D openTex, Texture2D capTex, Shader shader, Texture2D shotholeTex);
void UpdateDoor(Door *door, Vector3 playerPos);
void DrawDoor(Door door);
bool CheckDoorCollision(Door door, float x, float z, float radius);
bool RayDoorIntersect(Door door, Vector3 origin, Vector3 dir, float maxDist, Vector3 &hitPos, Vector3 &hitNormal);
void UnloadDoor();
