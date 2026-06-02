#pragma once

// Declares capsule caches and lookup tables

#include "animation/character_data.hpp"

#include <vector>

namespace bvhview
{
struct CapsuleSort
{
    int index;
    float value;
};

struct CapsuleData
{
    int capsuleCount = 0;
    std::vector<Vector3> capsulePositions;
    std::vector<Quaternion> capsuleRotations;
    std::vector<float> capsuleRadii;
    std::vector<float> capsuleHalfLengths;
    std::vector<Vector3> capsuleColors;
    std::vector<float> capsuleOpacities;
    std::vector<Vector3> capsuleStarts;
    std::vector<Vector3> capsuleEnds;
    std::vector<Vector3> capsuleVectors;
    std::vector<CapsuleSort> capsuleSort;
    int aoCapsuleCount = 0;
    std::vector<Vector3> aoCapsuleStarts;
    std::vector<Vector3> aoCapsuleVectors;
    std::vector<float> aoCapsuleRadii;
    std::vector<CapsuleSort> aoCapsuleSort;
    int shadowCapsuleCount = 0;
    std::vector<Vector3> shadowCapsuleStarts;
    std::vector<Vector3> shadowCapsuleVectors;
    std::vector<float> shadowCapsuleRadii;
    std::vector<CapsuleSort> shadowCapsuleSort;
    Image aoLookupImage{};
    Texture2D aoLookupTable{};
    Vector2 aoLookupResolution{};
    Image shadowLookupImage{};
    Texture2D shadowLookupTable{};
    Vector2 shadowLookupResolution{};
};

void CapsuleDataInit(CapsuleData* data);
void CapsuleDataResize(CapsuleData* data, int maxCapsuleCount);
void CapsuleDataFree(CapsuleData* data);
void CapsuleDataReset(CapsuleData* data);
void CapsuleDataAppendFromTransformData(
    CapsuleData* data,
    TransformData* xforms,
    float maxCapsuleRadius,
    Color color,
    float opacity,
    bool ignoreEndSite);
void CapsuleDataUpdateAOLookupTable(CapsuleData* data);
void CapsuleDataUpdateShadowLookupTable(CapsuleData* data, float coneAngle);
void CapsuleDataUpdateAOCapsulesForGroundSegment(CapsuleData* data, Vector3 groundSegmentPosition);
void CapsuleDataUpdateAOCapsulesForCapsule(CapsuleData* data, int capsuleIndex);
void CapsuleDataUpdateShadowCapsulesForGroundSegment(
    CapsuleData* data,
    Vector3 groundSegmentPosition,
    Vector3 lightDir,
    float lightConeAngle);
void CapsuleDataUpdateShadowCapsulesForCapsule(
    CapsuleData* data,
    int capsuleIndex,
    Vector3 lightDir,
    float lightConeAngle);
void CapsuleDataUpdateForCharacters(CapsuleData* capsuleData, CharacterData* characterData);
}
