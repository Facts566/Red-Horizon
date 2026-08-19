#include "bonus.h"
#include "config.h"
#include "billboard.h"
#include <rlgl.h>
#include <raymath.h>
#include <cmath>

void InitBonuses(Bonus bonuses[], BonusSpawn spawns[], int count, Texture2D medicTex, Texture2D keyTex, Texture2D weaponTex, Texture2D weaponTex2, float tileSize)
{
    for (int i = 0; i < count; i++) {
        bonuses[i].position.x = (float)spawns[i].col * tileSize + tileSize / 2.0f;
        bonuses[i].position.y = 0.0f;
        bonuses[i].position.z = (float)spawns[i].row * tileSize + tileSize / 2.0f;
        if (spawns[i].type == BONUS_KEY)
            bonuses[i].texture = keyTex;
        else if (spawns[i].type == BONUS_WEAPON)
            bonuses[i].texture = (spawns[i].weaponIndex == 2) ? weaponTex2 : weaponTex;
        else
            bonuses[i].texture = medicTex;
        bonuses[i].active = true;
        bonuses[i].bobTimer = (float)GetRandomValue(0, 100) * 0.1f;
        bonuses[i].type = spawns[i].type;
        bonuses[i].weaponIndex = spawns[i].weaponIndex;
        bonuses[i].keyId = spawns[i].keyId;
        bonuses[i].healthAmount = spawns[i].healthAmount;
    }
}

void UpdateBonuses(Bonus bonuses[], int count, Vector3 playerPos, float &health, int maxHealth, bool hasKeys[], WeaponState weapons[], int weaponCount, Sound itemSound)
{
    float dt = GetFrameTime();
    for (int i = 0; i < count; i++) {
        if (!bonuses[i].active) continue;
        bonuses[i].bobTimer += dt * BONUS_BOB_SPEED;
        float dx = playerPos.x - bonuses[i].position.x;
        float dz = playerPos.z - bonuses[i].position.z;
        if (dx * dx + dz * dz < BONUS_PICKUP_RADIUS * BONUS_PICKUP_RADIUS) {
            if (bonuses[i].type == BONUS_KEY) {
                int kid = bonuses[i].keyId;
                if (kid >= 0 && kid < MAX_KEYS && !hasKeys[kid]) {
                    bonuses[i].active = false;
                    if (itemSound.frameCount > 0)
                        PlaySound(itemSound);
                    hasKeys[kid] = true;
                }
            } else if (bonuses[i].type == BONUS_WEAPON) {
                if (bonuses[i].weaponIndex >= 0 && bonuses[i].weaponIndex < weaponCount && !weapons[bonuses[i].weaponIndex].unlocked) {
                    bonuses[i].active = false;
                    if (itemSound.frameCount > 0)
                        PlaySound(itemSound);
                    weapons[bonuses[i].weaponIndex].unlocked = true;
                }
            } else if (health < (float)maxHealth) {
                bonuses[i].active = false;
                if (itemSound.frameCount > 0)
                    PlaySound(itemSound);
                health += bonuses[i].healthAmount;
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

        DrawSprite3D(bonuses[i].texture, pos, camera.position, (Vector2){BONUS_SIZE, BONUS_SIZE}, WHITE);
    }
    rlEnableDepthTest();
}
