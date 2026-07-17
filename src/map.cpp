#include "map.h"
#include <cstdlib>

Model MakePlane(float w, float l, float tu, float tv, Texture2D tex)
{
    Mesh m = GenMeshPlane(w, l, 1, 1);
    for (int i = 0; i < m.vertexCount; i++) {
        m.texcoords[i*2]   *= tu;
        m.texcoords[i*2+1] *= tv;
    }
    free(m.vboId); m.vboId = NULL; m.vaoId = 0;
    UploadMesh(&m, false);
    Model mod = LoadModelFromMesh(m);
    mod.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex;
    return mod;
}

Model MakeWall(float w, float h, float tu, float tv, Texture2D tex)
{
    Mesh m = { 0 };
    m.vertexCount = 4;
    m.triangleCount = 2;
    m.vertices = (float *)MemAlloc(12 * sizeof(float));
    m.texcoords = (float *)MemAlloc(8 * sizeof(float));
    m.normals = (float *)MemAlloc(12 * sizeof(float));
    m.indices = (unsigned short *)MemAlloc(6 * sizeof(unsigned short));

    float hw = w / 2.0f;
    m.vertices[0]  = -hw; m.vertices[1]  = 0; m.vertices[2]  = 0;
    m.vertices[3]  =  hw; m.vertices[4]  = 0; m.vertices[5]  = 0;
    m.vertices[6]  = -hw; m.vertices[7]  = h; m.vertices[8]  = 0;
    m.vertices[9]  =  hw; m.vertices[10] = h; m.vertices[11] = 0;

    m.texcoords[0] = 0;  m.texcoords[1] = 0;
    m.texcoords[2] = tu; m.texcoords[3] = 0;
    m.texcoords[4] = 0;  m.texcoords[5] = tv;
    m.texcoords[6] = tu; m.texcoords[7] = tv;

    for (int i = 0; i < 4; i++) {
        m.normals[i*3] = 0; m.normals[i*3+1] = 0; m.normals[i*3+2] = 1;
    }

    m.indices[0] = 0; m.indices[1] = 1; m.indices[2] = 2;
    m.indices[3] = 1; m.indices[4] = 3; m.indices[5] = 2;

    UploadMesh(&m, false);
    Model mod = LoadModelFromMesh(m);
    mod.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex;
    return mod;
}
