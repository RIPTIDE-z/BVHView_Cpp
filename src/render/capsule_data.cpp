// Manages capsule caches and occlusion lookup tables

#include "render/capsule_data.hpp"
#include "core/math_utils.hpp"
#include "raymath.h"
#include "render/geometry.hpp"

#include <algorithm>
#include <cstdlib>

namespace bvhview
{
namespace
{
bool DistanceGreaterThan(Vector3 lhs, Vector3 rhs, float distance)
{
    return Vector3DistanceSqr(lhs, rhs) > distance * distance;
}

bool DistanceLessThan(Vector3 lhs, Vector3 rhs, float distance)
{
    return Vector3DistanceSqr(lhs, rhs) < distance * distance;
}

bool DistanceLessThanOrEqual(Vector3 lhs, Vector3 rhs, float distance)
{
    return Vector3DistanceSqr(lhs, rhs) <= distance * distance;
}

void SortCapsulesGreater(std::vector<CapsuleSort>* capsules, int count)
{
    std::sort(capsules->begin(), capsules->begin() + count,
              [](const CapsuleSort& lhs, const CapsuleSort& rhs) { return lhs.value > rhs.value; });
}
}
void CapsuleDataInit(CapsuleData* data)
{
    *data = {};

    data->aoLookupImage.data = std::calloc(32 * 32, 1);
    data->aoLookupImage.width = 32;
    data->aoLookupImage.height = 32;
    data->aoLookupImage.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
    data->aoLookupImage.mipmaps = 1;
    data->aoLookupTable = LoadTextureFromImage(data->aoLookupImage);
    data->aoLookupResolution =
        Vector2{static_cast<float>(data->aoLookupImage.width), static_cast<float>(data->aoLookupImage.height)};
    SetTextureWrap(data->aoLookupTable, TEXTURE_WRAP_CLAMP);
    SetTextureFilter(data->aoLookupTable, TEXTURE_FILTER_BILINEAR);
    CapsuleDataUpdateAOLookupTable(data);

    data->shadowLookupImage.data = std::calloc(256 * 128, 1);
    data->shadowLookupImage.width = 256;
    data->shadowLookupImage.height = 128;
    data->shadowLookupImage.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
    data->shadowLookupImage.mipmaps = 1;
    data->shadowLookupTable = LoadTextureFromImage(data->shadowLookupImage);
    data->shadowLookupResolution =
        Vector2{static_cast<float>(data->shadowLookupImage.width), static_cast<float>(data->shadowLookupImage.height)};
    SetTextureWrap(data->shadowLookupTable, TEXTURE_WRAP_CLAMP);
    SetTextureFilter(data->shadowLookupTable, TEXTURE_FILTER_BILINEAR);
    CapsuleDataUpdateShadowLookupTable(data, 0.2f);
}

void CapsuleDataResize(CapsuleData* data, int maxCapsuleCount)
{
    data->capsulePositions.resize(maxCapsuleCount);
    data->capsuleRotations.resize(maxCapsuleCount);
    data->capsuleRadii.resize(maxCapsuleCount);
    data->capsuleHalfLengths.resize(maxCapsuleCount);
    data->capsuleColors.resize(maxCapsuleCount);
    data->capsuleOpacities.resize(maxCapsuleCount);
    data->capsuleStarts.resize(maxCapsuleCount);
    data->capsuleEnds.resize(maxCapsuleCount);
    data->capsuleVectors.resize(maxCapsuleCount);
    data->capsuleSort.resize(maxCapsuleCount);
    data->aoCapsuleStarts.resize(maxCapsuleCount);
    data->aoCapsuleVectors.resize(maxCapsuleCount);
    data->aoCapsuleRadii.resize(maxCapsuleCount);
    data->aoCapsuleSort.resize(maxCapsuleCount);
    data->shadowCapsuleStarts.resize(maxCapsuleCount);
    data->shadowCapsuleVectors.resize(maxCapsuleCount);
    data->shadowCapsuleRadii.resize(maxCapsuleCount);
    data->shadowCapsuleSort.resize(maxCapsuleCount);
}

void CapsuleDataFree(CapsuleData* data)
{
    UnloadImage(data->aoLookupImage);
    UnloadTexture(data->aoLookupTable);
    UnloadImage(data->shadowLookupImage);
    UnloadTexture(data->shadowLookupTable);
    *data = {};
}

void CapsuleDataUpdateAOLookupTable(CapsuleData* data)
{
    int width = (int)data->aoLookupResolution.x;
    int height = (int)data->aoLookupResolution.y;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float nlAngle = (((float)x) / (width - 1)) * PI;
            float h = 1.0f + (AO_RATIO_MAX - 1.0f) * (((float)y) / (height - 1));
            ((unsigned char*)data->aoLookupImage.data)[y * width + x] =
                (unsigned char)Clamp(255.0 * SphereOcclusionLookup(nlAngle, h), 0.0, 255.0);
        }
    }

    UpdateTexture(data->aoLookupTable, data->aoLookupImage.data);
}

void CapsuleDataUpdateShadowLookupTable(CapsuleData* data, float coneAngle)
{
    int width = (int)data->shadowLookupResolution.x;
    int height = (int)data->shadowLookupResolution.y;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float phi = (((float)x) / (width - 1)) * PI;
            float theta = ((float)y) / (height - 1) * (PI / 2.0f);
            ((unsigned char*)data->shadowLookupImage.data)[y * width + x] =
                (unsigned char)Clamp(255.0 * SphereDirectionalOcclusionLookup(phi, theta, coneAngle), 0.0, 255.0);
        }
    }

    UpdateTexture(data->shadowLookupTable, data->shadowLookupImage.data);
}

void CapsuleDataReset(CapsuleData* data)
{
    data->capsuleCount = 0;
    data->aoCapsuleCount = 0;
    data->shadowCapsuleCount = 0;
}

void CapsuleDataAppendFromTransformData(CapsuleData* data, TransformData* xforms, float maxCapsuleRadius, Color color,
                                        float opacity, bool ignoreEndSite)
{
    for (int i = 0; i < xforms->jointCount; i++)
    {
        int p = xforms->parents[i];

        if (p == -1)
        {
            continue;
        }
        if (ignoreEndSite && xforms->endSite[i])
        {
            continue;
        }

        float capsuleHalfLength = Vector3Length(xforms->localPositions[i]) / 2.0f;
        float capsuleRadius = Min(maxCapsuleRadius, capsuleHalfLength) + (i % 2) * 0.001f;

        if (capsuleRadius < 0.001f)
        {
            continue;
        }

        Vector3 capsulePosition =
            Vector3Scale(Vector3Add(xforms->globalPositions[i], xforms->globalPositions[p]), 0.5f);
        Quaternion capsuleRotation = QuaternionMultiply(
            xforms->globalRotations[p],
            QuaternionBetween(Vector3{1.0f, 0.0f, 0.0f}, Vector3Normalize(xforms->localPositions[i])));

        data->capsulePositions[data->capsuleCount] = capsulePosition;
        data->capsuleRotations[data->capsuleCount] = capsuleRotation;
        data->capsuleHalfLengths[data->capsuleCount] = capsuleHalfLength;
        data->capsuleRadii[data->capsuleCount] = capsuleRadius;
        data->capsuleColors[data->capsuleCount] = Vector3{color.r / 255.0f, color.g / 255.0f, color.b / 255.0f};
        data->capsuleOpacities[data->capsuleCount] = opacity;
        data->capsuleStarts[data->capsuleCount] = CapsuleStart(capsulePosition, capsuleRotation, capsuleHalfLength);
        data->capsuleEnds[data->capsuleCount] = CapsuleEnd(capsulePosition, capsuleRotation, capsuleHalfLength);
        data->capsuleVectors[data->capsuleCount] = CapsuleVector(capsulePosition, capsuleRotation, capsuleHalfLength);
        data->capsuleCount++;
    }
}

void CapsuleDataUpdateAOCapsulesForGroundSegment(CapsuleData* data, Vector3 groundSegmentPosition)
{
    data->aoCapsuleCount = 0;

    for (int i = 0; i < data->capsuleCount; i++)
    {
        Vector3 capsulePosition = data->capsulePositions[i];
        float capsuleHalfLength = data->capsuleHalfLengths[i];
        float capsuleRadius = data->capsuleRadii[i];

        if (DistanceGreaterThan(groundSegmentPosition, capsulePosition,
                                sqrtf(2.0f) + capsuleHalfLength + AO_RATIO_MAX * capsuleRadius))
        {
            continue;
        }

        const Vector3 capsuleStart = data->capsuleStarts[i];
        const Vector3 capsuleEnd = data->capsuleEnds[i];
        const Vector3 capsuleVector = data->capsuleVectors[i];

        float capsuleTime;
        Vector3 groundPoint;
        NearestPointBetweenLineSegmentAndGroundSegment(
            &capsuleTime, &groundPoint, capsuleStart, capsuleEnd,
            Vector3{groundSegmentPosition.x - 1.0f, 0.0f, groundSegmentPosition.z - 1.0f},
            Vector3{groundSegmentPosition.x + 1.0f, 0.0f, groundSegmentPosition.z + 1.0f});

        Vector3 capsulePoint = Vector3Add(capsuleStart, Vector3Scale(capsuleVector, capsuleTime));

        if (DistanceGreaterThan(groundPoint, capsulePoint, AO_RATIO_MAX * capsuleRadius))
        {
            continue;
        }

        float capsuleOcclusion =
            DistanceLessThan(groundPoint, capsulePoint, capsuleRadius)
                ? 0.0f
                : SphereOcclusion(groundPoint, Vector3{0.0f, 1.0f, 0.0f}, capsulePoint, capsuleRadius);

        if (capsuleOcclusion < 0.99f)
        {
            data->aoCapsuleSort[data->aoCapsuleCount] = CapsuleSort{i, capsuleOcclusion};
            data->aoCapsuleCount++;
        }
    }

    SortCapsulesGreater(&data->aoCapsuleSort, data->aoCapsuleCount);

    for (int i = 0; i < data->aoCapsuleCount; i++)
    {
        int j = data->aoCapsuleSort[i].index;
        data->aoCapsuleStarts[i] = data->capsuleStarts[j];
        data->aoCapsuleVectors[i] = data->capsuleVectors[j];
        data->aoCapsuleRadii[i] = data->capsuleRadii[j];
    }
}

void CapsuleDataUpdateAOCapsulesForCapsule(CapsuleData* data, int capsuleIndex)
{
    Vector3 queryCapsulePosition = data->capsulePositions[capsuleIndex];
    float queryCapsuleHalfLength = data->capsuleHalfLengths[capsuleIndex];
    float queryCapsuleRadius = data->capsuleRadii[capsuleIndex];
    const Vector3 queryCapsuleStart = data->capsuleStarts[capsuleIndex];
    const Vector3 queryCapsuleEnd = data->capsuleEnds[capsuleIndex];
    const Vector3 queryCapsuleVector = data->capsuleVectors[capsuleIndex];

    data->aoCapsuleCount = 0;

    for (int i = 0; i < data->capsuleCount; i++)
    {
        if (i == capsuleIndex)
        {
            continue;
        }

        Vector3 capsulePosition = data->capsulePositions[i];
        float capsuleRadius = data->capsuleRadii[i];
        float capsuleHalfLength = data->capsuleHalfLengths[i];

        if (DistanceGreaterThan(queryCapsulePosition, capsulePosition,
                                queryCapsuleHalfLength + queryCapsuleRadius + capsuleHalfLength +
                                    AO_RATIO_MAX * capsuleRadius))
        {
            continue;
        }

        const Vector3 capsuleStart = data->capsuleStarts[i];
        const Vector3 capsuleEnd = data->capsuleEnds[i];
        const Vector3 capsuleVector = data->capsuleVectors[i];

        float capsuleTime, queryTime;
        NearestPointBetweenLineSegments(&capsuleTime, &queryTime, capsuleStart, capsuleEnd, queryCapsuleStart,
                                        queryCapsuleEnd);

        Vector3 capsulePoint = Vector3Add(capsuleStart, Vector3Scale(capsuleVector, capsuleTime));
        Vector3 queryPoint = Vector3Add(queryCapsuleStart, Vector3Scale(queryCapsuleVector, queryTime));

        if (DistanceGreaterThan(queryPoint, capsulePoint, queryCapsuleRadius + AO_RATIO_MAX * capsuleRadius))
        {
            continue;
        }

        Vector3 surfaceNormal = Vector3Normalize(Vector3Subtract(capsulePoint, queryPoint));
        Vector3 surfacePoint = Vector3Add(queryPoint, Vector3Scale(surfaceNormal, queryCapsuleRadius));
        float capsuleOcclusion = DistanceLessThanOrEqual(queryPoint, capsulePoint, queryCapsuleRadius + capsuleRadius)
                                     ? 0.0f
                                     : SphereOcclusion(surfacePoint, surfaceNormal, capsulePoint, capsuleRadius);

        if (capsuleOcclusion < 0.99f)
        {
            data->aoCapsuleSort[data->aoCapsuleCount] = CapsuleSort{i, capsuleOcclusion};
            data->aoCapsuleCount++;
        }
    }

    SortCapsulesGreater(&data->aoCapsuleSort, data->aoCapsuleCount);

    for (int i = 0; i < data->aoCapsuleCount; i++)
    {
        int j = data->aoCapsuleSort[i].index;
        data->aoCapsuleStarts[i] = data->capsuleStarts[j];
        data->aoCapsuleVectors[i] = data->capsuleVectors[j];
        data->aoCapsuleRadii[i] = data->capsuleRadii[j];
    }
}

void CapsuleDataUpdateShadowCapsulesForGroundSegment(CapsuleData* data, Vector3 groundSegmentPosition, Vector3 lightDir,
                                                     float lightConeAngle)
{
    Vector3 lightRay = Vector3Scale(lightDir, 10.0f);

    data->shadowCapsuleCount = 0;

    for (int i = 0; i < data->capsuleCount; i++)
    {
        Vector3 capsulePosition = data->capsulePositions[i];
        float capsuleHalfLength = data->capsuleHalfLengths[i];
        float capsuleRadius = data->capsuleRadii[i];

        float midRayTime = NearestPointBetweenLineSegmentAndGroundPlane(capsulePosition, lightRay);
        Vector3 groundCapsuleMid = Vector3Add(capsulePosition, Vector3Scale(lightRay, midRayTime));
        float maxRatio = 4.0f;

        if (DistanceGreaterThan(groundSegmentPosition, groundCapsuleMid,
                                sqrtf(2.0f) + capsuleHalfLength + maxRatio * capsuleRadius))
        {
            continue;
        }

        const Vector3 capsuleStart = data->capsuleStarts[i];
        const Vector3 capsuleEnd = data->capsuleEnds[i];
        const Vector3 capsuleVector = data->capsuleVectors[i];

        float startRayTime = NearestPointBetweenLineSegmentAndGroundPlane(capsuleStart, lightRay);
        float endRayTime = NearestPointBetweenLineSegmentAndGroundPlane(capsuleEnd, lightRay);

        Vector3 groundCapsuleStart = Vector3Add(capsuleStart, Vector3Scale(lightRay, startRayTime));
        Vector3 groundCapsuleEnd = Vector3Add(capsuleEnd, Vector3Scale(lightRay, endRayTime));

        groundCapsuleStart.x =
            Clamp(groundCapsuleStart.x, groundSegmentPosition.x - 1.0f, groundSegmentPosition.x + 1.0f);
        groundCapsuleStart.z =
            Clamp(groundCapsuleStart.z, groundSegmentPosition.z - 1.0f, groundSegmentPosition.z + 1.0f);
        groundCapsuleEnd.x = Clamp(groundCapsuleEnd.x, groundSegmentPosition.x - 1.0f, groundSegmentPosition.x + 1.0f);
        groundCapsuleEnd.z = Clamp(groundCapsuleEnd.z, groundSegmentPosition.z - 1.0f, groundSegmentPosition.z + 1.0f);

        if (DistanceGreaterThan(groundSegmentPosition, groundCapsuleStart, sqrtf(2.0f) + maxRatio * capsuleRadius) &&
            DistanceGreaterThan(groundSegmentPosition, groundCapsuleEnd, sqrtf(2.0f) + maxRatio * capsuleRadius))
        {
            continue;
        }

        float capsuleOcclusion = Min(CapsuleDirectionalOcclusion(groundCapsuleStart, capsuleStart, capsuleVector,
                                                                 capsuleRadius, lightDir, lightConeAngle),
                                     CapsuleDirectionalOcclusion(groundCapsuleEnd, capsuleStart, capsuleVector,
                                                                 capsuleRadius, lightDir, lightConeAngle));

        if (capsuleOcclusion < 0.99f)
        {
            data->shadowCapsuleSort[data->shadowCapsuleCount] = CapsuleSort{i, capsuleOcclusion};
            data->shadowCapsuleCount++;
        }
    }

    SortCapsulesGreater(&data->shadowCapsuleSort, data->shadowCapsuleCount);

    for (int i = 0; i < data->shadowCapsuleCount; i++)
    {
        int j = data->shadowCapsuleSort[i].index;
        data->shadowCapsuleStarts[i] = data->capsuleStarts[j];
        data->shadowCapsuleVectors[i] = data->capsuleVectors[j];
        data->shadowCapsuleRadii[i] = data->capsuleRadii[j];
    }
}

void CapsuleDataUpdateShadowCapsulesForCapsule(CapsuleData* data, int capsuleIndex, Vector3 lightDir,
                                               float lightConeAngle)
{
    Vector3 lightRay = Vector3Scale(lightDir, 10.0f);

    Vector3 queryCapsulePosition = data->capsulePositions[capsuleIndex];
    float queryCapsuleHalfLength = data->capsuleHalfLengths[capsuleIndex];
    float queryCapsuleRadius = data->capsuleRadii[capsuleIndex];
    const Vector3 queryCapsuleStart = data->capsuleStarts[capsuleIndex];
    const Vector3 queryCapsuleEnd = data->capsuleEnds[capsuleIndex];
    const Vector3 queryCapsuleVector = data->capsuleVectors[capsuleIndex];

    data->shadowCapsuleCount = 0;

    for (int i = 0; i < data->capsuleCount; i++)
    {
        if (i == capsuleIndex)
        {
            continue;
        }

        Vector3 capsulePosition = data->capsulePositions[i];
        float capsuleHalfLength = data->capsuleHalfLengths[i];
        float capsuleRadius = data->capsuleRadii[i];

        float midRayTime = NearestPointOnLineSegment(capsulePosition, lightRay, queryCapsulePosition);

        Vector3 capsuleMid = Vector3Add(capsulePosition, Vector3Scale(lightRay, midRayTime));
        float maxRatio = 4.0f;

        if (DistanceGreaterThan(queryCapsulePosition, capsuleMid,
                                queryCapsuleHalfLength + queryCapsuleRadius + capsuleHalfLength +
                                    maxRatio * capsuleRadius))
        {
            continue;
        }

        const Vector3 capsuleStart = data->capsuleStarts[i];
        const Vector3 capsuleEnd = data->capsuleEnds[i];
        const Vector3 capsuleVector = data->capsuleVectors[i];

        float queryCapsuleTime;
        Vector3 nearestRayPoint;
        NearestPointBetweenLineSegmentAndSweptLine(&queryCapsuleTime, &nearestRayPoint, queryCapsuleStart,
                                                   queryCapsuleEnd, capsuleStart, capsuleEnd, lightRay);

        Vector3 queryCapsulePoint = Vector3Add(queryCapsuleStart, Vector3Scale(queryCapsuleVector, queryCapsuleTime));

        if (DistanceGreaterThan(queryCapsulePoint, nearestRayPoint,
                                queryCapsuleRadius + capsuleHalfLength + maxRatio * capsuleRadius))
        {
            continue;
        }

        Vector3 surfaceNormal = Vector3Normalize(Vector3Subtract(nearestRayPoint, queryCapsulePoint));
        Vector3 surfacePoint = Vector3Add(queryCapsulePoint, Vector3Scale(surfaceNormal, queryCapsuleRadius));

        float capsuleOcclusion =
            DistanceLessThanOrEqual(queryCapsulePoint, nearestRayPoint, queryCapsuleRadius + capsuleRadius)
                ? 0.0f
                : CapsuleDirectionalOcclusion(surfacePoint, capsuleStart, capsuleVector, capsuleRadius, lightDir,
                                              lightConeAngle);

        if (capsuleOcclusion < 0.99f)
        {
            data->shadowCapsuleSort[data->shadowCapsuleCount] = CapsuleSort{i, capsuleOcclusion};
            data->shadowCapsuleCount++;
        }
    }

    SortCapsulesGreater(&data->shadowCapsuleSort, data->shadowCapsuleCount);

    for (int i = 0; i < data->shadowCapsuleCount; i++)
    {
        int j = data->shadowCapsuleSort[i].index;
        data->shadowCapsuleStarts[i] = data->capsuleStarts[j];
        data->shadowCapsuleVectors[i] = data->capsuleVectors[j];
        data->shadowCapsuleRadii[i] = data->capsuleRadii[j];
    }
}

void CapsuleDataUpdateForCharacters(CapsuleData* capsuleData, CharacterData* characterData)
{
    int totalJointCount = 0;
    for (int i = 0; i < characterData->count; i++)
    {
        totalJointCount += characterData->bvhData[i].jointCount;
    }

    CapsuleDataResize(capsuleData, totalJointCount);
}
}
