#pragma once
#include <raylib.h>
#include <vector>

struct BulletHole {
    Vector3 pos;
    Vector3 normal;
};

const int MAX_DOORS = 8;

struct Door {
    Vector3 position;
    Vector3 rotationAxis;
    float rotationAngle;
    bool isOpen;
    float triggerRadius;
    Texture2D closedTex;
    Texture2D openTex;
    Texture2D capLeftTex;
    Texture2D capRightTex;
    std::vector<BulletHole> bulletHoles;
};

Door CreateDoor(Vector3 position, Vector3 rotationAxis, float rotationAngle, Texture2D closedTex, Texture2D openTex, Texture2D capLeftTex, Texture2D capRightTex, Shader shader, Texture2D shotholeTex);
void UpdateDoors(Door doors[], int count, Vector3 positions[], int posCount);
void DrawDoors(Door doors[], int count);
bool CheckAnyDoorCollision(Door doors[], int count, float x, float z, float radius);
bool RayDoorIntersect(Door door, Vector3 origin, Vector3 dir, float maxDist, Vector3 &hitPos, Vector3 &hitNormal);
void UnloadDoors();
