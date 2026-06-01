#pragma once

// Declares helper drawing functions

#include "animation/transform_data.hpp"
#include "render/capsule_data.hpp"

namespace bvhview
{
void DrawTransform(Vector3 position, Quaternion rotation, float size);
void DrawSkeleton(TransformData* xformData, bool drawEndSites, Color color, Color endSiteColor);
void DrawTransforms(TransformData* xformData);
void DrawWireFrames(CapsuleData* capsuleData, Color color);
}
