#include "scene.h"
#include <rlgl.h>
#include <cstring>
#include <raymath.h>
#include <algorithm>

void LoadScene(Scene &scene, Shader shader, float tileSize, Vector3 playerStart, Texture2D greenTex, Texture2D wallTex, Texture2D shotholeTex)
{
    Texture2D doorTexClosed = LoadTexture("tex/door/door_1.png");
    SetTextureFilter(doorTexClosed, TEXTURE_FILTER_POINT);
    SetTextureWrap(doorTexClosed, TEXTURE_WRAP_REPEAT);
    rlTextureParameters(doorTexClosed.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
    rlTextureParameters(doorTexClosed.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);

    Texture2D doorTexOpen = LoadTexture("tex/door/door_2.png");
    SetTextureFilter(doorTexOpen, TEXTURE_FILTER_POINT);
    SetTextureWrap(doorTexOpen, TEXTURE_WRAP_REPEAT);
    rlTextureParameters(doorTexOpen.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
    rlTextureParameters(doorTexOpen.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);

    scene.tileSize = tileSize;

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
    scene.doors[scene.doorCount++] = CreateDoor(
        (Vector3){12 * tileSize, 0, 9 * tileSize},
        (Vector3){0,1,0}, 0.0f,
        doorTexClosed, doorTexOpen, greenTex, wallTex, shader, LoadTexture("tex/shothole.png")
    );
    scene.doors[scene.doorCount++] = CreateDoor(
        (Vector3){22 * tileSize, 0, 17 * tileSize},
        (Vector3){0,1,0}, 270.0f,
        doorTexClosed, doorTexOpen, greenTex, wallTex, shader, LoadTexture("tex/shothole.png")
    );
    scene.doors[scene.doorCount++] = CreateDoor(
        (Vector3){37 * tileSize, 0, 4 * tileSize},
        (Vector3){0,1,0}, 90.0f,
        doorTexClosed, doorTexOpen, wallTex, greenTex, shader, LoadTexture("tex/shothole.png")
    );

    scene.doors[scene.doorCount++] = CreateDoor(
        (Vector3){32 * tileSize, 0, 17 * tileSize},
        (Vector3){0,1,0}, 90.0f,
        doorTexClosed, doorTexOpen, greenTex, wallTex, shader, LoadTexture("tex/shothole.png")
    );
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

void DrawScene(Scene &scene, Camera3D camera, Shader shader)
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

    {
        int order[SCENE_MAX_ZOMBIES];
        for (int i = 0; i < scene.zombieCount; i++)
            order[i] = i;
        for (int i = 0; i < scene.zombieCount - 1; i++) {
            for (int j = i + 1; j < scene.zombieCount; j++) {
                float di = Vector3DistanceSqr(camera.position, scene.zombies[order[i]].position);
                float dj = Vector3DistanceSqr(camera.position, scene.zombies[order[j]].position);
                if (di < dj) { int tmp = order[i]; order[i] = order[j]; order[j] = tmp; }
            }
        }
        for (int i = 0; i < scene.zombieCount; i++)
            DrawZombie(scene.zombies[order[i]], camera, shader);
    }
}

void UnloadScene(Scene &scene)
{
    UnloadDoors();

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
