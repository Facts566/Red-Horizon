#pragma once
#include <raylib.h>
#include "level.h"
#include "door.h"

void UpdatePlayer(Camera3D *camera, float *yaw, Level level, Door *door);
