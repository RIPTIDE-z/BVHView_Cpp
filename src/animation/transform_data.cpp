// Samples poses and computes global transforms

#include "animation/transform_data.hpp"

#include "core/math_utils.hpp"
#include "raymath.h"

#include <cassert>
#include <cmath>

namespace bvhview
{
void TransformDataInit(TransformData* data)
{
    *data = {};
}

void TransformDataResize(TransformData* data, BVHData* bvh)
{
    data->jointCount = bvh->jointCount;
    data->parents.resize(data->jointCount);
    data->endSite.resize(data->jointCount);
    data->localPositions.resize(data->jointCount);
    data->localRotations.resize(data->jointCount);
    data->globalPositions.resize(data->jointCount);
    data->globalRotations.resize(data->jointCount);
    for (int i = 0; i < data->jointCount; i++)
    {
        data->endSite[i] = bvh->joints[i].endSite;
        data->parents[i] = bvh->joints[i].parent;
    }
}

void TransformDataFree(TransformData* data)
{
    *data = {};
}

void TransformDataSampleFrame(TransformData* data, BVHData* bvh, int frame, float scale)
{
    frame = frame < 0 ? 0 : frame >= bvh->frameCount ? bvh->frameCount - 1 : frame;

    int offset = 0;
    for (int i = 0; i < bvh->jointCount; i++)
    {
        Vector3 position = Vector3Scale(bvh->joints[i].offset, scale);
        Quaternion rotation = QuaternionIdentity();

        for (int c = 0; c < bvh->joints[i].channelCount; c++)
        {
            switch (bvh->joints[i].channels[c])
            {
                case CHANNEL_X_POSITION:
                    position.x = scale * bvh->motionData[frame * bvh->channelCount + offset++];
                    break;
                case CHANNEL_Y_POSITION:
                    position.y = scale * bvh->motionData[frame * bvh->channelCount + offset++];
                    break;
                case CHANNEL_Z_POSITION:
                    position.z = scale * bvh->motionData[frame * bvh->channelCount + offset++];
                    break;
                case CHANNEL_X_ROTATION:
                    rotation = QuaternionMultiply(rotation, QuaternionFromAxisAngle(
                        Vector3{1, 0, 0},
                        DEG2RAD * bvh->motionData[frame * bvh->channelCount + offset++]));
                    break;
                case CHANNEL_Y_ROTATION:
                    rotation = QuaternionMultiply(rotation, QuaternionFromAxisAngle(
                        Vector3{0, 1, 0},
                        DEG2RAD * bvh->motionData[frame * bvh->channelCount + offset++]));
                    break;
                case CHANNEL_Z_ROTATION:
                    rotation = QuaternionMultiply(rotation, QuaternionFromAxisAngle(
                        Vector3{0, 0, 1},
                        DEG2RAD * bvh->motionData[frame * bvh->channelCount + offset++]));
                    break;
            }
        }

        data->localPositions[i] = position;
        data->localRotations[i] = rotation;
    }
    assert(offset == bvh->channelCount);
}

void TransformDataSampleFrameNearest(TransformData* data, BVHData* bvh, float time, float scale)
{
    const int frame = ClampInt(static_cast<int>(time / bvh->frameTime + 0.5f), 0, bvh->frameCount - 1);
    TransformDataSampleFrame(data, bvh, frame, scale);
}

void TransformDataSampleFrameLinear(
    TransformData* data,
    TransformData* tmp0,
    TransformData* tmp1,
    BVHData* bvh,
    float time,
    float scale)
{
    const float alpha = std::fmod(time / bvh->frameTime, 1.0f);
    const int frame0 = ClampInt(static_cast<int>(time / bvh->frameTime), 0, bvh->frameCount - 1);
    const int frame1 = ClampInt(static_cast<int>(time / bvh->frameTime) + 1, 0, bvh->frameCount - 1);
    TransformDataSampleFrame(tmp0, bvh, frame0, scale);
    TransformDataSampleFrame(tmp1, bvh, frame1, scale);

    for (int i = 0; i < data->jointCount; i++)
    {
        data->localPositions[i] = Vector3Lerp(tmp0->localPositions[i], tmp1->localPositions[i], alpha);
        data->localRotations[i] = QuaternionSlerp(tmp0->localRotations[i], tmp1->localRotations[i], alpha);
    }
}

void TransformDataSampleFrameCubic(
    TransformData* data,
    TransformData* tmp0,
    TransformData* tmp1,
    TransformData* tmp2,
    TransformData* tmp3,
    BVHData* bvh,
    float time,
    float scale)
{
    const float alpha = std::fmod(time / bvh->frameTime, 1.0f);
    const int frame0 = ClampInt(static_cast<int>(time / bvh->frameTime) - 1, 0, bvh->frameCount - 1);
    const int frame1 = ClampInt(static_cast<int>(time / bvh->frameTime), 0, bvh->frameCount - 1);
    const int frame2 = ClampInt(static_cast<int>(time / bvh->frameTime) + 1, 0, bvh->frameCount - 1);
    const int frame3 = ClampInt(static_cast<int>(time / bvh->frameTime) + 2, 0, bvh->frameCount - 1);
    TransformDataSampleFrame(tmp0, bvh, frame0, scale);
    TransformDataSampleFrame(tmp1, bvh, frame1, scale);
    TransformDataSampleFrame(tmp2, bvh, frame2, scale);
    TransformDataSampleFrame(tmp3, bvh, frame3, scale);

    for (int i = 0; i < data->jointCount; i++)
    {
        data->localPositions[i] = Vector3InterpolateCubic(
            tmp0->localPositions[i],
            tmp1->localPositions[i],
            tmp2->localPositions[i],
            tmp3->localPositions[i],
            alpha);
        data->localRotations[i] = QuaternionInterpolateCubic(
            tmp0->localRotations[i],
            tmp1->localRotations[i],
            tmp2->localRotations[i],
            tmp3->localRotations[i],
            alpha);
    }
}

void TransformDataForwardKinematics(TransformData* data)
{
    for (int i = 0; i < data->jointCount; i++)
    {
        const int parent = data->parents[i];
        assert(parent <= i);
        if (parent == -1)
        {
            data->globalPositions[i] = data->localPositions[i];
            data->globalRotations[i] = data->localRotations[i];
        }
        else
        {
            data->globalPositions[i] = Vector3Add(
                Vector3RotateByQuaternion(data->localPositions[i], data->globalRotations[parent]),
                data->globalPositions[parent]);
            data->globalRotations[i] = QuaternionMultiply(
                data->globalRotations[parent],
                data->localRotations[i]);
        }
    }
}
}
