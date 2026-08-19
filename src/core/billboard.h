#pragma once
#include <raylib.h>

void InitBillboards();
void DrawSprite3D(Texture2D texture, Vector3 position, Vector3 cameraPosition, Vector2 size, Color tint);
void UnloadBillboards();