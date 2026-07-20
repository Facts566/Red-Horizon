#pragma once
#include <raylib.h>

struct BoxCollider {
    Vector3 min;
    Vector3 max;
};

bool CheckBoxCollision(BoxCollider box, float x, float z, float radius);
