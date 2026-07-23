#pragma once
#include <raylib.h>
#include "level.h"
#include "zombie.h"
#include "props.h"
#include "door.h"

#define SCENE_MAX_OBJECTS 64
#define SCENE_MAX_LAMPS 4
#define SCENE_MAX_ZOMBIES 64

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

    Door doors[MAX_DOORS];
    int doorCount;
    Texture2D doorTexClosed;
    Texture2D doorTexOpen;

    Zombie zombies[SCENE_MAX_ZOMBIES];
    int zombieCount;
};

void LoadScene(Scene &scene, Shader shader, float tileSize, Vector3 playerStart, Texture2D greenTex, Texture2D wallTex, Texture2D shotholeTex, Texture2D whiteTex);
void AddObject(Scene &scene, const char *name, Vector3 pos, float rot, float sc, bool addCollision, Shader shader);
void AddDoor(Scene &scene, Vector3 pos, float rot, Texture2D closedTex, Texture2D openTex, Texture2D capLeftTex, Texture2D capRightTex, Shader shader, Texture2D shotholeTex);
void DrawScene(Scene &scene, Camera3D camera, Shader shader);
void UnloadScene(Scene &scene);
bool CheckBoxCollision(BoxCollider box, float x, float z, float radius);
BoxCollider GetCollider(Scene &scene, int index);
bool CheckSceneCollision(Scene &scene, float x, float z, float radius);
bool CheckZombieCollision(Scene &scene, float x, float z, float radius, float oldX, float oldZ);
