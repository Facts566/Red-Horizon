#pragma once
#include <raylib.h>

Shader LoadLightShader();
void SetLightUniforms(Shader shader, Vector3 pos, Vector3 color, float range, float ambient);
