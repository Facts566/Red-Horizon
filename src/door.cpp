#include "door.h"
#include "map.h"
#include <math.h>

static Model doorModelClosed = { 0 };
static Model doorModelOpen = { 0 };
static Model doorCapLeft = { 0 };
static Model doorCapRight = { 0 };
static Model doorDecalModel = { 0 };
static bool doorModelsLoaded = false;

Door CreateDoor(Vector3 position, Vector3 rotationAxis, float rotationAngle, Texture2D closedTex, Texture2D openTex, Texture2D capLeftTex, Texture2D capRightTex, Shader shader, Texture2D shotholeTex)
{
    float ts = 5.0f;

    if (!doorModelsLoaded)
    {
        doorModelClosed = MakeWall(ts * 2, 3 * ts, 1.0f, -1.0f, closedTex);
        doorModelClosed.materials[0].shader = shader;

        doorModelOpen = MakeWall(ts * 2, 3 * ts, 1.0f, -1.0f, openTex);
        doorModelOpen.materials[0].shader = shader;

        Mesh capMeshLeft = GenMeshCube(ts * 2, ts, ts);
        doorCapLeft = LoadModelFromMesh(capMeshLeft);
        doorCapLeft.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = capLeftTex;
        doorCapLeft.materials[0].shader = shader;

        Mesh capMeshRight = GenMeshCube(ts * 2, ts, ts);
        doorCapRight = LoadModelFromMesh(capMeshRight);
        doorCapRight.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = capRightTex;
        doorCapRight.materials[0].shader = shader;

        doorDecalModel = MakeWall(0.6f, 0.6f, 1.0f, 1.0f, shotholeTex);
        doorDecalModel.materials[0].shader = shader;

        doorModelsLoaded = true;
    }

    Door door = { 0 };
    door.position = position;
    door.rotationAxis = rotationAxis;
    door.rotationAngle = rotationAngle;
    door.isOpen = false;
    door.triggerRadius = 8.0f;
    door.closedTex = closedTex;
    door.openTex = openTex;
    door.capLeftTex = capLeftTex;
    door.capRightTex = capRightTex;
    return door;
}

void UpdateDoors(Door doors[], int count, Vector3 playerPos)
{
    for (int i = 0; i < count; i++)
    {
        float dx = playerPos.x - doors[i].position.x;
        float dz = playerPos.z - doors[i].position.z;
        float dist = sqrtf(dx * dx + dz * dz);
        doors[i].isOpen = dist < doors[i].triggerRadius;
    }
}

void DrawDoors(Door doors[], int count)
{
    float ts = 5.0f;

    for (int i = 0; i < count; i++)
    {
        if (doors[i].isOpen)
        {
            DrawModelEx(doorModelOpen, doors[i].position, doors[i].rotationAxis, doors[i].rotationAngle, (Vector3){1,1,1}, WHITE);
        }
        else
        {
            DrawModelEx(doorModelClosed, doors[i].position, doors[i].rotationAxis, doors[i].rotationAngle, (Vector3){1,1,1}, WHITE);

            for (auto &bh : doors[i].bulletHoles)
            {
                Vector3 p = bh.pos;
                p.y -= 0.3f;
                if (bh.normal.z < 0)
                    DrawModelEx(doorDecalModel, p, (Vector3){0,1,0}, 180.0f, (Vector3){1,1,1}, WHITE);
                else if (bh.normal.z > 0)
                    DrawModel(doorDecalModel, p, 1.0f, WHITE);
                else if (bh.normal.x < 0)
                    DrawModelEx(doorDecalModel, p, (Vector3){0,1,0}, -90.0f, (Vector3){1,1,1}, WHITE);
                else if (bh.normal.x > 0)
                    DrawModelEx(doorDecalModel, p, (Vector3){0,1,0}, 90.0f, (Vector3){1,1,1}, WHITE);
            }
        }

        float rad = doors[i].rotationAngle * 3.14159f / 180.0f;
        float sx = sinf(rad);
        float sz = cosf(rad);

        Vector3 capLeftPos = doors[i].position;
        capLeftPos.y = 3.5f * ts;
        capLeftPos.x -= sx * ts / 2.0f;
        capLeftPos.z -= sz * ts / 2.0f;
        DrawModelEx(doorCapLeft, capLeftPos, doors[i].rotationAxis, doors[i].rotationAngle, (Vector3){1,1,1}, WHITE);

        Vector3 capRightPos = doors[i].position;
        capRightPos.y = 3.5f * ts;
        capRightPos.x += sx * ts / 2.0f;
        capRightPos.z += sz * ts / 2.0f;
        DrawModelEx(doorCapRight, capRightPos, doors[i].rotationAxis, doors[i].rotationAngle, (Vector3){1,1,1}, WHITE);
    }
}

bool CheckAnyDoorCollision(Door doors[], int count, float x, float z, float radius)
{
    for (int i = 0; i < count; i++)
    {
        if (doors[i].isOpen) continue;

        float ts = 5.0f;
        float hw = ts;
        float left = doors[i].position.x - hw;
        float right = doors[i].position.x + hw;
        float top = doors[i].position.z - ts / 2.0f;
        float bottom = doors[i].position.z + ts / 2.0f;

        float closestX = (x < left) ? left : (x > right) ? right : x;
        float closestZ = (z < top) ? top : (z > bottom) ? bottom : z;

        float dx = x - closestX;
        float dz = z - closestZ;

        if (dx * dx + dz * dz < radius * radius)
            return true;
    }
    return false;
}

bool RayDoorIntersect(Door door, Vector3 origin, Vector3 dir, float maxDist, Vector3 &hitPos, Vector3 &hitNormal)
{
    if (fabsf(dir.z) < 0.0001f) return false;

    float ts = 5.0f;
    float doorZ = door.position.z;
    float t = (doorZ - origin.z) / dir.z;
    if (t < 0 || t > maxDist) return false;

    float hx = origin.x + dir.x * t;
    float hy = origin.y + dir.y * t;
    float hw = ts;
    float doorHeight = 3.0f * ts;

    if (hx >= door.position.x - hw && hx <= door.position.x + hw &&
        hy >= 0 && hy <= doorHeight)
    {
        hitPos.x = hx;
        hitPos.y = hy;
        hitPos.z = doorZ;
        hitNormal = (Vector3){0, 0, (dir.z > 0) ? -1.0f : 1.0f};
        return true;
    }
    return false;
}

void UnloadDoors()
{
    UnloadModel(doorModelClosed);
    UnloadModel(doorModelOpen);
    UnloadModel(doorCapLeft);
    UnloadModel(doorCapRight);
    UnloadModel(doorDecalModel);
}
