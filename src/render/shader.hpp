#pragma once

// Declares shader data and uniform locations

#include "raylib.h"

namespace bvhview
{
inline constexpr int AO_CAPSULES_MAX = 32;
inline constexpr int SHADOW_CAPSULES_MAX = 64;

struct ShaderUniforms
{
    int cameraPosition;
    int exposure;
    int sunDir;
    int sunStrength;
    int sunColor;
    int skyStrength;
    int skyColor;
    int ambientStrength;
    int groundStrength;
    int objectSpecularity;
    int objectGlossiness;
    int objectOpacity;
    int objectColor;
    int isCapsule;
    int capsulePosition;
    int capsuleRotation;
    int capsuleHalfLength;
    int capsuleRadius;
    int capsuleStart;
    int capsuleVector;
    int aoCapsuleCount;
    int aoCapsuleStarts;
    int aoCapsuleVectors;
    int aoCapsuleRadii;
    int aoLookupTable;
    int aoLookupResolution;
    int shadowCapsuleCount;
    int shadowCapsuleStarts;
    int shadowCapsuleVectors;
    int shadowCapsuleRadii;
    int shadowLookupTable;
    int shadowLookupResolution;
};

Shader LoadViewerShader();
void ShaderUniformsInit(ShaderUniforms* uniforms, Shader shader);
void BindViewerLookupTextures(Shader shader, const ShaderUniforms& uniforms, Texture2D aoLookupTable,
                              Texture2D shadowLookupTable);
void UnbindViewerLookupTextures();
}
