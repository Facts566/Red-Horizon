#include "scene.h"
#include <cstring>
#include <raymath.h>

void LoadScene(Scene &scene, Shader shader, float tileSize, Vector3 playerStart)
{
    scene.tileSize = tileSize;

    AddObject(scene, "sofa", {12 * tileSize, 0.7f * tileSize, 1.8f * tileSize}, 0.0f, 4.0f, true, shader);
    AddObject(scene, "lamp", {playerStart.x, 19.0f, playerStart.z}, 0.0f, 0.5f, false, shader);
    AddObject(scene, "lamp", {playerStart.x + 2.0f, 19.0f, playerStart.z + -70.0f}, 0.0f, 0.5f, false, shader);
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

void DrawScene(Scene &scene, Camera3D camera)
{
    for (int i = 0; i < scene.objectCount; i++) {
        SceneObject &obj = scene.objects[i];
        Matrix transform = MatrixMultiply(
            MatrixScale(obj.scale, obj.scale, obj.scale),
            MatrixTranslate(obj.position.x, obj.position.y, obj.position.z)
        );
        if (obj.rotation != 0.0f)
            transform = MatrixMultiply(transform, MatrixRotateY(obj.rotation * 3.14159f / 180.0f));

        for (int mi = 0; mi < obj.model.meshCount; mi++)
            DrawMesh(obj.model.meshes[mi], obj.model.materials[obj.model.meshMaterial[mi]], transform);
    }

    for (int i = 0; i < scene.zombieCount; i++)
        DrawZombie(scene.zombies[i], camera);
}

void UnloadScene(Scene &scene)
{
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
