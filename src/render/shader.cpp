// Loads the original shaders and caches uniform locations

#include "render/shader.hpp"

#include "rlgl.h"

#define AO_RATIO_MAX 4.0
#include "render/shader_source.inc"

namespace bvhview
{
Shader LoadViewerShader()
{
    return LoadShaderFromMemory(shaderVS, shaderFS);
}

void ShaderUniformsInit(ShaderUniforms* uniforms, Shader shader)
{
    uniforms->isCapsule = GetShaderLocation(shader, "isCapsule");
    uniforms->capsulePosition = GetShaderLocation(shader, "capsulePosition");
    uniforms->capsuleRotation = GetShaderLocation(shader, "capsuleRotation");
    uniforms->capsuleHalfLength = GetShaderLocation(shader, "capsuleHalfLength");
    uniforms->capsuleRadius = GetShaderLocation(shader, "capsuleRadius");
    uniforms->capsuleStart = GetShaderLocation(shader, "capsuleStart");
    uniforms->capsuleVector = GetShaderLocation(shader, "capsuleVector");
    uniforms->shadowCapsuleCount = GetShaderLocation(shader, "shadowCapsuleCount");
    uniforms->shadowCapsuleStarts = GetShaderLocation(shader, "shadowCapsuleStarts");
    uniforms->shadowCapsuleVectors = GetShaderLocation(shader, "shadowCapsuleVectors");
    uniforms->shadowCapsuleRadii = GetShaderLocation(shader, "shadowCapsuleRadii");
    uniforms->shadowLookupTable = GetShaderLocation(shader, "shadowLookupTable");
    uniforms->shadowLookupResolution = GetShaderLocation(shader, "shadowLookupResolution");
    uniforms->aoCapsuleCount = GetShaderLocation(shader, "aoCapsuleCount");
    uniforms->aoCapsuleStarts = GetShaderLocation(shader, "aoCapsuleStarts");
    uniforms->aoCapsuleVectors = GetShaderLocation(shader, "aoCapsuleVectors");
    uniforms->aoCapsuleRadii = GetShaderLocation(shader, "aoCapsuleRadii");
    uniforms->aoLookupTable = GetShaderLocation(shader, "aoLookupTable");
    uniforms->aoLookupResolution = GetShaderLocation(shader, "aoLookupResolution");
    uniforms->cameraPosition = GetShaderLocation(shader, "cameraPosition");
    uniforms->objectColor = GetShaderLocation(shader, "objectColor");
    uniforms->objectSpecularity = GetShaderLocation(shader, "objectSpecularity");
    uniforms->objectGlossiness = GetShaderLocation(shader, "objectGlossiness");
    uniforms->objectOpacity = GetShaderLocation(shader, "objectOpacity");
    uniforms->sunStrength = GetShaderLocation(shader, "sunStrength");
    uniforms->sunDir = GetShaderLocation(shader, "sunDir");
    uniforms->sunColor = GetShaderLocation(shader, "sunColor");
    uniforms->skyStrength = GetShaderLocation(shader, "skyStrength");
    uniforms->skyColor = GetShaderLocation(shader, "skyColor");
    uniforms->ambientStrength = GetShaderLocation(shader, "ambientStrength");
    uniforms->groundStrength = GetShaderLocation(shader, "groundStrength");
    uniforms->exposure = GetShaderLocation(shader, "exposure");
}
void BindViewerLookupTextures(Shader shader, const ShaderUniforms& uniforms, Texture2D aoLookupTable,
                              Texture2D shadowLookupTable)
{
    constexpr int AoTextureSlot = 1;
    constexpr int ShadowTextureSlot = 2;

    SetShaderValue(shader, uniforms.aoLookupTable, &AoTextureSlot, SHADER_UNIFORM_INT);
    SetShaderValue(shader, uniforms.shadowLookupTable, &ShadowTextureSlot, SHADER_UNIFORM_INT);

    rlActiveTextureSlot(AoTextureSlot);
    rlEnableTexture(aoLookupTable.id);
    rlActiveTextureSlot(ShadowTextureSlot);
    rlEnableTexture(shadowLookupTable.id);
    rlActiveTextureSlot(0);
}

void UnbindViewerLookupTextures()
{
    rlActiveTextureSlot(1);
    rlDisableTexture();
    rlActiveTextureSlot(2);
    rlDisableTexture();
    rlActiveTextureSlot(0);
}
}
