#include "props.h"

bool CheckBoxCollision(BoxCollider box, float x, float z, float radius)
{
    float closestX = (x < box.min.x) ? box.min.x : (x > box.max.x) ? box.max.x : x;
    float closestZ = (z < box.min.z) ? box.min.z : (z > box.max.z) ? box.max.z : z;
    float dx = x - closestX;
    float dz = z - closestZ;
    return (dx * dx + dz * dz) < (radius * radius);
}
