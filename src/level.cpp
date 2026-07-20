#include "level.h"
#include "map.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

Level LoadLevel(const char *path, float tileSize, float wallHeight, Texture2D floorTex, Texture2D planksTex, Texture2D wallTex, Texture2D greenTex, Shader shader)
{
    Level level = { 0 };
    level.tileSize = tileSize;
    level.wallHeight = wallHeight;
    level.playerStart = (Vector3){0, 0, 0};

    FILE *f = fopen(path, "r");
    if (!f) return level;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = (char *)malloc(size + 1);
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);

    int maxW = 0;
    int h = 0;
    int w = 0;

    for (long i = 0; i <= size; i++) {
        if (buf[i] == '\n' || buf[i] == '\0') {
            if (w > maxW) maxW = w;
            w = 0;
            h++;
        } else if (buf[i] != ' ') {
            w++;
        }
    }

    level.width = maxW;
    level.height = h;
    level.data = (char *)calloc(maxW * h, sizeof(char));

    int col = 0, row = 0;
    for (long i = 0; i <= size; i++) {
        if (buf[i] == '\n' || buf[i] == '\0') {
            row++;
            col = 0;
        } else if (buf[i] != ' ') {
            level.data[row * maxW + col] = buf[i];
            if (buf[i] == 'P') {
                level.playerStart.x = col * tileSize + tileSize / 2.0f;
                level.playerStart.z = row * tileSize + tileSize / 2.0f;
            }
            col++;
        }
    }

    level.playerStart.y = wallHeight / 2.0f;
    free(buf);

    float ts = tileSize;
    float wh = wallHeight;

    level.models.floor = MakePlane(ts, ts, 1.0f, 1.0f, floorTex);
    level.models.floor.materials[0].shader = shader;

    level.models.planks = MakePlane(ts, ts, 1.0f, 1.0f, planksTex);
    level.models.planks.materials[0].shader = shader;

    level.models.wallN = MakeWall(ts, wh, 1.0f, wh/ts, wallTex);
    level.models.wallN.materials[0].shader = shader;
    level.models.wallS = MakeWall(ts, wh, 1.0f, wh/ts, wallTex);
    level.models.wallS.materials[0].shader = shader;
    level.models.wallW = MakeWall(ts, wh, 1.0f, wh/ts, wallTex);
    level.models.wallW.materials[0].shader = shader;
    level.models.wallE = MakeWall(ts, wh, 1.0f, wh/ts, wallTex);
    level.models.wallE.materials[0].shader = shader;

    level.models.greenN = MakeWall(ts, wh, 1.0f, wh/ts, greenTex);
    level.models.greenN.materials[0].shader = shader;
    level.models.greenS = MakeWall(ts, wh, 1.0f, wh/ts, greenTex);
    level.models.greenS.materials[0].shader = shader;
    level.models.greenW = MakeWall(ts, wh, 1.0f, wh/ts, greenTex);
    level.models.greenW.materials[0].shader = shader;
    level.models.greenE = MakeWall(ts, wh, 1.0f, wh/ts, greenTex);
    level.models.greenE.materials[0].shader = shader;

    level.models.ceiling = MakePlane(ts, ts, 1.0f, 1.0f, floorTex);
    level.models.ceiling.materials[0].shader = shader;

    return level;
}

static bool IsSolid(Level level, int col, int row)
{
    if (col < 0 || col >= level.width || row < 0 || row >= level.height)
        return false;
    char c = level.data[row * level.width + col];
    return c == '&' || c == '@';
}

bool CheckWallCollision(Level level, float x, float z, float radius)
{
    float ts = level.tileSize;

    if (x - radius < 0 || x + radius > level.width * ts ||
        z - radius < 0 || z + radius > level.height * ts)
        return true;

    int minCol = (int)((x - radius) / ts);
    int maxCol = (int)((x + radius) / ts);
    int minRow = (int)((z - radius) / ts);
    int maxRow = (int)((z + radius) / ts);

    for (int row = minRow; row <= maxRow; row++) {
        for (int col = minCol; col <= maxCol; col++) {
            if (IsSolid(level, col, row)) {
                float left = col * ts;
                float right = left + ts;
                float top = row * ts;
                float bottom = top + ts;

                float closestX = (x < left) ? left : (x > right) ? right : x;
                float closestZ = (z < top) ? top : (z > bottom) ? bottom : z;

                float dx = x - closestX;
                float dz = z - closestZ;

                if (dx * dx + dz * dz < radius * radius)
                    return true;
            }
        }
    }
    return false;
}

void DrawLevel(Level level)
{
    float ts = level.tileSize;
    LevelModels &m = level.models;

    for (int row = 0; row < level.height; row++) {
        for (int col = 0; col < level.width; col++) {
            char c = level.data[row * level.width + col];
            if (c == ' ') continue;

            float cx = col * ts + ts / 2.0f;
            float cz = row * ts + ts / 2.0f;

            DrawModel(m.floor, (Vector3){cx, 0, cz}, 1.0f, WHITE);
            if (c == '0')
                DrawModel(m.planks, (Vector3){cx, 0, cz}, 1.0f, WHITE);

            if (c == '@' || c == '0') {
                DrawModelEx(m.ceiling, (Vector3){cx, level.wallHeight, cz}, (Vector3){1,0,0}, 180.0f, (Vector3){1,1,1}, WHITE);
            }

            if (c == '&' || c == '@') {
                Model *n = (c == '@') ? &m.greenN : &m.wallN;
                Model *s = (c == '@') ? &m.greenS : &m.wallS;
                Model *w = (c == '@') ? &m.greenW : &m.wallW;
                Model *e = (c == '@') ? &m.greenE : &m.wallE;

                if (!IsSolid(level, col, row - 1))
                    DrawModelEx(*n, (Vector3){cx, 0, row * ts}, (Vector3){0,1,0}, 180.0f, (Vector3){1,1,1}, WHITE);
                if (!IsSolid(level, col, row + 1))
                    DrawModel(*s, (Vector3){cx, 0, (row + 1) * ts}, 1.0f, WHITE);
                if (!IsSolid(level, col - 1, row))
                    DrawModelEx(*w, (Vector3){col * ts, 0, cz}, (Vector3){0,1,0}, -90.0f, (Vector3){1,1,1}, WHITE);
                if (!IsSolid(level, col + 1, row))
                    DrawModelEx(*e, (Vector3){(col + 1) * ts, 0, cz}, (Vector3){0,1,0}, 90.0f, (Vector3){1,1,1}, WHITE);
            }
        }
    }
}

void UnloadLevel(Level level)
{
    if (level.data) free(level.data);
    UnloadModel(level.models.floor);
    UnloadModel(level.models.planks);
    UnloadModel(level.models.ceiling);
    UnloadModel(level.models.wallN);
    UnloadModel(level.models.wallS);
    UnloadModel(level.models.wallW);
    UnloadModel(level.models.wallE);
    UnloadModel(level.models.greenN);
    UnloadModel(level.models.greenS);
    UnloadModel(level.models.greenW);
    UnloadModel(level.models.greenE);
}

int LoadZombieSpawns(const char *path, ZombieSpawn *spawns, int maxSpawns)
{
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = (char *)malloc(size + 1);
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);

    int count = 0;
    int col = 0;
    int row = 0;
    bool inToken = false;

    for (long i = 0; i <= size && count < maxSpawns; i++) {
        char c = buf[i];
        if (c == '\n' || c == '\0') {
            row++;
            col = 0;
            inToken = false;
        } else if (c == ' ') {
            inToken = false;
        } else {
            if (!inToken) {
                if (c == 'Z' || c == 'z') {
                    spawns[count].col = col;
                    spawns[count].row = row;
                    count++;
                }
                col++;
                inToken = true;
            }
        }
    }

    free(buf);
    return count;
}
