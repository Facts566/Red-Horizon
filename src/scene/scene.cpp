#include "scene.h"
#include "config.h"
#include <rlgl.h>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <raymath.h>
#include <algorithm>

static Vector3 TilePos(float col, float row, float y = 0.0f) {
    return {col * TILE_SIZE + TILE_SIZE / 2.0f, y, row * TILE_SIZE + TILE_SIZE / 2.0f};
}

void LoadDecor(Scene &scene, const char *decorPath, Shader shader, Texture2D greenTex, Texture2D wallTex, Texture2D shotholeTex, Texture2D whiteTex, Level level)
{
    FILE *f = fopen(decorPath, "r");
    if (!f) return;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n') continue;

        char type[32];
        if (sscanf(line, "obj %31s", type) == 1) {
            if (strcmp(type, "key") == 0) continue;
            float col, row, y, rot, scale;
            int collision;
            if (sscanf(line, "obj %31s %f %f %f %f %f %d", type, &col, &row, &y, &rot, &scale, &collision) == 7) {
                AddObject(scene, type, TilePos(col, row, y), rot, scale, collision != 0, shader);
            }
        } else if (strncmp(line, "door", 4) == 0) {
            float col, row, rot;
            int locked, exit;
            int keyId = 0;
            char wl[16] = "brick", wr[16] = "brick";
            if (sscanf(line, "door %f %f %f %d %d %15s %15s %d", &col, &row, &rot, &locked, &exit, wl, wr, &keyId) >= 5) {
                Texture2D capLeftTex, capRightTex;
                if (strcmp(wl, "green") == 0) capLeftTex = greenTex;
                else if (strcmp(wl, "white") == 0) capLeftTex = whiteTex;
                else capLeftTex = wallTex;
                if (strcmp(wr, "green") == 0) capRightTex = greenTex;
                else if (strcmp(wr, "white") == 0) capRightTex = whiteTex;
                else capRightTex = wallTex;
                AddDoor(scene, TilePos(col, row), rot, scene.doorTexClosed, scene.doorTexOpen, capLeftTex, capRightTex, shader, shotholeTex, locked != 0, exit != 0, keyId);
            }
        }
    }
    fclose(f);
}

void LoadScene(Scene &scene, Shader shader, float tileSize, Texture2D greenTex, Texture2D wallTex, Texture2D shotholeTex, Texture2D whiteTex, int levelIndex, Level level)
{
    scene.tileSize = tileSize;
    scene.objectCount = 0;
    scene.lampCount = 0;

    scene.doorTexClosed = LoadTexture("tex/decor/door_1.png");
    SetTextureFilter(scene.doorTexClosed, TEXTURE_FILTER_POINT);
    SetTextureWrap(scene.doorTexClosed, TEXTURE_WRAP_REPEAT);
    rlTextureParameters(scene.doorTexClosed.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
    rlTextureParameters(scene.doorTexClosed.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);

    scene.doorTexOpen = LoadTexture("tex/decor/door_2.png");
    SetTextureFilter(scene.doorTexOpen, TEXTURE_FILTER_POINT);
    SetTextureWrap(scene.doorTexOpen, TEXTURE_WRAP_REPEAT);
    rlTextureParameters(scene.doorTexOpen.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
    rlTextureParameters(scene.doorTexOpen.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);

    scene.doorCount = 0;
    scene.particleCount = 0;

    char decorPath[256];
    sprintf(decorPath, "map/level_%d/decor.txt", levelIndex);
    LoadDecor(scene, decorPath, shader, greenTex, wallTex, shotholeTex, whiteTex, level);
}

static void LoadObjectModel(SceneObject &obj, const char *name, Shader shader)
{
    obj.isLamp = false;

    if (strcmp(name, "lamp") == 0) {
        obj.model = LoadModel("models/lamp.obj");
        obj.texture = LoadTexture("tex/decor/lamp.png");
        obj.isLamp = true;
    } else if (strcmp(name, "sofa") == 0) {
        obj.model = LoadModel("models/sofa.obj");
        obj.texture = LoadTexture("tex/decor/sofa.png");
    } else if (strcmp(name, "blood") == 0) {
        obj.model = LoadModel("models/blood.obj");
        obj.texture = LoadTexture("tex/decor/blood.png");
    } else if (strcmp(name, "trash") == 0) {
        obj.model = LoadModel("models/trash.obj");
        obj.texture = LoadTexture("tex/decor/trash.png");
    } else if (strcmp(name, "box") == 0) {
        Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);
        obj.model = LoadModelFromMesh(cube);
        obj.texture = LoadTexture("tex/decor/box.png");
        obj.destructible = true;
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
    obj.destructible = false;
    LoadObjectModel(obj, name, shader);
    obj.position = pos;
    obj.rotation = rot;
    obj.scale = sc;
    obj.addCollision = addCollision;
    obj.active = true;

    if (obj.isLamp)
        scene.lampCount++;

    if (addCollision)
        obj.collider = MakeColliderFromModel(obj);

    if (obj.destructible)
        obj.collider.max.y = obj.position.y + WALL_HEIGHT;

    scene.objectCount++;
}

void AddDoor(Scene &scene, Vector3 pos, float rot, Texture2D closedTex, Texture2D openTex, Texture2D capLeftTex, Texture2D capRightTex, Shader shader, Texture2D shotholeTex, bool isLocked, bool isExit, int keyId)
{
    if (scene.doorCount >= MAX_DOORS) return;
    scene.doors[scene.doorCount++] = CreateDoor(
        pos, (Vector3){0,1,0}, rot,
        closedTex, openTex, capLeftTex, capRightTex, shader, shotholeTex, isLocked, isExit, keyId
    );
}

void DrawScene(Scene &scene, Camera3D camera, Shader shader, Bonus bonuses[], int bonusCount)
{
    DrawDoors(scene.doors, scene.doorCount);

    for (int i = 0; i < scene.objectCount; i++) {
        SceneObject &obj = scene.objects[i];
        if (!obj.active) continue;
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
            pos.y = BONUS_Y_HEIGHT + sinf(b.bobTimer) * BONUS_BOB_AMPLITUDE;

            Vector3 forward = Vector3Normalize(Vector3Subtract(camera.position, pos));
            Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, (Vector3){0, 1.0f, 0.0f}));
            Vector3 up = {0, 1.0f, 0};
            float size = BONUS_SIZE;

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
    if (scene.doorTexClosed.id != 0) UnloadTexture(scene.doorTexClosed);
    if (scene.doorTexOpen.id != 0) UnloadTexture(scene.doorTexOpen);

    for (int i = 0; i < scene.objectCount; i++) {
        UnloadModel(scene.objects[i].model);
        if (scene.objects[i].texture.id != 0) UnloadTexture(scene.objects[i].texture);
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
        if (!scene.objects[i].active) continue;
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

void SpawnBoxParticles(Scene &scene, Vector3 pos, Texture2D tex)
{
    for (int i = 0; i < BOX_PARTICLE_COUNT; i++) {
        if (scene.particleCount >= MAX_PARTICLES) break;
        Particle &p = scene.particles[scene.particleCount];
        p.position = pos;
        p.velocity.x = ((float)GetRandomValue(-1000, 1000) / 1000.0f) * BOX_PARTICLE_SPEED;
        p.velocity.y = ((float)GetRandomValue(200, 1000) / 1000.0f) * BOX_PARTICLE_SPEED;
        p.velocity.z = ((float)GetRandomValue(-1000, 1000) / 1000.0f) * BOX_PARTICLE_SPEED;
        p.lifetime = BOX_PARTICLE_LIFETIME;
        p.maxLifetime = BOX_PARTICLE_LIFETIME;
        p.texture = tex;
        scene.particleCount++;
    }
}

void UpdateParticles(Scene &scene, float dt)
{
    for (int i = scene.particleCount - 1; i >= 0; i--) {
        Particle &p = scene.particles[i];
        p.velocity.y -= BOX_PARTICLE_GRAVITY * dt;
        p.position.x += p.velocity.x * dt;
        p.position.y += p.velocity.y * dt;
        p.position.z += p.velocity.z * dt;
        p.lifetime -= dt;
        if (p.lifetime <= 0.0f) {
            scene.particles[i] = scene.particles[scene.particleCount - 1];
            scene.particleCount--;
        }
    }
}

void DrawParticles(Scene &scene, Camera3D camera)
{
    rlDisableDepthMask();
    for (int i = 0; i < scene.particleCount; i++) {
        Particle &p = scene.particles[i];
        float alpha = p.lifetime / p.maxLifetime;
        float size = BOX_PARTICLE_SIZE * alpha;

        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.position, p.position));
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, (Vector3){0, 1.0f, 0.0f}));
        Vector3 up = {0, 1.0f, 0};

        Vector3 bl = Vector3Subtract(p.position, Vector3Add(Vector3Scale(right, size * 0.5f), Vector3Scale(up, size * 0.5f)));
        Vector3 br = Vector3Add(p.position, Vector3Subtract(Vector3Scale(right, size * 0.5f), Vector3Scale(up, size * 0.5f)));
        Vector3 tr = Vector3Add(p.position, Vector3Add(Vector3Scale(right, size * 0.5f), Vector3Scale(up, size * 0.5f)));
        Vector3 tl = Vector3Add(p.position, Vector3Subtract(Vector3Scale(up, size * 0.5f), Vector3Scale(right, size * 0.5f)));

        rlSetTexture(p.texture.id);
        rlBegin(RL_QUADS);
            rlColor4ub(255, 255, 255, (unsigned char)(alpha * 255));
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(bl.x, bl.y, bl.z);
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(br.x, br.y, br.z);
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(tr.x, tr.y, tr.z);
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(tl.x, tl.y, tl.z);
        rlEnd();
        rlSetTexture(0);
    }
    rlEnableDepthMask();
}

bool RayBoxIntersect(BoxCollider box, Vector3 origin, Vector3 dir, float maxDist, Vector3 &hitPos, Vector3 &hitNormal)
{
    float tmin = -INFINITY, tmax = INFINITY;
    Vector3 normals[6] = {
        {-1, 0, 0}, {1, 0, 0},
        {0, -1, 0}, {0, 1, 0},
        {0, 0, -1}, {0, 0, 1}
    };
    float tMin[3], tMax[3];
    int minAxis = 0, maxAxis = 0;

    float invDir[3] = {
        dir.x != 0 ? 1.0f / dir.x : INFINITY,
        dir.y != 0 ? 1.0f / dir.y : INFINITY,
        dir.z != 0 ? 1.0f / dir.z : INFINITY
    };

    float bmin[3] = {box.min.x, box.min.y, box.min.z};
    float bmax[3] = {box.max.x, box.max.y, box.max.z};
    float orig[3] = {origin.x, origin.y, origin.z};

    for (int i = 0; i < 3; i++) {
        tMin[i] = (bmin[i] - orig[i]) * invDir[i];
        tMax[i] = (bmax[i] - orig[i]) * invDir[i];
        if (tMin[i] > tMax[i]) {
            float tmp = tMin[i]; tMin[i] = tMax[i]; tMax[i] = tmp;
        }
        if (tMin[i] > tmin) { tmin = tMin[i]; minAxis = i; }
        if (tMax[i] < tmax) { tmax = tMax[i]; maxAxis = i; }
        if (tmin > tmax || tmax < 0) return false;
    }

    float t = tmin;
    if (t < 0) { t = tmax; if (t < 0 || t > maxDist) return false; }
    if (t > maxDist) return false;

    hitPos.x = origin.x + dir.x * t;
    hitPos.y = origin.y + dir.y * t;
    hitPos.z = origin.z + dir.z * t;

    if (t == tMin[minAxis])
        hitNormal = normals[minAxis * 2];
    else
        hitNormal = normals[maxAxis * 2 + 1];

    return true;
}
