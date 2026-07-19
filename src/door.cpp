#include "door.h"
#include "map.h"
#include <math.h>

static Model doorModelClosed = { 0 };
static Model doorModelOpen = { 0 };
static Model doorTopCap = { 0 };

Door CreateDoor(Vector3 position, Vector3 rotationAxis, float rotationAngle, Texture2D closedTex, Texture2D openTex, Texture2D capTex, Shader shader)
{
    float ts = 5.0f;

    doorModelClosed = MakeWall(ts * 2, 3 * ts, 1.0f, -1.0f, closedTex);
    doorModelClosed.materials[0].shader = shader;

    doorModelOpen = MakeWall(ts * 2, 3 * ts, 1.0f, -1.0f, openTex);
    doorModelOpen.materials[0].shader = shader;

    Mesh capMesh = GenMeshCube(ts * 2, ts, ts);
    doorTopCap = LoadModelFromMesh(capMesh);
    doorTopCap.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = capTex;
    doorTopCap.materials[0].shader = shader;

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
    if (door.isOpen)
    {
        DrawModelEx(doorModelOpen, door.position, door.rotationAxis, door.rotationAngle, (Vector3){1,1,1}, WHITE);
    }
    else
    {
        DrawModelEx(doorModelClosed, door.position, door.rotationAxis, door.rotationAngle, (Vector3){1,1,1}, WHITE);
    }

    float ts = 5.0f;
    Vector3 capPos = door.position;
    capPos.y = 3.5f * ts;
    capPos.z += ts / 2.0f;
    DrawModelEx(doorTopCap, capPos, door.rotationAxis, door.rotationAngle, (Vector3){1,1,1}, WHITE);
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
    UnloadModel(doorTopCap);
}
