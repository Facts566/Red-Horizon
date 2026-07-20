#include "weapon.h"
#include "raycast.h"
#include <raymath.h>
#include <cmath>

void LoadWeapon(WeaponState &w, Shader shader, Texture2D shotholeTex)
{
    w.gunTex = LoadTexture("tex/gun.png");
    w.muzzleTex = LoadTexture("tex/Muzzle.png");
    w.shotholeTex = shotholeTex;
    w.decalModel = MakeWall(0.6f, 0.6f, 1.0f, 1.0f, w.shotholeTex);
    w.decalModel.materials[0].shader = shader;

    w.shakeTime = 0.0f;
    w.flashTime = 0.0f;
    w.flashRotation = 0.0f;
    w.fireCooldown = 0.0f;

    w.maxAmmo = 7;
    w.currentAmmo = w.maxAmmo;
    w.isReloading = false;
    w.reloadTimer = 0.0f;

    w.shakeDuration = 0.3f;
    w.shakeAmount = 1.6f;
    w.fireRate = 0.5f;
    w.reloadTime = 5.0f;

    w.flashOffsetX = 170.0f;
    w.flashOffsetY = 70.0f;
    w.flashScale = 2.0f;
    w.flashDuration = 0.08f;
}

void UpdateWeapon(WeaponState &w)
{
    if (w.fireCooldown > 0.0f)
        w.fireCooldown -= GetFrameTime();

    if (w.isReloading)
    {
        w.reloadTimer -= GetFrameTime();
        if (w.reloadTimer <= 0.0f)
        {
            w.currentAmmo = w.maxAmmo;
            w.isReloading = false;
        }
    }

    if (w.flashTime > 0.0f)
        w.flashTime -= GetFrameTime();
}

void ShootWeapon(WeaponState &w, Camera3D camera, Level level, Door doors[], int doorCount, Shader shader)
{
    if (IsKeyPressed(KEY_R) && !w.isReloading && w.currentAmmo < w.maxAmmo)
    {
        w.isReloading = true;
        w.reloadTimer = w.reloadTime;
    }

    if ((IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_SPACE)) && w.fireCooldown <= 0.0f && !w.isReloading && w.currentAmmo > 0)
    {
        w.currentAmmo--;
        w.fireCooldown = w.fireRate;
        w.shakeTime = w.shakeDuration;
        w.flashTime = w.flashDuration;
        w.flashRotation = (float)GetRandomValue(0, 3600) / 10.0f;

        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, (Vector3){0, 1, 0}));
        Vector3 up = Vector3Normalize(Vector3CrossProduct(right, forward));

        const int PELLET_COUNT = 7;
        const float SPREAD = 0.1f;

        for (int i = 0; i < PELLET_COUNT; i++)
        {
            float rx = ((float)GetRandomValue(-10000, 10000) / 10000.0f) * SPREAD;
            float ry = ((float)GetRandomValue(-10000, 10000) / 10000.0f) * SPREAD;

            Vector3 dir = Vector3Normalize(Vector3Add(forward, Vector3Add(Vector3Scale(right, rx), Vector3Scale(up, ry))));

            Vector3 wallPos, wallNorm;
            bool wallHit = RaycastWall(level, camera.position, dir, 100.0f, wallPos, wallNorm);

            Vector3 doorPos, doorNorm;
            bool doorHit = false;
            int hitDoorIdx = -1;

            for (int d = 0; d < doorCount; d++)
            {
                if (!doors[d].isOpen && RayDoorIntersect(doors[d], camera.position, dir, 100.0f, doorPos, doorNorm))
                {
                    doorHit = true;
                    hitDoorIdx = d;
                    break;
                }
            }

            if (wallHit || doorHit)
            {
                bool hitDoor = false;
                Vector3 bestPos, bestNorm;

                if (wallHit && doorHit)
                {
                    float wd = Vector3Distance(camera.position, wallPos);
                    float dd = Vector3Distance(camera.position, doorPos);
                    if (dd < wd)
                    {
                        hitDoor = true;
                        bestPos = doorPos;
                        bestNorm = doorNorm;
                    }
                    else
                    {
                        bestPos = wallPos;
                        bestNorm = wallNorm;
                    }
                }
                else if (wallHit)
                {
                    bestPos = wallPos;
                    bestNorm = wallNorm;
                }
                else
                {
                    hitDoor = true;
                    bestPos = doorPos;
                    bestNorm = doorNorm;
                }

                bestPos.x += bestNorm.x * 0.05f;
                bestPos.z += bestNorm.z * 0.05f;

                if (hitDoor && hitDoorIdx >= 0)
                {
                    if (doors[hitDoorIdx].bulletHoles.size() >= 50)
                        doors[hitDoorIdx].bulletHoles.erase(doors[hitDoorIdx].bulletHoles.begin());
                    doors[hitDoorIdx].bulletHoles.push_back({bestPos, bestNorm});
                }
                else
                {
                    if (w.bulletHoles.size() >= 50)
                        w.bulletHoles.erase(w.bulletHoles.begin());
                    w.bulletHoles.push_back({bestPos, bestNorm});
                }
            }
        }
    }
}

void DrawWeaponDecals(WeaponState &w, Model decalModel)
{
    for (auto &bh : w.bulletHoles)
    {
        Vector3 p = bh.pos;
        p.y -= 0.3f;
        float decalAngle = atan2f(bh.normal.x, bh.normal.z) * 180.0f / 3.14159f;
        DrawModelEx(decalModel, p, (Vector3){0,1,0}, decalAngle, (Vector3){1,1,1}, WHITE);
    }
}

void UnloadWeapon(WeaponState &w)
{
    UnloadTexture(w.gunTex);
    UnloadTexture(w.muzzleTex);
    UnloadModel(w.decalModel);
}

void DrawWeaponHUD(WeaponState &w, int health, int maxHealth)
{
    float gunScale = 5.0f;
    float gunKickY = 0.0f;

    if (w.shakeTime > 0.0f)
    {
        float t = w.shakeTime / w.shakeDuration;
        float intensity = w.shakeAmount * t;
        gunKickY = -intensity * 12.0f;
        gunScale = 5.0f + 0.2f;
    }

    Vector2 gunPos = {
        (float)GetScreenWidth()/2 - w.gunTex.width*gunScale/2,
        (float)GetScreenHeight() - w.gunTex.height*gunScale + gunKickY + 50.0f
    };

    if (w.flashTime > 0.0f)
    {
        Vector2 flashCenter = {
            gunPos.x + w.flashOffsetX * gunScale,
            gunPos.y + w.flashOffsetY * gunScale
        };
        float fw = (float)w.muzzleTex.width * w.flashScale;
        float fh = (float)w.muzzleTex.height * w.flashScale;
        Rectangle flashDest = {flashCenter.x - fw/2, flashCenter.y - fh/2, fw, fh};
        DrawTexturePro(w.muzzleTex, (Rectangle){0, 0, (float)w.muzzleTex.width, (float)w.muzzleTex.height}, flashDest, (Vector2){fw/2, fh/2}, w.flashRotation, WHITE);
    }

    DrawTextureEx(w.gunTex, gunPos, 0.0f, gunScale, WHITE);

    const char *ammoText = TextFormat("%i / %i", w.currentAmmo, w.maxAmmo);
    int ammoFontSize = 40;
    int ammoTextWidth = MeasureText(ammoText, ammoFontSize);
    DrawText(ammoText, GetScreenWidth() / 2 - ammoTextWidth / 2, GetScreenHeight() - 120, ammoFontSize, WHITE);

    if (w.isReloading)
    {
        const char *reloadText = "RELOADING...";
        int reloadFontSize = 24;
        int reloadTextWidth = MeasureText(reloadText, reloadFontSize);
        int reloadY = GetScreenHeight() - 80;
        DrawText(reloadText, GetScreenWidth() / 2 - reloadTextWidth / 2, reloadY, reloadFontSize, YELLOW);

        float reloadProgress = 1.0f - w.reloadTimer / w.reloadTime;
        int barWidth = 200;
        int barHeight = 8;
        int barX = GetScreenWidth() / 2 - barWidth / 2;
        int barY = reloadY + 30;
        DrawRectangle(barX, barY, barWidth, barHeight, DARKGRAY);
        DrawRectangle(barX, barY, (int)(barWidth * reloadProgress), barHeight, YELLOW);
    }

    int hpBarWidth = 375;
    int hpBarHeight = 33;
    int hpBarX = 20;
    int hpBarY = GetScreenHeight() - 70;
    float hpPercent = (float)health / maxHealth;
    Color hpColor = hpPercent > 0.5f ? GREEN : (hpPercent > 0.25f ? ORANGE : RED);
    DrawRectangle(hpBarX - 3, hpBarY - 3, hpBarWidth + 6, hpBarHeight + 6, GRAY);
    DrawRectangle(hpBarX, hpBarY, hpBarWidth, hpBarHeight, ColorAlpha(BLACK, 0.7f));
    DrawRectangle(hpBarX, hpBarY, (int)(hpBarWidth * hpPercent), hpBarHeight, hpColor);
    DrawText(TextFormat("HP: %i / %i", health, maxHealth), hpBarX + 15, hpBarY + 4, 27, WHITE);

    DrawFPS(10, 10);
}
