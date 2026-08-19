#pragma once
#include <raylib.h>

struct PostFX {
    RenderTexture2D sceneRT;
    RenderTexture2D brightRT;
    RenderTexture2D blurRT;
    Shader extractShader;
    Shader blurShader;
    Shader compositeShader;
    int extractThresholdLoc;
    int blurTexelLoc;
    int blurDirLoc;
    int compBloomLoc;
    int compVignetteLoc;
    int compTex1Loc;
    float bloomThreshold;
    float bloomStrength;
    float vignetteStrength;
    bool ready;
};

void InitPostFX(PostFX &fx);
void BeginPostFX(PostFX &fx);
void EndPostFX(PostFX &fx);
void UnloadPostFX(PostFX &fx);