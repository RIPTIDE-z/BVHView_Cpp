// Draws skeleton transforms and capsule wireframes

#include "render/drawing.hpp"
#include "render/capsule_data.hpp"
#include "render/geometry.hpp"
#include "animation/transform_data.hpp"
#include "raymath.h"

namespace bvhview
{
namespace
{
constexpr float OriginAxisLength = 1.0f;
constexpr float OriginAxisRadius = 0.008f;
constexpr int OriginAxisSides = 8;
}

void DrawTransform(const Vector3 position, const Quaternion rotation, const float size)
{
    DrawLine3D(position, Vector3Add(position, Vector3RotateByQuaternion(Vector3{ size, 0.0, 0.0 }, rotation)), RED);
    DrawLine3D(position, Vector3Add(position, Vector3RotateByQuaternion(Vector3{ 0.0, size, 0.0 }, rotation)), GREEN);
    DrawLine3D(position, Vector3Add(position, Vector3RotateByQuaternion(Vector3{ 0.0, 0.0, size }, rotation)), BLUE);
}

void DrawSkeleton(TransformData* xformData, bool drawEndSites, Color color, Color endSiteColor)
{
    for (int i = 0; i < xformData->jointCount; i++)
    {
        if (!xformData->endSite[i])
        {
            DrawSphereWires(
                xformData->globalPositions[i],
                0.01f,
                4,
                6,
                color);
        }
        else if (drawEndSites)
        {
            DrawCubeWiresV(
                xformData->globalPositions[i],
                Vector3{ 0.02f, 0.02f, 0.02f },
                endSiteColor);
        }

        if (xformData->parents[i] != -1)
        {
            if (!xformData->endSite[i])
            {
                DrawLine3D(
                    xformData->globalPositions[i],
                    xformData->globalPositions[xformData->parents[i]],
                    color);
            }
            else if (drawEndSites)
            {
                DrawLine3D(
                    xformData->globalPositions[i],
                    xformData->globalPositions[xformData->parents[i]],
                    endSiteColor);
            }
        }
    }
}

void DrawTransforms(TransformData* xformData)
{
    for (int i = 0; i < xformData->jointCount; i++)
    {
        if (!xformData->endSite[i])
        {
            DrawTransform(
                xformData->globalPositions[i],
                xformData->globalRotations[i],
                0.1f);
        }
    }
}

void DrawWireFrames(CapsuleData* capsuleData, Color color)
{
    for (int i = 0; i < capsuleData->capsuleCount; i++)
    {
        const Vector3 capsuleStart = capsuleData->capsuleStarts[i];
        const Vector3 capsuleEnd = capsuleData->capsuleEnds[i];
        float capsuleRadius = capsuleData->capsuleRadii[i];

        DrawSphereWires(capsuleStart, capsuleRadius, 4, 6, color);
        DrawSphereWires(capsuleEnd, capsuleRadius, 4, 6, color);
        DrawCylinderWiresEx(capsuleStart, capsuleEnd, capsuleRadius, capsuleRadius, 6, color);
    }
}

void DrawViewerGrid()
{
    DrawGrid(20, 1.0f);
}

void DrawViewerOrigin()
{
    constexpr Vector3 Origin = Vector3{0.0f, 0.01f, 0.0f};

    DrawCylinderEx(Origin, Vector3{OriginAxisLength, Origin.y, 0.0f}, OriginAxisRadius, OriginAxisRadius, OriginAxisSides, RED);
    DrawCylinderEx(Origin, Vector3{0.0f, Origin.y + OriginAxisLength, 0.0f}, OriginAxisRadius, OriginAxisRadius, OriginAxisSides, GREEN);
    DrawCylinderEx(Origin, Vector3{0.0f, Origin.y, OriginAxisLength}, OriginAxisRadius, OriginAxisRadius, OriginAxisSides, BLUE);
}
}
