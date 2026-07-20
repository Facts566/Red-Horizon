#include "scene.h"

void LoadScene(Scene &scene, Shader shader, float tileSize, Vector3 playerStart)
{
    scene.tileSize = tileSize;

    // === КОМНАТА 1: ЗАЛ ===
    LoadSofa(scene.sofa, shader, tileSize);
    scene.sofa.position = (Vector3){12 * tileSize, 0.7f * tileSize, 1.8f * tileSize};
    scene.sofaBox = MakeSofaCollider(scene.sofa);

    // === ЛАМПЫ ===
    scene.lampCount = 0;

    LoadLamp(scene.lamps[scene.lampCount], shader, tileSize);
    scene.lamps[scene.lampCount].position = (Vector3){playerStart.x + 80.0f, 19.0f, playerStart.z + 2.0f};
    scene.lampCount++;

    LoadLamp(scene.lamps[scene.lampCount], shader, tileSize);
    scene.lamps[scene.lampCount].position = (Vector3){playerStart.x + 10.0f, 19.0f, playerStart.z + -70.0f};
    scene.lampCount++;

    // === ЗОМБИ ===

    scene.zombieCount = 1;
    InitZombie(scene.zombies[0], (Vector3){30 * tileSize + tileSize / 2.0f, 5.4f, 4 * tileSize + tileSize / 2.0f});

    // === добавляй новые объекты сюда ===
}

void DrawScene(Scene &scene, Camera3D camera)
{
    DrawSofa(scene.sofa);

    for (int i = 0; i < scene.lampCount; i++)
        DrawLamp(scene.lamps[i]);

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

    for (int i = 0; i < scene.lampCount; i++)
        UnloadLamp(scene.lamps[i]);

    for (int i = 0; i < scene.zombieCount; i++)
        UnloadZombie(scene.zombies[i]);

    // === добавляй выгрузку новых объектов сюда ===
}
