#pragma once

// Declares common math and view-frustum functions

#include "raylib.h"

namespace bvhview
{
constexpr Rectangle MakeRectangle(float x, float y, float width, float height)
{
    return Rectangle{x, y, width, height};
}

float Max(float x, float y);
float Min(float x, float y);
float Saturate(float x);
float Square(float x);
int ClampInt(int x, int min, int max);
int MaxInt(int x, int y);
int MinInt(int x, int y);
Quaternion QuaternionBetween(Vector3 p, Vector3 q);
Quaternion QuaternionAbsolute(Quaternion q);
Quaternion QuaternionExp(Vector3 v);
Vector3 QuaternionLog(Quaternion q);
Vector3 QuaternionToScaledAngleAxis(Quaternion q);
Quaternion QuaternionFromScaledAngleAxis(Vector3 v);
Vector3 Vector3Hermite(Vector3 p0, Vector3 p1, Vector3 v0, Vector3 v1, float alpha);
Vector3 Vector3InterpolateCubic(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, float alpha);
Quaternion QuaternionHermite(Quaternion r0, Quaternion r1, Vector3 v0, Vector3 v1, float alpha);
Quaternion QuaternionInterpolateCubic(Quaternion r0, Quaternion r1, Quaternion r2, Quaternion r3, float alpha);

struct Frustum
{
    Vector4 back;
    Vector4 front;
    Vector4 bottom;
    Vector4 top;
    Vector4 right;
    Vector4 left;
};

Vector4 FrustumPlaneNormalize(Vector4 plane);
Frustum FrustumFromCameraMatrices(Matrix projection, Matrix modelview);
float FrustumPlaneDistanceToPoint(Vector4 plane, Vector3 position);
bool FrustumContainsSphere(Frustum frustum, Vector3 position, float radius);
}
