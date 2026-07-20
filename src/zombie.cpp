#include "zombie.h"
#include "raycast.h"
#include "map.h"
#include <rlgl.h>
#include <raymath.h>
#include <cmath>
#include <cstring>

static Model s_zombieModel = { 0 };

void InitZombieModel(Shader shader)
{
    Mesh m = { 0 };
    m.vertexCount = 4;
    m.triangleCount = 2;
    m.vertices = (float *)MemAlloc(12 * sizeof(float));
    m.texcoords = (float *)MemAlloc(8 * sizeof(float));
    m.normals = (float *)MemAlloc(12 * sizeof(float));
    m.indices = (unsigned short *)MemAlloc(6 * sizeof(unsigned short));

    float hw = 5.4f;
    m.vertices[0]  = -hw; m.vertices[1]  = -hw; m.vertices[2]  = 0;
    m.vertices[3]  =  hw; m.vertices[4]  = -hw; m.vertices[5]  = 0;
    m.vertices[6]  = -hw; m.vertices[7]  =  hw; m.vertices[8]  = 0;
    m.vertices[9]  =  hw; m.vertices[10] =  hw; m.vertices[11] = 0;

    m.texcoords[0] = 0; m.texcoords[1] = 1;
    m.texcoords[2] = 1; m.texcoords[3] = 1;
    m.texcoords[4] = 0; m.texcoords[5] = 0;
    m.texcoords[6] = 1; m.texcoords[7] = 0;

    for (int i = 0; i < 4; i++) {
        m.normals[i*3] = 0; m.normals[i*3+1] = 0; m.normals[i*3+2] = 1;
    }

    m.indices[0] = 0; m.indices[1] = 1; m.indices[2] = 2;
    m.indices[3] = 1; m.indices[4] = 3; m.indices[5] = 2;

    UploadMesh(&m, false);
    s_zombieModel = LoadModelFromMesh(m);
    s_zombieModel.materials[0].shader = shader;
}

#define ASTAR_MAX_NODES 2048

static struct AStarNode {
    float g, f;
    int px, pz;
    unsigned char flags;
} s_astarNodes[ASTAR_MAX_NODES];

static int s_openHeap[ASTAR_MAX_NODES];
static int s_heapSize;

static void HeapPush(int idx)
{
    s_openHeap[s_heapSize] = idx;
    s_heapSize++;
    int i = s_heapSize - 1;
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (s_astarNodes[s_openHeap[parent]].f <= s_astarNodes[s_openHeap[i]].f) break;
        int tmp = s_openHeap[parent];
        s_openHeap[parent] = s_openHeap[i];
        s_openHeap[i] = tmp;
        i = parent;
    }
}

static int HeapPop()
{
    int top = s_openHeap[0];
    s_heapSize--;
    s_openHeap[0] = s_openHeap[s_heapSize];
    int i = 0;
    while (true) {
        int smallest = i;
        int l = 2 * i + 1;
        int r = 2 * i + 2;
        if (l < s_heapSize && s_astarNodes[s_openHeap[l]].f < s_astarNodes[s_openHeap[smallest]].f)
            smallest = l;
        if (r < s_heapSize && s_astarNodes[s_openHeap[r]].f < s_astarNodes[s_openHeap[smallest]].f)
            smallest = r;
        if (smallest == i) break;
        int tmp = s_openHeap[i];
        s_openHeap[i] = s_openHeap[smallest];
        s_openHeap[smallest] = tmp;
        i = smallest;
    }
    return top;
}

static bool IsWalkable(Level level, int col, int row)
{
    if (col < 0 || col >= level.width || row < 0 || row >= level.height)
        return false;
    char c = level.data[row * level.width + col];
    return c != '&' && c != '@';
}

static int FindPath(Level level, float startX, float startZ, float endX, float endZ, Vector2 *outPath, int maxPathLen)
{
    int w = level.width;
    int h = level.height;
    float ts = level.tileSize;
    int total = w * h;
    if (total > ASTAR_MAX_NODES) return 0;

    int sc = (int)(startX / ts);
    int sr = (int)(startZ / ts);
    int ec = (int)(endX / ts);
    int er = (int)(endZ / ts);

    if (sc < 0) sc = 0; if (sc >= w) sc = w - 1;
    if (sr < 0) sr = 0; if (sr >= h) sr = h - 1;
    if (ec < 0) ec = 0; if (ec >= w) ec = w - 1;
    if (er < 0) er = 0; if (er >= h) er = h - 1;

    if (!IsWalkable(level, sc, sr) || !IsWalkable(level, ec, er))
        return 0;

    memset(s_astarNodes, 0, total * sizeof(AStarNode));
    s_heapSize = 0;

    int si = sr * w + sc;
    s_astarNodes[si].g = 0;
    float dx = (float)(ec - sc), dz = (float)(er - sr);
    s_astarNodes[si].f = sqrtf(dx * dx + dz * dz);
    s_astarNodes[si].px = sc; s_astarNodes[si].pz = sr;
    s_astarNodes[si].flags = 1;
    HeapPush(si);

    int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    bool found = false;

    while (s_heapSize > 0) {
        int bestIdx = HeapPop();
        AStarNode &cur = s_astarNodes[bestIdx];
        cur.flags = 2;
        int cx = bestIdx % w;
        int cz = bestIdx / w;
        if (cx == ec && cz == er) { found = true; break; }

        for (int d = 0; d < 4; d++) {
            int nx = cx + dirs[d][0];
            int nz = cz + dirs[d][1];
            if (nx < 0 || nx >= w || nz < 0 || nz >= h) continue;
            if (!IsWalkable(level, nx, nz)) continue;

            int ni = nz * w + nx;
            AStarNode &nb = s_astarNodes[ni];
            if (nb.flags == 2) continue;

            float ng = cur.g + 1.0f;
            if (nb.flags == 0 || ng < nb.g) {
                nb.g = ng;
                float ddx = (float)(ec - nx), ddz = (float)(er - nz);
                nb.f = ng + sqrtf(ddx * ddx + ddz * ddz);
                nb.px = cx; nb.pz = cz;
                if (nb.flags == 0) {
                    nb.flags = 1;
                    HeapPush(ni);
                } else {
                    for (int i = 0; i < s_heapSize; i++) {
                        if (s_openHeap[i] == ni) {
                            while (i > 0) {
                                int parent = (i - 1) / 2;
                                if (s_astarNodes[s_openHeap[parent]].f <= s_astarNodes[s_openHeap[i]].f) break;
                                int tmp = s_openHeap[parent];
                                s_openHeap[parent] = s_openHeap[i];
                                s_openHeap[i] = tmp;
                                i = parent;
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    int count = 0;
    if (found) {
        int cx = ec, cz = er;
        while (!(cx == sc && cz == sr)) {
            if (count < maxPathLen) {
                outPath[count].x = (float)cx;
                outPath[count].y = (float)cz;
                count++;
            }
            AStarNode &n = s_astarNodes[cz * w + cx];
            cx = n.px; cz = n.pz;
        }
        for (int i = 0; i < count / 2; i++) {
            Vector2 tmp = outPath[i];
            outPath[i] = outPath[count - 1 - i];
            outPath[count - 1 - i] = tmp;
        }
    }

    return count;
}

void InitZombie(Zombie &zombie, Vector3 pos, Texture2D idle, Texture2D walk1, Texture2D walk2, Texture2D dead)
{
    zombie.position = pos;
    zombie.health = 100.0f;
    zombie.speed = 12.0f;
    zombie.radius = 1.5f;
    zombie.pathCount = 0;
    zombie.pathRecalcTimer = 0.0f;
    zombie.pathIndex = 0;
    zombie.textureIdle = idle;
    zombie.textureWalk1 = walk1;
    zombie.textureWalk2 = walk2;
    zombie.textureDead = dead;
    zombie.isWalking = false;
    zombie.animTimer = 0.0f;
    zombie.animFrame = false;
    zombie.active = true;
    zombie.triggered = false;
}

void UpdateZombie(Zombie &zombie, Level level, Door doors[], int doorCount, BoxCollider sofaBox, Vector3 playerPos, float dt)
{
    if (!zombie.active || zombie.health <= 0.0f) return;

    zombie.isWalking = false;

    if (!zombie.triggered) {
        Vector3 toPlayer = Vector3Subtract(playerPos, zombie.position);
        float dist = Vector3Length(toPlayer);
        if (dist < 60.0f) {
            Vector3 dir = Vector3Normalize(toPlayer);
            Vector3 hitPos, hitNorm;
            if (!RaycastWall(level, zombie.position, dir, dist, hitPos, hitNorm))
                zombie.triggered = true;
        }
    }
    if (!zombie.triggered)
        return;

    zombie.pathRecalcTimer -= dt;
    if (zombie.pathRecalcTimer <= 0.0f) {
        zombie.pathCount = FindPath(level, zombie.position.x, zombie.position.z, playerPos.x, playerPos.z, zombie.path, ZOMBIE_MAX_PATH);
        zombie.pathIndex = 0;
        zombie.pathRecalcTimer = 0.3f;
    }

    if (zombie.pathCount == 0) return;

    if (zombie.pathIndex < zombie.pathCount) {
        float targetX = zombie.path[zombie.pathIndex].x * level.tileSize + level.tileSize / 2.0f;
        float targetZ = zombie.path[zombie.pathIndex].y * level.tileSize + level.tileSize / 2.0f;
        float dist = sqrtf((targetX - zombie.position.x) * (targetX - zombie.position.x) + (targetZ - zombie.position.z) * (targetZ - zombie.position.z));
        if (dist < 1.5f) {
            zombie.pathIndex++;
        }
    }

    if (zombie.pathIndex >= zombie.pathCount) {
        float dx = playerPos.x - zombie.position.x;
        float dz = playerPos.z - zombie.position.z;
        float dist = sqrtf(dx * dx + dz * dz);
        if (dist > 0.1f) {
            float speed = zombie.speed * dt;
            float nx = zombie.position.x + (dx / dist) * speed;
            float nz = zombie.position.z + (dz / dist) * speed;
            if (!CheckWallCollision(level, nx, zombie.position.z, zombie.radius) &&
                !CheckAnyDoorCollision(doors, doorCount, nx, zombie.position.z, zombie.radius) &&
                !CheckBoxCollision(sofaBox, nx, zombie.position.z, zombie.radius))
                zombie.position.x = nx;
            if (!CheckWallCollision(level, zombie.position.x, nz, zombie.radius) &&
                !CheckAnyDoorCollision(doors, doorCount, zombie.position.x, nz, zombie.radius) &&
                !CheckBoxCollision(sofaBox, zombie.position.x, nz, zombie.radius))
                zombie.position.z = nz;
            zombie.isWalking = true;
        }
        return;
    }

    float tx = zombie.path[zombie.pathIndex].x * level.tileSize + level.tileSize / 2.0f;
    float tz = zombie.path[zombie.pathIndex].y * level.tileSize + level.tileSize / 2.0f;
    float dx = tx - zombie.position.x;
    float dz = tz - zombie.position.z;
    float dist = sqrtf(dx * dx + dz * dz);

    if (dist > 0.1f) {
        float speed = zombie.speed * dt;
        float nx = zombie.position.x + (dx / dist) * speed;
        float nz = zombie.position.z + (dz / dist) * speed;

        if (!CheckWallCollision(level, nx, zombie.position.z, zombie.radius) &&
            !CheckAnyDoorCollision(doors, doorCount, nx, zombie.position.z, zombie.radius) &&
            !CheckBoxCollision(sofaBox, nx, zombie.position.z, zombie.radius))
            zombie.position.x = nx;
        if (!CheckWallCollision(level, zombie.position.x, nz, zombie.radius) &&
            !CheckAnyDoorCollision(doors, doorCount, zombie.position.x, nz, zombie.radius) &&
            !CheckBoxCollision(sofaBox, zombie.position.x, nz, zombie.radius))
            zombie.position.z = nz;
        zombie.isWalking = true;
    }

    if (zombie.isWalking) {
        zombie.animTimer += dt;
        if (zombie.animTimer >= 0.3f) {
            zombie.animTimer -= 0.3f;
            zombie.animFrame = !zombie.animFrame;
        }
    } else {
        zombie.animTimer = 0.0f;
        zombie.animFrame = false;
    }
}

void DrawZombie(Zombie &zombie, Camera3D camera, Shader shader)
{
    if (!zombie.active) return;
    Texture2D tex;
    if (zombie.health <= 0.0f)
        tex = zombie.textureDead;
    else if (zombie.isWalking)
        tex = zombie.animFrame ? zombie.textureWalk1 : zombie.textureWalk2;
    else
        tex = zombie.textureIdle;

    float size = 10.8f;
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.position, zombie.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, (Vector3){0, 1.0f, 0.0f}));
    Vector3 up = {0, 1.0f, 0};

    Vector3 bl = Vector3Subtract(zombie.position, Vector3Add(Vector3Scale(right, size * 0.5f), Vector3Scale(up, size * 0.5f)));
    Vector3 br = Vector3Add(zombie.position, Vector3Subtract(Vector3Scale(right, size * 0.5f), Vector3Scale(up, size * 0.5f)));
    Vector3 tr = Vector3Add(zombie.position, Vector3Add(Vector3Scale(right, size * 0.5f), Vector3Scale(up, size * 0.5f)));
    Vector3 tl = Vector3Add(zombie.position, Vector3Subtract(Vector3Scale(up, size * 0.5f), Vector3Scale(right, size * 0.5f)));

    rlDisableDepthMask();
    BeginShaderMode(shader);
    rlSetTexture(tex.id);
    rlBegin(RL_QUADS);
        rlColor4ub(255, 255, 255, 255);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(bl.x, bl.y, bl.z);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(br.x, br.y, br.z);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(tr.x, tr.y, tr.z);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(tl.x, tl.y, tl.z);
    rlEnd();
    rlSetTexture(0);
    EndShaderMode();
    rlEnableDepthMask();
}

void UnloadZombie(Zombie &zombie)
{
}

bool ZombieHitByRay(Zombie &zombie, Vector3 origin, Vector3 dir)
{
    if (!zombie.active) return false;

    float hitRadius = 5.5f;
    Vector3 oc = {origin.x - zombie.position.x, origin.y - zombie.position.y, origin.z - zombie.position.z};
    float a = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
    float b = 2.0f * (oc.x * dir.x + oc.y * dir.y + oc.z * dir.z);
    float c = oc.x * oc.x + oc.y * oc.y + oc.z * oc.z - hitRadius * hitRadius;
    float disc = b * b - 4.0f * a * c;
    return disc >= 0.0f;
}
