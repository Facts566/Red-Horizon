#include "bonus.h"
#include "config.h"
#include <rlgl.h>
#include <raymath.h>
#include <cmath>

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

void UpdateBonuses(Bonus bonuses[], int count, Vector3 playerPos, float &health, int maxHealth, bool &hasKey, Sound itemSound)
{
    float dt = GetFrameTime();
    for (int i = 0; i < count; i++) {
        if (!bonuses[i].active) continue;
        bonuses[i].bobTimer += dt * BONUS_BOB_SPEED;
        float dx = playerPos.x - bonuses[i].position.x;
        float dz = playerPos.z - bonuses[i].position.z;
        if (dx * dx + dz * dz < BONUS_PICKUP_RADIUS * BONUS_PICKUP_RADIUS) {
            if (bonuses[i].type == BONUS_KEY) {
                bonuses[i].active = false;
                if (itemSound.frameCount > 0)
                    PlaySound(itemSound);
                hasKey = true;
            } else if (health < (float)maxHealth) {
                bonuses[i].active = false;
                if (itemSound.frameCount > 0)
                    PlaySound(itemSound);
                health += BONUS_HEALTH_AMOUNT;
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
        pos.y = BONUS_Y_HEIGHT + sinf(bonuses[i].bobTimer) * BONUS_BOB_AMPLITUDE;

        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.position, pos));
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, (Vector3){0, 1.0f, 0.0f}));
        Vector3 up = {0, 1.0f, 0};

        Vector3 bl = Vector3Subtract(pos, Vector3Add(Vector3Scale(right, BONUS_SIZE * 0.5f), Vector3Scale(up, BONUS_SIZE * 0.5f)));
        Vector3 br = Vector3Add(pos, Vector3Subtract(Vector3Scale(right, BONUS_SIZE * 0.5f), Vector3Scale(up, BONUS_SIZE * 0.5f)));
        Vector3 tr = Vector3Add(pos, Vector3Add(Vector3Scale(right, BONUS_SIZE * 0.5f), Vector3Scale(up, BONUS_SIZE * 0.5f)));
        Vector3 tl = Vector3Add(pos, Vector3Subtract(Vector3Scale(up, BONUS_SIZE * 0.5f), Vector3Scale(right, BONUS_SIZE * 0.5f)));

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
