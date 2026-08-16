#include "menu3d.h"
#include "config.h"
#include "light.h"
#include <rlgl.h>
#include <cmath>
#include <cstdio>

#define MENU_ROOM_W 15
#define MENU_ROOM_H 15

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

    BeginTextureMode(m.sceneRT);
        ClearBackground(BLACK);
        BeginMode3D(m.camera);
            DrawLevel(m.level);
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
            DrawTextureRec(m.sceneRT.texture, (Rectangle){0, 0, (float)m.sceneRT.texture.width, (float)m.sceneRT.texture.height}, (Vector2){0, 0}, WHITE);
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
    m.ready = false;
}