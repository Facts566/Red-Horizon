#pragma once
#include <raylib.h>
#include "level.h"

bool RaycastWall(Level level, Vector3 origin, Vector3 dir, float maxDist, Vector3 &hitPos, Vector3 &hitNormal);
