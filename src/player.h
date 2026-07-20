#pragma once
#include <raylib.h>
#include "level.h"
#include "door.h"
#include "props.h"

constexpr float PLAYER_SPEED = 20.0f;
constexpr float PLAYER_RADIUS = 1.0f;

struct Scene;

void UpdatePlayer(Camera3D *camera, float *yaw, Level level, Door doors[], int doorCount, Scene &scene);
