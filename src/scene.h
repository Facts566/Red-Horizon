#pragma once
#include <raylib.h>
#include "level.h"
#include "zombie.h"
#include "props.h"

#define SCENE_MAX_OBJECTS 64
#define SCENE_MAX_LAMPS 4
#define SCENE_MAX_ZOMBIES 8

struct SceneObject {
    Model model;
    Texture2D texture;
    Vector3 position;
    float rotation;
    float scale;
    bool addCollision;
    BoxCollider collider;
    bool isLamp;
};

struct Scene {
    float tileSize;
    SceneObject objects[SCENE_MAX_OBJECTS];
    int objectCount;
    int lampCount;

    Zombie zombies[SCENE_MAX_ZOMBIES];
    int zombieCount;
};

void LoadScene(Scene &scene, Shader shader, float tileSize, Vector3 playerStart);
void AddObject(Scene &scene, const char *name, Vector3 pos, float rot, float sc, bool addCollision, Shader shader);
void DrawScene(Scene &scene, Camera3D camera);
void UnloadScene(Scene &scene);
bool CheckBoxCollision(BoxCollider box, float x, float z, float radius);
BoxCollider GetCollider(Scene &scene, int index);
