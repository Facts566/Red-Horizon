#include "player.h"
#include <math.h>

const float PLAYER_SPEED = 20.0f;

void UpdatePlayer(Camera3D *camera, float *yaw)
{
    *yaw -= GetMouseDelta().x * 0.003f;

    float fx = sinf(*yaw);
    float fz = cosf(*yaw);
    float rx = cosf(*yaw);
    float rz = -sinf(*yaw);
    float dx = 0, dz = 0;

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    { dx += fx; dz += fz; }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))   { dx -= fx; dz -= fz; }
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))   { dx += rx; dz += rz; }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))  { dx -= rx; dz -= rz; }

    float len = sqrtf(dx * dx + dz * dz);
    if (len > 0) { dx /= len; dz /= len; }

    float speed = IsKeyDown(KEY_LEFT_SHIFT) ? PLAYER_SPEED * 1.5f : PLAYER_SPEED;
    float dt = speed * GetFrameTime();
    camera->position.x += dx * dt;
    camera->position.z += dz * dt;
    camera->target.x = camera->position.x + fx;
    camera->target.y = camera->position.y;
    camera->target.z = camera->position.z + fz;
}
