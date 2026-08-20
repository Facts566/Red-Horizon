#include "menu3d.h"
#include "config.h"
#include "light.h"
#include "billboard.h"
#include <rlgl.h>
#include <raymath.h>
#include <cmath>
#include <cstdio>

#define MENU_ROOM_W 15
#define MENU_ROOM_H 15
#define MENU_ZOMBIE_SIZE 8.0f

static void PickZombieTarget(MenuZombie &z, int roomW, int roomH, float tileSize)
{
    int col = GetRandomValue(1, roomW - 2);
    int row = GetRandomValue(1, roomH - 2);
    z.target = (Vector3){col * tileSize + tileSize / 2.0f, 0.0f, row * tileSize + tileSize / 2.0f};
}

static Texture2D LoadMenuTex(const char *path)
{
    Texture2D tex = LoadTexture(path);
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);
    SetTextureWrap(tex, TEXTURE_WRAP_REPEAT);
    return tex;
}

static Level BuildMenuRoom(Texture2D floorTex, Texture2D planksTex, Texture2D brickTex, Texture2D whiteTex, Shader shader)
{
    const char *path = "/tmp/red_horizon_menu_level.txt";

    FILE *f = fopen(path, "w");
    if (f) {
        for (int row = 0; row < MENU_ROOM_H; row++) {
            for (int col = 0; col < MENU_ROOM_W; col++) {
                char c = '0';
                if (row == 0 || row == MENU_ROOM_H - 1 || col == 0 || col == MENU_ROOM_W - 1)
                    c = '#';
                else if (col % 4 == 2 && row % 4 == 2)
                    c = '&';
                fputc(c, f);
            }
            fputc('\n', f);
        }
        fclose(f);
    }

    Level lvl = LoadLevel(path, TILE_SIZE, WALL_HEIGHT, floorTex, planksTex, brickTex, brickTex, whiteTex, shader);
    remove(path);
    return lvl;
}

void InitMenu3D(Menu3D &m)
{
    m = {};
    m.floorTex  = LoadMenuTex("tex/map/floor.png");
    m.planksTex = LoadMenuTex("tex/map/planks.png");
    m.brickTex  = LoadMenuTex("tex/map/bricks.png");
    m.whiteTex  = LoadMenuTex("tex/map/white_wall.png");

    m.lightShader = LoadLightShader();
    m.blurShader  = LoadShader(0, "src/shaders/blur.fs");
    m.blurTexelLoc = GetShaderLocation(m.blurShader, "texelSize");
    m.blurDirLoc   = GetShaderLocation(m.blurShader, "blurDirection");

    m.sceneRT = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    m.blurRT  = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

    m.level = BuildMenuRoom(m.floorTex, m.planksTex, m.brickTex, m.whiteTex, m.lightShader);

    m.zombiIdle  = LoadMenuTex("tex/zombi/zombi.png");
    m.zombiWalk1 = LoadMenuTex("tex/zombi/zombi_walk.png");
    m.zombiWalk2 = LoadMenuTex("tex/zombi/zombi_walk_1.png");

    m.zombieCount = MENU_MAX_ZOMBIES;
    for (int i = 0; i < m.zombieCount; i++) {
        MenuZombie &z = m.zombies[i];
        PickZombieTarget(z, MENU_ROOM_W, MENU_ROOM_H, TILE_SIZE);
        z.position = z.target;
        z.position.y = MENU_ZOMBIE_SIZE * 0.5f;
        z.speed = 0.15f + (float)GetRandomValue(0, 100) * 0.005f;
        z.animTimer = 0.0f;
        z.stateTimer = 0.0f;
        z.animFrame = 0;
        z.moving = false;
    }
    m.time = 0.0f;

    m.camera.up = (Vector3){0, 1, 0};
    m.camera.fovy = CAMERA_FOVY;
    m.camera.projection = CAMERA_PERSPECTIVE;

    rlDisableBackfaceCulling();
    m.ready = true;
}

void DrawMenu3D(Menu3D &m, float dt)
{
    if (!m.ready) return;
    m.time += dt;

    float cx = (float)m.level.width * m.level.tileSize / 2.0f;
    float cz = (float)m.level.height * m.level.tileSize / 2.0f;
    float radius = 26.0f;
    float lookRadius = 12.0f;
    float a = m.time * 0.12f;

    m.camera.position = (Vector3){cx + cosf(a) * radius, 14.0f, cz + sinf(a) * radius};
    m.camera.target   = (Vector3){cx + cosf(a * 0.7f) * lookRadius, 10.0f, cz + sinf(a * 0.7f) * lookRadius};

    SetLightUniforms(m.lightShader, m.camera.position, (Vector3){1, 1, 1}, 150.0f, 0.32f);

    for (int i = 0; i < m.zombieCount; i++) {
        MenuZombie &z = m.zombies[i];
        Vector3 dir = Vector3Subtract(z.target, z.position);
        dir.y = 0.0f;
        float dist = Vector3Length(dir);
        if (dist < 0.6f) {
            z.moving = false;
            z.stateTimer += dt;
            if (z.stateTimer > 2.0f + (float)i * 0.4f) {
                PickZombieTarget(z, MENU_ROOM_W, MENU_ROOM_H, TILE_SIZE);
                z.stateTimer = 0.0f;
            }
        } else {
            z.moving = true;
            z.stateTimer = 0.0f;
            Vector3 step = Vector3Scale(Vector3Normalize(dir), z.speed * dt);
            z.position = Vector3Add(z.position, step);
            z.yaw = atan2f(dir.x, dir.z);
        }

        if (z.moving) {
            z.animTimer -= dt;
            if (z.animTimer <= 0.0f) {
                z.animTimer = 0.4f;
                z.animFrame ^= 1;
            }
        }
    }

    BeginTextureMode(m.sceneRT);
        ClearBackground(BLACK);
        BeginMode3D(m.camera);
            DrawLevel(m.level);
            for (int i = 0; i < m.zombieCount; i++) {
                MenuZombie &z = m.zombies[i];
                Texture2D tex = (z.moving) ? (z.animFrame ? m.zombiWalk1 : m.zombiWalk2) : m.zombiIdle;
                float bob = z.moving ? fabsf(sinf(m.time * 8.0f)) * 0.6f : sinf(m.time * 1.2f + (float)i) * 0.3f;
                Vector3 pos = z.position;
                pos.y = MENU_ZOMBIE_SIZE * 0.5f + bob;
                DrawSprite3D(tex, pos, m.camera.position, (Vector2){MENU_ZOMBIE_SIZE, MENU_ZOMBIE_SIZE}, WHITE);
            }
        EndMode3D();
    EndTextureMode();

    float blur = 3.2f + sinf(m.time * 0.6f) * 1.2f;
    Vector2 texel = {blur / (float)m.sceneRT.texture.width, blur / (float)m.sceneRT.texture.height};

    Vector2 dirH = {1.0f, 0.0f};
    BeginTextureMode(m.blurRT);
        ClearBackground(BLACK);
        BeginShaderMode(m.blurShader);
            SetShaderValue(m.blurShader, m.blurTexelLoc, &texel, SHADER_UNIFORM_VEC2);
            SetShaderValue(m.blurShader, m.blurDirLoc, &dirH, SHADER_UNIFORM_VEC2);
            DrawTextureRec(m.sceneRT.texture, (Rectangle){0, 0, (float)m.sceneRT.texture.width, -(float)m.sceneRT.texture.height}, (Vector2){0, 0}, WHITE);
        EndShaderMode();
    EndTextureMode();

    Vector2 dirV = {0.0f, 1.0f};
    BeginShaderMode(m.blurShader);
        SetShaderValue(m.blurShader, m.blurTexelLoc, &texel, SHADER_UNIFORM_VEC2);
        SetShaderValue(m.blurShader, m.blurDirLoc, &dirV, SHADER_UNIFORM_VEC2);
        DrawTextureRec(m.blurRT.texture, (Rectangle){0, 0, (float)m.blurRT.texture.width, (float)-m.blurRT.texture.height}, (Vector2){0, 0}, WHITE);
    EndShaderMode();
}

void UnloadMenu3D(Menu3D &m)
{
    if (!m.ready) return;
    UnloadLevel(m.level);
    UnloadShader(m.lightShader);
    UnloadShader(m.blurShader);
    UnloadRenderTexture(m.sceneRT);
    UnloadRenderTexture(m.blurRT);
    UnloadTexture(m.floorTex);
    UnloadTexture(m.planksTex);
    UnloadTexture(m.brickTex);
    UnloadTexture(m.whiteTex);
    UnloadTexture(m.zombiIdle);
    UnloadTexture(m.zombiWalk1);
    UnloadTexture(m.zombiWalk2);
    m.ready = false;
}