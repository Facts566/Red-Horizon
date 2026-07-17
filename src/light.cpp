#include "light.h"

Shader LoadLightShader()
{
    return LoadShader("src/shaders/vert.glsl", "src/shaders/frag.glsl");
}

void SetLightUniforms(Shader shader, Vector3 pos, Vector3 color, float range, float ambient)
{
    SetShaderValue(shader, GetShaderLocation(shader, "lightPosition"), &pos, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, GetShaderLocation(shader, "lightColor"), &color, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, GetShaderLocation(shader, "lightRange"), &range, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, GetShaderLocation(shader, "ambientStrength"), &ambient, SHADER_UNIFORM_FLOAT);
}
