#include "spawn.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

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
                if (c == 'Z' || c == 'z' || c == 'M' || c == 'm' || c == 'F' || c == 'f') {
                    spawns[count].col = col;
                    spawns[count].row = row;
                    spawns[count].isMilitary = (c == 'M' || c == 'm');
                    spawns[count].isFast = (c == 'F' || c == 'f');
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

int LoadBonusFile(const char *path, BonusSpawn *spawns, int maxSpawns)
{
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    char line[256];
    int count = 0;
    while (fgets(line, sizeof(line), f) && count < maxSpawns) {
        if (line[0] == '#' || line[0] == '\n') continue;
        char type[16];
        float col = 0, row = 0;
        int p1 = 0;
        int n = sscanf(line, "%15s %f %f %d", type, &col, &row, &p1);
        if (n < 3) continue;

        BonusSpawn &bs = spawns[count];
        bs.col = (int)col;
        bs.row = (int)row;
        bs.weaponIndex = 0;
        bs.keyId = 0;
        bs.healthAmount = BONUS_HEALTH_AMOUNT;

        if (strcmp(type, "health") == 0) {
            bs.type = BONUS_HEALTH;
            if (n >= 4 && p1 > 0) bs.healthAmount = (float)p1;
            count++;
        } else if (strcmp(type, "weapon") == 0) {
            bs.type = BONUS_WEAPON;
            bs.weaponIndex = (n >= 4 && p1 >= 1 && p1 <= 2) ? p1 : 1;
            count++;
        } else if (strcmp(type, "key") == 0) {
            bs.type = BONUS_KEY;
            bs.keyId = (n >= 4) ? p1 : 0;
            if (bs.keyId < 0 || bs.keyId >= MAX_KEYS) bs.keyId = 0;
            count++;
        }
    }

    fclose(f);
    return count;
}

Vector3 LoadPlayerSpawn(const char *path, float tileSize, float wallHeight)
{
    Vector3 pos = {0, wallHeight / 2.0f, 0};

    FILE *f = fopen(path, "r");
    if (!f) return pos;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = (char *)malloc(size + 1);
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);

    int col = 0, row = 0;
    bool inToken = false;

    for (long i = 0; i <= size; i++) {
        char c = buf[i];
        if (c == '\n' || c == '\0') {
            row++;
            col = 0;
            inToken = false;
        } else if (c == ' ') {
            inToken = false;
        } else {
            if (!inToken) {
                if (c == 'P' || c == 'p') {
                    pos.x = col * tileSize + tileSize / 2.0f;
                    pos.z = row * tileSize + tileSize / 2.0f;
                }
                col++;
                inToken = true;
            }
        }
    }

    free(buf);
    return pos;
}
