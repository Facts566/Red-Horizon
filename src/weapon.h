#pragma once
#include <raylib.h>
#include <vector>
#include "level.h"
#include "door.h"
#include "map.h"

#define WEAPON_COUNT 2

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

    std::vector<BulletHole> bulletHoles;

    float shakeDuration;
    float shakeAmount;
    float fireRate;
    float reloadTime;

    float flashOffsetX;
    float flashOffsetY;
    float flashScale;
    float flashDuration;

    const char *name;
};

void LoadWeapon(WeaponState &w, Shader shader, Texture2D shotholeTex, const char *gunPath);
void LoadMachineGun(WeaponState &w, Shader shader, Texture2D shotholeTex);
void UpdateWeapon(WeaponState &w);
void ShootWeapon(WeaponState &w, Camera3D camera, Level level, Door doors[], int doorCount, Shader shader);
void DrawWeaponDecals(WeaponState &w, Model decalModel);
void UnloadWeapon(WeaponState &w);

void DrawWeaponHUD(WeaponState &w, int health, int maxHealth);
