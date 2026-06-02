#pragma once

// Declares capsule occlusion and spatial geometry calculations

#include "raylib.h"

namespace bvhview
{
inline constexpr float AO_RATIO_MAX = 4.0f;

float NearestPointOnLineSegment(Vector3 lineStart, Vector3 lineVector, Vector3 point);
void NearestPointBetweenLineSegments(float* nearestTime0, float* nearestTime1, Vector3 line0Start, Vector3 line0End,
                                     Vector3 line1Start, Vector3 line1End);
float NearestPointBetweenLineSegmentAndPlane(Vector3 lineStart, Vector3 lineVector, Vector3 planePosition,
                                             Vector3 planeNormal);
float NearestPointBetweenLineSegmentAndGroundPlane(Vector3 lineStart, Vector3 lineVector);
void NearestPointBetweenLineSegmentAndGroundSegment(float* nearestTime, Vector3* nearestGroundPoint, Vector3 lineStart,
                                                    Vector3 lineEnd, Vector3 groundStart, Vector3 groundEnd);
Vector3 ProjectPointOntoSweptLine(Vector3 sweptLineStart, Vector3 sweptLineVec, Vector3 sweptLineSweepVec,
                                  Vector3 position);
void NearestPointBetweenLineSegmentAndSweptLine(float* nearestTime, Vector3* nearestPoint, Vector3 lineStart,
                                                Vector3 lineEnd, Vector3 sweptLineStart, Vector3 sweptLineEnd,
                                                Vector3 sweptLineSweepVec);
float SphereOcclusionLookup(float nlAngle, float h);
float SphereOcclusion(Vector3 pos, Vector3 nor, Vector3 sph, float rad);
float SphereIntersectionArea(float r1, float r2, float d);
float SphereDirectionalOcclusionLookup(float phi, float theta, float coneAngle);
float SphereDirectionalOcclusion(Vector3 pos, Vector3 sph, float rad, Vector3 coneDir, float coneAngle);
Vector3 CapsuleStart(Vector3 capsulePosition, Quaternion capsuleRotation, float capsuleHalfLength);
Vector3 CapsuleEnd(Vector3 capsulePosition, Quaternion capsuleRotation, float capsuleHalfLength);
Vector3 CapsuleVector(Vector3 capsulePosition, Quaternion capsuleRotation, float capsuleHalfLength);
float CapsuleDirectionalOcclusion(Vector3 pos, Vector3 capStart, Vector3 capVec, float capRadius, Vector3 coneDir,
                                  float coneAngle);
}
