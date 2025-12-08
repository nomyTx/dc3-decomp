#pragma once
#include "types.h"
#include "utl/Str.h"
#include <vector>

enum ShaderType {
    kBloomShader = 0,
    kBlurShader = 1,
    kDepthVolumeShader = 2,
    kDownsampleShader = 3,
    kDownsample4xShader = 4,
    kDownsampleDepthShader = 5,
    kDrawRectShader = 6,
    kErrorShader = 7,
    kFurShader = 8,
    kLineNozShader = 9,
    kLineShader = 10,
    kMovieShader = 11,
    kMultimeshShader = 12,
    kMultimeshBBShader = 13,
    kParticlesShader = 14,
    kPostprocessErrorShader = 15,
    kPostprocessShader = 16,
    kShadowmapShader = 17,
    kStandardShader = 18,
    kStandardBBShader = 19,
    kSyncTrackShader = 20,
    kSyncTrackChargeEffectShader = 21,
    kUnwrapUVShader = 22,
    kVelocityCameraShader = 23,
    kVelocityObjectShader = 24,
    kPlayerDepthVisShader = 25,
    kPlayerDepthShellShader = 26,
    kBloomGlareShader = 27,
    kPlayerDepthShell2Shader = 28,
    kDepthBuffer3DShader = 29,
    kYUVtoRGBShader = 30,
    kYUVtoBlackAndWhiteShader = 31,
    kPlayerGreenScreenShader = 32,
    kPlayerDepthGreenScreenShader = 33,
    kCrewPhotoShader = 34,
    kTwirlShader = 35,
    kKillAlphaShader = 36,
    kAllWhiteShader = 37,
    kMaxShaderTypes = 38
};

struct ShaderMacro {
    ShaderMacro(const char *c1 = nullptr, const char *c2 = nullptr)
        : unk0(c1), unk4(c2) {}
    const char *unk0; // 0x0 - name
    const char *unk4; // 0x4 - level/depth?
};

struct ShaderOptions {
    ShaderOptions(u64 u) : unk(u) {}

    void GenerateMacros(ShaderType, std::vector<ShaderMacro> &) const;

    u64 unk;
};

void InitShaderOptions();
const char *ShaderTypeName(ShaderType);
ShaderType ShaderTypeFromName(const char *);
const char *ShaderSourcePath(const char *);
const char *ShaderCachedPath(const char *, u64, bool);
bool IsPostProcShaderType(ShaderType);
void ShaderMakeOptionsString(ShaderType, const ShaderOptions &, String &);
