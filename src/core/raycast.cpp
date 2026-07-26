#include "raycast.h"
#include <cmath>

bool RaycastWall(Level level, Vector3 origin, Vector3 dir, float maxDist, Vector3 &hitPos, Vector3 &hitNormal)
{
    float ts = level.tileSize;
    float wh = level.wallHeight;

    int col = (int)(origin.x / ts);
    int row = (int)(origin.z / ts);
    if (origin.x < 0) col--;
    if (origin.z < 0) row--;

    int stepX = (dir.x > 0) ? 1 : (dir.x < 0) ? -1 : 0;
    int stepZ = (dir.z > 0) ? 1 : (dir.z < 0) ? -1 : 0;

    float tMaxX = (dir.x != 0) ? ((col + (stepX > 0 ? 1 : 0)) * ts - origin.x) / dir.x : INFINITY;
    float tMaxZ = (dir.z != 0) ? ((row + (stepZ > 0 ? 1 : 0)) * ts - origin.z) / dir.z : INFINITY;
    if (tMaxX < 0) tMaxX = 0;
    if (tMaxZ < 0) tMaxZ = 0;

    float tDeltaX = (dir.x != 0) ? ts / fabsf(dir.x) : INFINITY;
    float tDeltaZ = (dir.z != 0) ? ts / fabsf(dir.z) : INFINITY;

    float t = 0;
    bool steppedX = false;

    for (int i = 0; i < 200; i++)
    {
        if (col >= 0 && col < level.width && row >= 0 && row < level.height)
        {
            char c = level.data[row * level.width + col];
            if (c == '&' || c == '@' || c == '#')
            {
                hitPos.x = origin.x + dir.x * t;
                hitPos.y = origin.y + dir.y * t;
                hitPos.z = origin.z + dir.z * t;

                if (hitPos.y < 0 || hitPos.y > wh)
                    return false;

                hitNormal = steppedX ? (Vector3){(float)-stepX, 0, 0} : (Vector3){0, 0, (float)-stepZ};
                return true;
            }
        }

        if (tMaxX < tMaxZ)
        {
            t = tMaxX;
            tMaxX += tDeltaX;
            col += stepX;
            steppedX = true;
        }
        else
        {
            t = tMaxZ;
            tMaxZ += tDeltaZ;
            row += stepZ;
            steppedX = false;
        }

        if (t > maxDist) break;
    }
    return false;
}
