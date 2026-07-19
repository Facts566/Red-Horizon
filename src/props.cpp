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

void LoadLamp(Lamp &lamp, Shader shader, float tileSize)
{
    lamp.model = LoadModel("models/lamp.obj");
    lamp.texture = LoadTexture("tex/lamp.png");
    SetTextureFilter(lamp.texture, TEXTURE_FILTER_POINT);
    lamp.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = lamp.texture;
    lamp.model.materials[0].shader = shader;
    lamp.scale = 0.5f;
}

void DrawLamp(Lamp &lamp)
{
    Matrix lampTransform = MatrixMultiply(MatrixScale(lamp.scale, lamp.scale, lamp.scale), MatrixTranslate(lamp.position.x, lamp.position.y, lamp.position.z));
    for (int mi = 0; mi < lamp.model.meshCount; mi++)
        DrawMesh(lamp.model.meshes[mi], lamp.model.materials[lamp.model.meshMaterial[mi]], lampTransform);
}

void UnloadLamp(Lamp &lamp)
{
    UnloadModel(lamp.model);
    UnloadTexture(lamp.texture);
}

BoxCollider MakeSofaCollider(Sofa &sofa)
{
    BoundingBox bb = GetMeshBoundingBox(sofa.model.meshes[0]);
    for (int i = 1; i < sofa.model.meshCount; i++)
    {
        BoundingBox b = GetMeshBoundingBox(sofa.model.meshes[i]);
        if (b.min.x < bb.min.x) bb.min.x = b.min.x;
        if (b.min.y < bb.min.y) bb.min.y = b.min.y;
        if (b.min.z < bb.min.z) bb.min.z = b.min.z;
        if (b.max.x > bb.max.x) bb.max.x = b.max.x;
        if (b.max.y > bb.max.y) bb.max.y = b.max.y;
        if (b.max.z > bb.max.z) bb.max.z = b.max.z;
    }
    BoxCollider box;
    box.min.x = sofa.position.x + bb.min.x * sofa.scale;
    box.min.y = sofa.position.y + bb.min.y * sofa.scale;
    box.min.z = sofa.position.z + bb.min.z * sofa.scale;
    box.max.x = sofa.position.x + bb.max.x * sofa.scale;
    box.max.y = sofa.position.y + bb.max.y * sofa.scale;
    box.max.z = sofa.position.z + bb.max.z * sofa.scale;
    return box;
}

bool CheckBoxCollision(BoxCollider box, float x, float z, float radius)
{
    float closestX = (x < box.min.x) ? box.min.x : (x > box.max.x) ? box.max.x : x;
    float closestZ = (z < box.min.z) ? box.min.z : (z > box.max.z) ? box.max.z : z;
    float dx = x - closestX;
    float dz = z - closestZ;
    return (dx * dx + dz * dz) < (radius * radius);
}
