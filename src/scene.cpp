#include "scene.h"
#include <rlgl.h>
#include <cstring>
#include <cmath>
#include <raymath.h>
#include <algorithm>

void LoadScene(Scene &scene, Shader shader, float tileSize, Vector3 playerStart, Texture2D greenTex, Texture2D wallTex, Texture2D shotholeTex, Texture2D whiteTex)
{
    scene.tileSize = tileSize;

    scene.doorTexClosed = LoadTexture("tex/door/door_1.png");
    SetTextureFilter(scene.doorTexClosed, TEXTURE_FILTER_POINT);
    SetTextureWrap(scene.doorTexClosed, TEXTURE_WRAP_REPEAT);
    rlTextureParameters(scene.doorTexClosed.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
    rlTextureParameters(scene.doorTexClosed.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);

    scene.doorTexOpen = LoadTexture("tex/door/door_2.png");
    SetTextureFilter(scene.doorTexOpen, TEXTURE_FILTER_POINT);
    SetTextureWrap(scene.doorTexOpen, TEXTURE_WRAP_REPEAT);
    rlTextureParameters(scene.doorTexOpen.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
    rlTextureParameters(scene.doorTexOpen.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);

    //decor
    AddObject(scene, "sofa", {playerStart.x + 0.0f, 0.7f * tileSize, playerStart.z + -84.0f}, 0.0f, 4.0f, true, shader);
    AddObject(scene, "sofa", {playerStart.x + 82.5f, 0.7f * tileSize, playerStart.z + 14.0f}, 180.0f, 4.0f, true, shader);
    //blood
    AddObject(scene, "blood", {playerStart.x + 40.0f, 0.1f, playerStart.z}, 0.0f, 6.0f, false, shader);
    AddObject(scene, "blood", {playerStart.x + -20.0f, 0.1f, playerStart.z + 10.0f}, 0.0f, 6.0f, false, shader);
    AddObject(scene, "blood", {playerStart.x + 0.0f, 0.1f, playerStart.z + -30.0f}, 0.0f, 6.0f, false, shader);
    AddObject(scene, "blood", {playerStart.x + 0.0f, 0.1f, playerStart.z + -70.0f}, 0.0f, 6.0f, false, shader);
    //lamp
    AddObject(scene, "lamp", {playerStart.x + 80.0f, 19.0f, playerStart.z + 2.0f}, 0.0f, 0.5f, false, shader);
    AddObject(scene, "lamp", {playerStart.x + 2.0f, 19.0f, playerStart.z + -70.0f}, 0.0f, 0.5f, false, shader);
    //trash
    AddObject(scene, "trash", {playerStart.x + -40.0f, 3.0f, playerStart.z + -18.0f}, 45.0f, 3.0f, true, shader);
    AddObject(scene, "trash", {playerStart.x + 28.0f, 3.0f, playerStart.z + -35.0f}, 45.0f, 3.0f, true, shader);
    AddObject(scene, "trash", {playerStart.x + 38.0f, 3.0f, playerStart.z + -35.0f}, 0.0f, 3.0f, true, shader);
    AddObject(scene, "trash", {playerStart.x + 48.0f, 3.0f, playerStart.z + -35.0f}, -25.0f, 3.0f, true, shader);

    scene.doorCount = 0;
    AddDoor(scene, (Vector3){playerStart.x + 7.5f, 0, playerStart.z + -47.5f}, 0.0f, scene.doorTexClosed, scene.doorTexOpen, greenTex, wallTex, shader, shotholeTex);
    AddDoor(scene, (Vector3){playerStart.x + 7.5f, 0, playerStart.z + 22.5f}, 180.0f, scene.doorTexClosed, scene.doorTexOpen, whiteTex, wallTex, shader, shotholeTex);
    AddDoor(scene, (Vector3){playerStart.x + 57.5f, 0, playerStart.z + -7.5f}, 270.0f, scene.doorTexClosed, scene.doorTexOpen, greenTex, wallTex, shader, shotholeTex);
    AddDoor(scene, (Vector3){playerStart.x + 107.5f, 0, playerStart.z + -7.5f}, 90.0f, scene.doorTexClosed, scene.doorTexOpen, greenTex, wallTex, shader, shotholeTex);
    AddDoor(scene, (Vector3){playerStart.x + 132.5f, 0, playerStart.z + -72.5f}, 90.0f, scene.doorTexClosed, scene.doorTexOpen, greenTex, wallTex, shader, shotholeTex);
}

static void LoadObjectModel(SceneObject &obj, const char *name, Shader shader)
{
    obj.isLamp = false;

    if (strcmp(name, "lamp") == 0) {
        obj.model = LoadModel("models/lamp.obj");
        obj.texture = LoadTexture("tex/lamp.png");
        obj.isLamp = true;
    } else if (strcmp(name, "sofa") == 0) {
        obj.model = LoadModel("models/sofa.obj");
        obj.texture = LoadTexture("tex/sofa.png");
    } else if (strcmp(name, "blood") == 0) {
        obj.model = LoadModel("models/blood.obj");
        obj.texture = LoadTexture("tex/blood.png");
    } else if (strcmp(name, "trash") == 0) {
        obj.model = LoadModel("models/trash.obj");
        obj.texture = LoadTexture("tex/trash.png");
    }

    SetTextureFilter(obj.texture, TEXTURE_FILTER_POINT);
    obj.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = obj.texture;
    obj.model.materials[0].shader = shader;
}

static BoxCollider MakeColliderFromModel(SceneObject &obj)
{
    BoundingBox bb = GetMeshBoundingBox(obj.model.meshes[0]);
    for (int i = 1; i < obj.model.meshCount; i++) {
        BoundingBox b = GetMeshBoundingBox(obj.model.meshes[i]);
        if (b.min.x < bb.min.x) bb.min.x = b.min.x;
        if (b.min.y < bb.min.y) bb.min.y = b.min.y;
        if (b.min.z < bb.min.z) bb.min.z = b.min.z;
        if (b.max.x > bb.max.x) bb.max.x = b.max.x;
        if (b.max.y > bb.max.y) bb.max.y = b.max.y;
        if (b.max.z > bb.max.z) bb.max.z = b.max.z;
    }
    BoxCollider box;
    box.min.x = obj.position.x + bb.min.x * obj.scale;
    box.min.y = obj.position.y + bb.min.y * obj.scale;
    box.min.z = obj.position.z + bb.min.z * obj.scale;
    box.max.x = obj.position.x + bb.max.x * obj.scale;
    box.max.y = obj.position.y + bb.max.y * obj.scale;
    box.max.z = obj.position.z + bb.max.z * obj.scale;
    return box;
}

void AddObject(Scene &scene, const char *name, Vector3 pos, float rot, float sc, bool addCollision, Shader shader)
{
    if (scene.objectCount >= SCENE_MAX_OBJECTS) return;

    SceneObject &obj = scene.objects[scene.objectCount];
    LoadObjectModel(obj, name, shader);
    obj.position = pos;
    obj.rotation = rot;
    obj.scale = sc;
    obj.addCollision = addCollision;

    if (obj.isLamp)
        scene.lampCount++;

    if (addCollision)
        obj.collider = MakeColliderFromModel(obj);

    scene.objectCount++;
}

void AddDoor(Scene &scene, Vector3 pos, float rot, Texture2D closedTex, Texture2D openTex, Texture2D capLeftTex, Texture2D capRightTex, Shader shader, Texture2D shotholeTex)
{
    if (scene.doorCount >= MAX_DOORS) return;
    scene.doors[scene.doorCount++] = CreateDoor(
        pos, (Vector3){0,1,0}, rot,
        closedTex, openTex, capLeftTex, capRightTex, shader, shotholeTex
    );
}

void DrawScene(Scene &scene, Camera3D camera, Shader shader, Bonus bonuses[], int bonusCount)
{
    DrawDoors(scene.doors, scene.doorCount);

    for (int i = 0; i < scene.objectCount; i++) {
        SceneObject &obj = scene.objects[i];
        Matrix transform = MatrixMultiply(
            MatrixMultiply(
                MatrixScale(obj.scale, obj.scale, obj.scale),
                (obj.rotation != 0.0f) ? MatrixRotateY(obj.rotation * 3.14159f / 180.0f) : MatrixIdentity()
            ),
            MatrixTranslate(obj.position.x, obj.position.y, obj.position.z)
        );

        rlDisableBackfaceCulling();
        for (int mi = 0; mi < obj.model.meshCount; mi++)
            DrawMesh(obj.model.meshes[mi], obj.model.materials[obj.model.meshMaterial[mi]], transform);
    }

    int totalBillboards = scene.zombieCount + bonusCount;

    struct Billboard {
        int type; // 0 = zombie, 1 = bonus
        int index;
        float dist;
    };

    Billboard billboards[SCENE_MAX_ZOMBIES + MAX_BONUSES];
    int count = 0;

    for (int i = 0; i < scene.zombieCount; i++) {
        billboards[count].type = 0;
        billboards[count].index = i;
        billboards[count].dist = Vector3DistanceSqr(camera.position, scene.zombies[i].position);
        count++;
    }
    for (int i = 0; i < bonusCount; i++) {
        if (!bonuses[i].active) continue;
        billboards[count].type = 1;
        billboards[count].index = i;
        billboards[count].dist = Vector3DistanceSqr(camera.position, bonuses[i].position);
        count++;
    }

    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (billboards[j].dist > billboards[i].dist) {
                Billboard tmp = billboards[i];
                billboards[i] = billboards[j];
                billboards[j] = tmp;
            }
        }
    }

    rlDisableDepthMask();

    for (int i = 0; i < count; i++) {
        if (billboards[i].type == 0) {
            DrawZombie(scene.zombies[billboards[i].index], camera, shader);
        } else {
            Bonus &b = bonuses[billboards[i].index];
            Vector3 pos = b.position;
            pos.y = 5.0f + sinf(b.bobTimer) * 1.0f;

            Vector3 forward = Vector3Normalize(Vector3Subtract(camera.position, pos));
            Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, (Vector3){0, 1.0f, 0.0f}));
            Vector3 up = {0, 1.0f, 0};
            float size = 4.0f;

            Vector3 bl = Vector3Subtract(pos, Vector3Add(Vector3Scale(right, size * 0.5f), Vector3Scale(up, size * 0.5f)));
            Vector3 br = Vector3Add(pos, Vector3Subtract(Vector3Scale(right, size * 0.5f), Vector3Scale(up, size * 0.5f)));
            Vector3 tr = Vector3Add(pos, Vector3Add(Vector3Scale(right, size * 0.5f), Vector3Scale(up, size * 0.5f)));
            Vector3 tl = Vector3Add(pos, Vector3Subtract(Vector3Scale(up, size * 0.5f), Vector3Scale(right, size * 0.5f)));

            rlSetTexture(b.texture.id);
            rlBegin(RL_QUADS);
                rlColor4ub(255, 255, 255, 255);
                rlTexCoord2f(0.0f, 1.0f); rlVertex3f(bl.x, bl.y, bl.z);
                rlTexCoord2f(1.0f, 1.0f); rlVertex3f(br.x, br.y, br.z);
                rlTexCoord2f(1.0f, 0.0f); rlVertex3f(tr.x, tr.y, tr.z);
                rlTexCoord2f(0.0f, 0.0f); rlVertex3f(tl.x, tl.y, tl.z);
            rlEnd();
            rlSetTexture(0);
        }
    }

    rlEnableDepthMask();
}

void UnloadScene(Scene &scene)
{
    UnloadDoors();
    UnloadTexture(scene.doorTexClosed);
    UnloadTexture(scene.doorTexOpen);

    for (int i = 0; i < scene.objectCount; i++) {
        UnloadModel(scene.objects[i].model);
        UnloadTexture(scene.objects[i].texture);
    }

    for (int i = 0; i < scene.zombieCount; i++)
        UnloadZombie(scene.zombies[i]);
}

BoxCollider GetCollider(Scene &scene, int index)
{
    return scene.objects[index].collider;
}

bool CheckSceneCollision(Scene &scene, float x, float z, float radius)
{
    for (int i = 0; i < scene.objectCount; i++) {
        if (scene.objects[i].addCollision &&
            CheckBoxCollision(scene.objects[i].collider, x, z, radius))
            return true;
    }
    return false;
}

bool CheckZombieCollision(Scene &scene, float x, float z, float radius, float oldX, float oldZ)
{
    for (int i = 0; i < scene.zombieCount; i++) {
        if (!scene.zombies[i].active || scene.zombies[i].health <= 0.0f) continue;
        float dx = x - scene.zombies[i].position.x;
        float dz = z - scene.zombies[i].position.z;
        float minDist = radius + scene.zombies[i].radius;
        if (dx * dx + dz * dz < minDist * minDist) {
            float odx = oldX - scene.zombies[i].position.x;
            float odz = oldZ - scene.zombies[i].position.z;
            if (odx * odx + odz * odz >= minDist * minDist)
                return true;
        }
    }
    return false;
}
