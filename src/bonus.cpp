#include "bonus.h"
#include <rlgl.h>
#include <raymath.h>
#include <cstdio>
#include <cmath>

int LoadBonusSpawns(const char *path, BonusSpawn *spawns, int maxSpawns)
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
                if (c == 'H' || c == 'h') {
                    spawns[count].col = col;
                    spawns[count].row = row;
                    spawns[count].type = BONUS_HEALTH;
                    count++;
                } else if (c == 'K' || c == 'k') {
                    spawns[count].col = col;
                    spawns[count].row = row;
                    spawns[count].type = BONUS_KEY;
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

void InitBonuses(Bonus bonuses[], BonusSpawn spawns[], int count, Texture2D medicTex, Texture2D keyTex, float tileSize)
{
    for (int i = 0; i < count; i++) {
        bonuses[i].position.x = (float)spawns[i].col * tileSize + tileSize / 2.0f;
        bonuses[i].position.y = 0.0f;
        bonuses[i].position.z = (float)spawns[i].row * tileSize + tileSize / 2.0f;
        bonuses[i].texture = (spawns[i].type == BONUS_KEY) ? keyTex : medicTex;
        bonuses[i].active = true;
        bonuses[i].bobTimer = (float)GetRandomValue(0, 100) * 0.1f;
        bonuses[i].type = spawns[i].type;
    }
}

void UpdateBonuses(Bonus bonuses[], int count, Vector3 playerPos, float &health, int maxHealth, bool &hasKey)
{
    float dt = GetFrameTime();
    for (int i = 0; i < count; i++) {
        if (!bonuses[i].active) continue;
        bonuses[i].bobTimer += dt * 3.0f;
        float dx = playerPos.x - bonuses[i].position.x;
        float dz = playerPos.z - bonuses[i].position.z;
        if (dx * dx + dz * dz < 3.5f * 3.5f) {
            bonuses[i].active = false;
            if (bonuses[i].type == BONUS_KEY) {
                hasKey = true;
            } else {
                health += 20.0f;
                if (health > (float)maxHealth) health = (float)maxHealth;
            }
        }
    }
}

void DrawBonuses(Bonus bonuses[], int count, Camera3D camera)
{
    rlDisableDepthTest();
    for (int i = 0; i < count; i++) {
        if (!bonuses[i].active) continue;
        Vector3 pos = bonuses[i].position;
        pos.y = 5.0f + sinf(bonuses[i].bobTimer) * 1.0f;

        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.position, pos));
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, (Vector3){0, 1.0f, 0.0f}));
        Vector3 up = {0, 1.0f, 0};
        float size = 4.0f;

        Vector3 bl = Vector3Subtract(pos, Vector3Add(Vector3Scale(right, size * 0.5f), Vector3Scale(up, size * 0.5f)));
        Vector3 br = Vector3Add(pos, Vector3Subtract(Vector3Scale(right, size * 0.5f), Vector3Scale(up, size * 0.5f)));
        Vector3 tr = Vector3Add(pos, Vector3Add(Vector3Scale(right, size * 0.5f), Vector3Scale(up, size * 0.5f)));
        Vector3 tl = Vector3Add(pos, Vector3Subtract(Vector3Scale(up, size * 0.5f), Vector3Scale(right, size * 0.5f)));

        rlSetTexture(bonuses[i].texture.id);
        rlBegin(RL_QUADS);
            rlColor4ub(255, 255, 255, 255);
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(bl.x, bl.y, bl.z);
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(br.x, br.y, br.z);
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(tr.x, tr.y, tr.z);
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(tl.x, tl.y, tl.z);
        rlEnd();
        rlSetTexture(0);
    }
    rlEnableDepthTest();
}
