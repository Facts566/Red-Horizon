#pragma once
#include <raylib.h>
#include "level.h"
#include "door.h"
#include "props.h"

void UpdatePlayer(Camera3D *camera, float *yaw, Level level, Door doors[], int doorCount, BoxCollider sofaBox);
