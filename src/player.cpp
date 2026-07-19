#include "player.h"
#include "door.h"
#include <math.h>

const float PLAYER_SPEED = 20.0f;
const float PLAYER_RADIUS = 1.0f;

void UpdatePlayer(Camera3D *camera, float *yaw, Level level, Door doors[], int doorCount, BoxCollider sofaBox)
{
    *yaw -= GetMouseDelta().x * 0.003f;
    if (IsKeyDown(KEY_LEFT))  *yaw += 2.0f * GetFrameTime();
    if (IsKeyDown(KEY_RIGHT)) *yaw -= 2.0f * GetFrameTime();

    float fx = sinf(*yaw);
    float fz = cosf(*yaw);
    float rx = cosf(*yaw);
    float rz = -sinf(*yaw);
    float dx = 0, dz = 0;

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    { dx += fx; dz += fz; }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))   { dx -= fx; dz -= fz; }
    if (IsKeyDown(KEY_A))                          { dx += rx; dz += rz; }
    if (IsKeyDown(KEY_D))                          { dx -= rx; dz -= rz; }

    float len = sqrtf(dx * dx + dz * dz);
    if (len > 0) { dx /= len; dz /= len; }

    float speed = IsKeyDown(KEY_LEFT_SHIFT) ? PLAYER_SPEED * 1.5f : PLAYER_SPEED;
    float dt = speed * GetFrameTime();

    float nx = camera->position.x + dx * dt;
    float nz = camera->position.z + dz * dt;

    if (!CheckWallCollision(level, nx, camera->position.z, PLAYER_RADIUS) &&
        !CheckAnyDoorCollision(doors, doorCount, nx, camera->position.z, PLAYER_RADIUS) &&
        !CheckBoxCollision(sofaBox, nx, camera->position.z, PLAYER_RADIUS))
        camera->position.x = nx;
    if (!CheckWallCollision(level, camera->position.x, nz, PLAYER_RADIUS) &&
        !CheckAnyDoorCollision(doors, doorCount, camera->position.x, nz, PLAYER_RADIUS) &&
        !CheckBoxCollision(sofaBox, camera->position.x, nz, PLAYER_RADIUS))
        camera->position.z = nz;

    UpdateDoors(doors, doorCount, camera->position);

    camera->target.x = camera->position.x + fx;
    camera->target.y = camera->position.y;
    camera->target.z = camera->position.z + fz;
}
