#pragma once
#include <raylib.h>
#include <vector>
#include "level.h"
#include "door.h"
#include "map.h"

#define WEAPON_COUNT 3

struct WeaponState {
    Texture2D gunTex;
    Texture2D muzzleTex;
    Texture2D shotholeTex;
    Model decalModel;

    float shakeTime;
    float flashTime;
    float flashRotation;
    float fireCooldown;

    int maxAmmo;
    int currentAmmo;
    bool isReloading;
    float reloadTimer;

    float shakeDuration;
    float shakeAmount;
    float fireRate;
    float reloadTime;

    float flashOffsetX;
    float flashOffsetY;
    float flashScale;
    float flashDuration;

    int pelletCount;
    float spread;

    float bobTimer;
    float bobOffsetX;
    float bobOffsetY;

    const char *name;
    bool unlocked;
};

void LoadWeapon(WeaponState &w, Shader shader, Texture2D shotholeTex, const char *gunPath);
void LoadPistol(WeaponState &w, Shader shader, Texture2D shotholeTex);
void LoadDoubleBarreledShotgun(WeaponState &w, Shader shader, Texture2D shotholeTex);
void UpdateWeapon(WeaponState &w);
void UpdateWeaponBob(WeaponState &w, bool isMoving, bool isSprinting);
void ShootWeapon(WeaponState &w, Camera3D camera, Level level, Door doors[], int doorCount, Shader shader, std::vector<BulletHole> &wallHoles);
void DrawWeaponDecals(std::vector<BulletHole> &wallHoles, Model decalModel);
void UnloadWeapon(WeaponState &w);

void DrawWeaponHUD(WeaponState &w, int health, int maxHealth);
void DrawWeaponPanel(WeaponState weapons[], int weaponCount, int currentWeapon);
