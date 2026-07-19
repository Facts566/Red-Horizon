#include "zombie.h"
#include <cmath>

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

    struct Node {
        float g, f;
        int px, pz;
        bool inOpen, inClosed;
    };
    Node *nodes = new Node[w * h]();

    nodes[sr * w + sc].g = 0;
    float dx = (float)(ec - sc), dz = (float)(er - sr);
    nodes[sr * w + sc].f = sqrtf(dx * dx + dz * dz);
    nodes[sr * w + sc].px = sc; nodes[sr * w + sc].pz = sr;
    nodes[sr * w + sc].inOpen = true;

    int openCount = 1;
    int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    bool found = false;
    int foundIdx = -1;

    while (openCount > 0) {
        int bestIdx = -1;
        float bestF = 1e9f;
        for (int i = 0; i < w * h; i++) {
            if (nodes[i].inOpen && !nodes[i].inClosed && nodes[i].f < bestF) {
                bestF = nodes[i].f;
                bestIdx = i;
            }
        }
        if (bestIdx == -1) break;

        Node &cur = nodes[bestIdx];
        int cx = bestIdx % w;
        int cz = bestIdx / w;
        if (cx == ec && cz == er) { found = true; foundIdx = bestIdx; break; }

        cur.inClosed = true;
        openCount--;

        for (int d = 0; d < 4; d++) {
            int nx = cx + dirs[d][0];
            int nz = cz + dirs[d][1];
            if (nx < 0 || nx >= w || nz < 0 || nz >= h) continue;
            if (!IsWalkable(level, nx, nz)) continue;

            Node &nb = nodes[nz * w + nx];
            if (nb.inClosed) continue;

            float ng = cur.g + 1.0f;
            if (!nb.inOpen || ng < nb.g) {
                nb.g = ng;
                float ddx = (float)(ec - nx), ddz = (float)(er - nz);
                nb.f = ng + sqrtf(ddx * ddx + ddz * ddz);
                nb.px = cx; nb.pz = cz;
                if (!nb.inOpen) { nb.inOpen = true; openCount++; }
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
            Node &n = nodes[cz * w + cx];
            cx = n.px; cz = n.pz;
        }
        for (int i = 0; i < count / 2; i++) {
            Vector2 tmp = outPath[i];
            outPath[i] = outPath[count - 1 - i];
            outPath[count - 1 - i] = tmp;
        }
    }

    delete[] nodes;
    return count;
}

void InitZombie(Zombie &zombie, Vector3 pos)
{
    zombie.position = pos;
    zombie.health = 50.0f;
    zombie.speed = 12.0f;
    zombie.radius = 1.5f;
    zombie.pathCount = 0;
    zombie.pathRecalcTimer = 0.0f;
    zombie.pathIndex = 0;
    zombie.texture = LoadTexture("tex/zombi.png");
    SetTextureFilter(zombie.texture, TEXTURE_FILTER_POINT);
    zombie.active = true;
}

void UpdateZombie(Zombie &zombie, Level level, Door doors[], int doorCount, BoxCollider sofaBox, Vector3 playerPos, float dt)
{
    if (!zombie.active) return;

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
    }
}

void DrawZombie(Zombie &zombie, Camera3D camera)
{
    if (!zombie.active) return;
    DrawBillboard(camera, zombie.texture, zombie.position, 10.8f, WHITE);
}

void UnloadZombie(Zombie &zombie)
{
    UnloadTexture(zombie.texture);
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
