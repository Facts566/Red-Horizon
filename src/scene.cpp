#include "scene.h"

void LoadScene(Scene &scene, Shader shader, float tileSize, Vector3 playerStart)
{
    scene.tileSize = tileSize;

    // === КОМНАТА 1: ЗАЛ ===
    LoadSofa(scene.sofa, shader, tileSize);
    scene.sofa.position = (Vector3){12 * tileSize, 0.7f * tileSize, 1.8f * tileSize};
    scene.sofaBox = MakeSofaCollider(scene.sofa);

    LoadLamp(scene.lamp, shader, tileSize);
    scene.lamp.position = (Vector3){playerStart.x + 80.0f, 19, playerStart.z + 2.0f};

    // === добавляй новые объекты сюда ===
}

void DrawScene(Scene &scene)
{
    DrawSofa(scene.sofa);
    DrawLamp(scene.lamp);

    // === добавляй отрисовку новых объектов сюда ===
}

BoxCollider GetSofaCollider(Scene &scene)
{
    return scene.sofaBox;
}

void UnloadScene(Scene &scene)
{
    UnloadSofa(scene.sofa);
    UnloadLamp(scene.lamp);

    // === добавляй выгрузку новых объектов сюда ===
}
