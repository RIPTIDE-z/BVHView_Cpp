#pragma once

// Declares pose sampling and forward kinematics functions

#include "bvh/bvh_data.hpp"

#include <vector>

namespace bvhview
{
struct TransformData
{
    int jointCount = 0;
    std::vector<int> parents;
    std::vector<bool> endSite;
    std::vector<Vector3> localPositions;
    std::vector<Quaternion> localRotations;
    std::vector<Vector3> globalPositions;
    std::vector<Quaternion> globalRotations;
};

void TransformDataInit(TransformData* data);
void TransformDataResize(TransformData* data, BVHData* bvh);
void TransformDataFree(TransformData* data);
void TransformDataSampleFrame(TransformData* data, BVHData* bvh, int frame, float scale);
void TransformDataSampleFrameNearest(TransformData* data, BVHData* bvh, float time, float scale);
void TransformDataSampleFrameLinear(
    TransformData* data,
    TransformData* tmp0,
    TransformData* tmp1,
    BVHData* bvh,
    float time,
    float scale);
void TransformDataSampleFrameCubic(
    TransformData* data,
    TransformData* tmp0,
    TransformData* tmp1,
    TransformData* tmp2,
    TransformData* tmp3,
    BVHData* bvh,
    float time,
    float scale);
void TransformDataForwardKinematics(TransformData* data);
}
