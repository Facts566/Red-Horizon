#include "spawn.h"
#include <cstdio>
#include <cstdlib>

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
                if (c == 'Z' || c == 'z' || c == 'M' || c == 'm') {
                    spawns[count].col = col;
                    spawns[count].row = row;
                    spawns[count].isMilitary = (c == 'M' || c == 'm');
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
