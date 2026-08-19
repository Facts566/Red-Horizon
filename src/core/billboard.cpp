#include "billboard.h"
#include <raymath.h>
#include <cstdlib>
#include <cmath>
#include <cstring>

static Mesh s_billboardMesh = { 0 };
static Material s_billboardMat = { 0 };

void InitBillboards()
{
    s_billboardMesh = (Mesh){ 0 };

    const int vcount = 6;
    const int tcount = 2;
    s_billboardMesh.vertexCount = vcount;
    s_billboardMesh.triangleCount = tcount;

    s_billboardMesh.vertices = (float *)calloc(vcount * 3, sizeof(float));
    s_billboardMesh.texcoords = (float *)calloc(vcount * 2, sizeof(float));
    s_billboardMesh.normals = (float *)calloc(vcount * 3, sizeof(float));

    float verts[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
    };
    float texs[] = {
        0.0f, 1.0f,  1.0f, 1.0f,  1.0f, 0.0f,
        0.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,
    };
    float norms[] = {
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    };

    memcpy(s_billboardMesh.vertices, verts, sizeof(verts));
    memcpy(s_billboardMesh.texcoords, texs, sizeof(texs));
    memcpy(s_billboardMesh.normals, norms, sizeof(norms));

    UploadMesh(&s_billboardMesh, false);

    s_billboardMat = LoadMaterialDefault();
    s_billboardMat.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
}

void DrawSprite3D(Texture2D texture, Vector3 position, Vector3 cameraPosition, Vector2 size, Color tint)
{
    if (s_billboardMesh.vaoId == 0) return;

    float dx = cameraPosition.x - position.x;
    float dz = cameraPosition.z - position.z;
    float yaw = (dx != 0.0f || dz != 0.0f) ? atan2f(dx, dz) : 0.0f;

    Matrix mScale = MatrixScale(size.x, size.y, 1.0f);
    Matrix mRot = MatrixRotateY(yaw);
    Matrix mTrans = MatrixTranslate(position.x, position.y, position.z);
    Matrix transform = MatrixMultiply(MatrixMultiply(mScale, mRot), mTrans);

    s_billboardMat.maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    s_billboardMat.maps[MATERIAL_MAP_DIFFUSE].color = tint;
    DrawMesh(s_billboardMesh, s_billboardMat, transform);
}

void UnloadBillboards()
{
    if (s_billboardMesh.vaoId != 0) UnloadMesh(s_billboardMesh);
    if (s_billboardMat.maps != NULL) UnloadMaterial(s_billboardMat);
    s_billboardMesh = (Mesh){ 0 };
    s_billboardMat = (Material){ 0 };
}