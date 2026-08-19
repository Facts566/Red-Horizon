#include "postfx.h"
#include <rlgl.h>

#define BLOOM_SCALE 0.5f
#define BLOOM_BLUR  2.0f

void InitPostFX(PostFX &fx)
{
    fx = {};
    int w = GetScreenWidth();
    int h = GetScreenHeight();
    int bw = (int)(w * BLOOM_SCALE);
    int bh = (int)(h * BLOOM_SCALE);

    fx.sceneRT   = LoadRenderTexture(w, h);
    fx.brightRT  = LoadRenderTexture(bw, bh);
    fx.blurRT    = LoadRenderTexture(bw, bh);

    fx.extractShader  = LoadShader(0, "src/shaders/bloom_extract.fs");
    fx.blurShader     = LoadShader(0, "src/shaders/blur.fs");
    fx.compositeShader = LoadShader(0, "src/shaders/composite.fs");

    fx.extractThresholdLoc = GetShaderLocation(fx.extractShader, "bloomThreshold");
    fx.blurTexelLoc = GetShaderLocation(fx.blurShader, "texelSize");
    fx.blurDirLoc   = GetShaderLocation(fx.blurShader, "blurDirection");
    fx.compBloomLoc = GetShaderLocation(fx.compositeShader, "bloomStrength");
    fx.compVignetteLoc = GetShaderLocation(fx.compositeShader, "vignetteStrength");
    fx.compTex1Loc  = GetShaderLocation(fx.compositeShader, "texture1");

    fx.bloomThreshold = 0.6f;
    fx.bloomStrength = 0.0f;
    fx.vignetteStrength = 0.45f;
    fx.ready = true;
}

void BeginPostFX(PostFX &fx)
{
    BeginTextureMode(fx.sceneRT);
    ClearBackground(BLACK);
}

void EndPostFX(PostFX &fx)
{
    EndTextureMode();

    if (fx.bloomStrength > 0.001f) {
        float blurTexel = BLOOM_BLUR / (float)fx.brightRT.texture.width;
        Vector2 texel = {blurTexel, blurTexel};
        Vector2 dirH = {1.0f, 0.0f};
        Vector2 dirV = {0.0f, 1.0f};

        BeginTextureMode(fx.brightRT);
            ClearBackground(BLACK);
            BeginShaderMode(fx.extractShader);
                SetShaderValue(fx.extractShader, fx.extractThresholdLoc, &fx.bloomThreshold, SHADER_UNIFORM_FLOAT);
                DrawTexturePro(fx.sceneRT.texture,
                    (Rectangle){0, 0, (float)fx.sceneRT.texture.width, (float)fx.sceneRT.texture.height},
                    (Rectangle){0, 0, (float)fx.brightRT.texture.width, (float)fx.brightRT.texture.height},
                    (Vector2){0, 0}, 0.0f, WHITE);
            EndShaderMode();
        EndTextureMode();

        BeginTextureMode(fx.blurRT);
            ClearBackground(BLACK);
            BeginShaderMode(fx.blurShader);
                SetShaderValue(fx.blurShader, fx.blurTexelLoc, &texel, SHADER_UNIFORM_VEC2);
                SetShaderValue(fx.blurShader, fx.blurDirLoc, &dirH, SHADER_UNIFORM_VEC2);
                DrawTextureRec(fx.brightRT.texture, (Rectangle){0, 0, (float)fx.brightRT.texture.width, (float)fx.brightRT.texture.height}, (Vector2){0, 0}, WHITE);
            EndShaderMode();
        EndTextureMode();

        BeginTextureMode(fx.brightRT);
            ClearBackground(BLACK);
            BeginShaderMode(fx.blurShader);
                SetShaderValue(fx.blurShader, fx.blurTexelLoc, &texel, SHADER_UNIFORM_VEC2);
                SetShaderValue(fx.blurShader, fx.blurDirLoc, &dirV, SHADER_UNIFORM_VEC2);
                DrawTextureRec(fx.blurRT.texture, (Rectangle){0, 0, (float)fx.blurRT.texture.width, (float)fx.blurRT.texture.height}, (Vector2){0, 0}, WHITE);
            EndShaderMode();
        EndTextureMode();
    }

    BeginShaderMode(fx.compositeShader);
        SetShaderValue(fx.compositeShader, fx.compBloomLoc, &fx.bloomStrength, SHADER_UNIFORM_FLOAT);
        SetShaderValue(fx.compositeShader, fx.compVignetteLoc, &fx.vignetteStrength, SHADER_UNIFORM_FLOAT);
        SetShaderValueTexture(fx.compositeShader, fx.compTex1Loc, fx.brightRT.texture);
        DrawTextureRec(fx.sceneRT.texture, (Rectangle){0, 0, (float)fx.sceneRT.texture.width, (float)-fx.sceneRT.texture.height}, (Vector2){0, 0}, WHITE);
    EndShaderMode();
}

void UnloadPostFX(PostFX &fx)
{
    if (!fx.ready) return;
    UnloadShader(fx.extractShader);
    UnloadShader(fx.blurShader);
    UnloadShader(fx.compositeShader);
    UnloadRenderTexture(fx.sceneRT);
    UnloadRenderTexture(fx.brightRT);
    UnloadRenderTexture(fx.blurRT);
    fx.ready = false;
}