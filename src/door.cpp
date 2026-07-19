#include "door.h"
#include "map.h"
#include <math.h>

static Model doorModelClosed = { 0 };
static Model doorModelOpen = { 0 };

Door CreateDoor(Vector3 position, Vector3 rotationAxis, float rotationAngle, Texture2D closedTex, Texture2D openTex, Shader shader)
{
    float ts = 5.0f;

    doorModelClosed = MakeWall(ts * 2, 3 * ts, 1.0f, (3 * ts) / (ts * 2), closedTex);
    doorModelClosed.materials[0].shader = shader;

    doorModelOpen = MakeWall(ts * 2, 3 * ts, 1.0f, (3 * ts) / (ts * 2), openTex);
    doorModelOpen.materials[0].shader = shader;

    Door door = { 0 };
    door.position = position;
    door.rotationAxis = rotationAxis;
    door.rotationAngle = rotationAngle;
    door.isOpen = false;
    door.triggerRadius = 8.0f;
    door.closedTex = closedTex;
    door.openTex = openTex;
    return door;
}

void UpdateDoor(Door *door, Vector3 playerPos)
{
    float dx = playerPos.x - door->position.x;
    float dz = playerPos.z - door->position.z;
    float dist = sqrtf(dx * dx + dz * dz);
    door->isOpen = dist < door->triggerRadius;
}

void DrawDoor(Door door)
{
    Model *m = door.isOpen ? &doorModelOpen : &doorModelClosed;
    DrawModelEx(*m, door.position, door.rotationAxis, door.rotationAngle, (Vector3){1,1,1}, WHITE);
}

bool CheckDoorCollision(Door door, float x, float z, float radius)
{
    if (door.isOpen) return false;

    float ts = 5.0f;
    float hw = ts;
    float left = door.position.x - hw;
    float right = door.position.x + hw;
    float top = door.position.z - ts / 2.0f;
    float bottom = door.position.z + ts / 2.0f;

    float closestX = (x < left) ? left : (x > right) ? right : x;
    float closestZ = (z < top) ? top : (z > bottom) ? bottom : z;

    float dx = x - closestX;
    float dz = z - closestZ;

    return dx * dx + dz * dz < radius * radius;
}

void UnloadDoor()
{
    UnloadModel(doorModelClosed);
    UnloadModel(doorModelOpen);
}
