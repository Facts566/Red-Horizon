#pragma once
#include <raylib.h>
#include "level.h"
#include "zombie.h"
#include "props.h"
#include "door.h"
#include "bonus.h"
#include "config.h"

struct SceneObject {
    Model model;
    Texture2D texture;
    Vector3 position;
    float rotation;
    float scale;
    bool addCollision;
    BoxCollider collider;
    bool isLamp;
    bool active;
    bool destructible;
};

struct Particle {
    Vector3 position;
    Vector3 velocity;
    float lifetime;
    float maxLifetime;
    Texture2D texture;
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

    Particle particles[MAX_PARTICLES];
    int particleCount;
};

void LoadScene(Scene &scene, Shader shader, float tileSize, Texture2D greenTex, Texture2D wallTex, Texture2D shotholeTex, Texture2D whiteTex, int levelIndex, Level level);
void LoadDecor(Scene &scene, const char *decorPath, Shader shader, Texture2D greenTex, Texture2D wallTex, Texture2D shotholeTex, Texture2D whiteTex, Level level);
void AddObject(Scene &scene, const char *name, Vector3 pos, float rot, float sc, bool addCollision, Shader shader);
void AddDoor(Scene &scene, Vector3 pos, float rot, Texture2D closedTex, Texture2D openTex, Texture2D capLeftTex, Texture2D capRightTex, Shader shader, Texture2D shotholeTex, bool isLocked = false, bool isExit = false, int keyId = 0);
void DrawScene(Scene &scene, Camera3D camera, Shader shader, Bonus bonuses[], int bonusCount);
void UnloadScene(Scene &scene);
bool CheckBoxCollision(BoxCollider box, float x, float z, float radius);
BoxCollider GetCollider(Scene &scene, int index);
bool CheckSceneCollision(Scene &scene, float x, float z, float radius);
bool CheckZombieCollision(Scene &scene, float x, float z, float radius, float oldX, float oldZ);
void SpawnBoxParticles(Scene &scene, Vector3 pos, Texture2D tex);
void UpdateParticles(Scene &scene, float dt);
void DrawParticles(Scene &scene, Camera3D camera);
bool RayBoxIntersect(BoxCollider box, Vector3 origin, Vector3 dir, float maxDist, Vector3 &hitPos, Vector3 &hitNormal);
