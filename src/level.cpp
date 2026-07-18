#include "level.h"
#include "map.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

Level LoadLevel(const char *path, float tileSize, float wallHeight)
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
    return level;
}

static bool IsSolid(Level level, int col, int row)
{
    if (col < 0 || col >= level.width || row < 0 || row >= level.height)
        return false;
    char c = level.data[row * level.width + col];
    return c == '&' || c == '@';
}

void DrawLevel(Level level, Texture2D floorTex, Texture2D wallTex, Texture2D greenTex, Shader shader)
{
    float ts = level.tileSize;
    float wh = level.wallHeight;

    Model floorModel = MakePlane(ts, ts, 1.0f, 1.0f, floorTex);
    floorModel.materials[0].shader = shader;

    Model wallN = MakeWall(ts, wh, 1.0f, wh/ts, wallTex);
    wallN.materials[0].shader = shader;
    Model wallS = MakeWall(ts, wh, 1.0f, wh/ts, wallTex);
    wallS.materials[0].shader = shader;
    Model wallW = MakeWall(ts, wh, 1.0f, wh/ts, wallTex);
    wallW.materials[0].shader = shader;
    Model wallE = MakeWall(ts, wh, 1.0f, wh/ts, wallTex);
    wallE.materials[0].shader = shader;

    Model greenN = MakeWall(ts, wh, 1.0f, wh/ts, greenTex);
    greenN.materials[0].shader = shader;
    Model greenS = MakeWall(ts, wh, 1.0f, wh/ts, greenTex);
    greenS.materials[0].shader = shader;
    Model greenW = MakeWall(ts, wh, 1.0f, wh/ts, greenTex);
    greenW.materials[0].shader = shader;
    Model greenE = MakeWall(ts, wh, 1.0f, wh/ts, greenTex);
    greenE.materials[0].shader = shader;

    for (int row = 0; row < level.height; row++) {
        for (int col = 0; col < level.width; col++) {
            char c = level.data[row * level.width + col];
            if (c == ' ') continue;

            float cx = col * ts + ts / 2.0f;
            float cz = row * ts + ts / 2.0f;

            DrawModel(floorModel, (Vector3){cx, 0, cz}, 1.0f, WHITE);

            if (c == '&' || c == '@') {
                Model *n = (c == '@') ? &greenN : &wallN;
                Model *s = (c == '@') ? &greenS : &wallS;
                Model *w = (c == '@') ? &greenW : &wallW;
                Model *e = (c == '@') ? &greenE : &wallE;

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

    UnloadModel(floorModel);
    UnloadModel(wallN);
    UnloadModel(wallS);
    UnloadModel(wallW);
    UnloadModel(wallE);
    UnloadModel(greenN);
    UnloadModel(greenS);
    UnloadModel(greenW);
    UnloadModel(greenE);
}

void UnloadLevel(Level level)
{
    if (level.data) free(level.data);
}
