#pragma once
#include <raylib.h>
#include "level.h"
#include "door.h"
#include "props.h"

struct Scene;

void UpdatePlayer(Camera3D *camera, float *yaw, Level level, Door doors[], int doorCount, Scene &scene);
