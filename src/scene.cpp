#include "scene.h"

void LoadScene(Scene &scene, Shader shader, float tileSize, Vector3 playerStart)
{
    scene.tileSize = tileSize;

    // === КОМНАТА 1: Стартовая ===
    LoadSofa(scene.sofa, shader, tileSize);
    scene.sofa.position = (Vector3){12 * tileSize, 0.7f * tileSize, 1.8f * tileSize};
    scene.sofaBox = MakeSofaCollider(scene.sofa);

    LoadLamp(scene.lamp, shader, tileSize);
    scene.lamp.position = (Vector3){playerStart.x + 2.0f, 0, playerStart.z + 2.0f};

    scene.zombieCount = 1;
    InitZombie(scene.zombies[0], (Vector3){30 * tileSize + tileSize / 2.0f, 5.4f, 4 * tileSize + tileSize / 2.0f});

    // === добавляй новые объекты сюда ===
}

void DrawScene(Scene &scene, Camera3D camera)
{
    DrawSofa(scene.sofa);
    DrawLamp(scene.lamp);

    for (int i = 0; i < scene.zombieCount; i++)
        DrawZombie(scene.zombies[i], camera);

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

    for (int i = 0; i < scene.zombieCount; i++)
        UnloadZombie(scene.zombies[i]);

    // === добавляй выгрузку новых объектов сюда ===
}
