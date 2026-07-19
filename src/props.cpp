#include "props.h"
#include <raymath.h>

void LoadSofa(Sofa &sofa, Shader shader, float tileSize)
{
    sofa.model = LoadModel("models/sofa.obj");
    sofa.texture = LoadTexture("tex/sofa.png");
    SetTextureFilter(sofa.texture, TEXTURE_FILTER_POINT);
    sofa.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = sofa.texture;
    sofa.model.materials[0].shader = shader;
    sofa.position = {12 * tileSize, 0.7f * tileSize, 1.8f * tileSize};
    sofa.scale = 4.0f;
}

void DrawSofa(Sofa &sofa)
{
    Matrix sofaTransform = MatrixMultiply(MatrixScale(sofa.scale, sofa.scale, sofa.scale), MatrixTranslate(sofa.position.x, sofa.position.y, sofa.position.z));
    for (int mi = 0; mi < sofa.model.meshCount; mi++)
        DrawMesh(sofa.model.meshes[mi], sofa.model.materials[sofa.model.meshMaterial[mi]], sofaTransform);
}

void UnloadSofa(Sofa &sofa)
{
    UnloadModel(sofa.model);
    UnloadTexture(sofa.texture);
}
