#pragma once

// Declares orbit camera state

#include "raylib.h"

namespace bvhview
{
struct OrbitCamera
{
    Camera3D cam3d;
    float azimuth;
    float altitude;
    float distance;
    Vector3 offset;
    bool track;
    int trackBone;
};

void OrbitCameraInit(OrbitCamera* camera, int argc, char** argv);
void OrbitCameraUpdate(
    OrbitCamera* camera,
    Vector3 target,
    float azimuthDelta,
    float altitudeDelta,
    float offsetDeltaX,
    float offsetDeltaY,
    float mouseWheel,
    float dt);
}
