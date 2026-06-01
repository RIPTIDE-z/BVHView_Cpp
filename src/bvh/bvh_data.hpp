#pragma once

// Declares BVH data structures and loading functions

#include "raylib.h"

#include <array>
#include <string>
#include <vector>

namespace bvhview
{
enum Channel
{
    CHANNEL_X_POSITION = 0,
    CHANNEL_Y_POSITION = 1,
    CHANNEL_Z_POSITION = 2,
    CHANNEL_X_ROTATION = 3,
    CHANNEL_Y_ROTATION = 4,
    CHANNEL_Z_ROTATION = 5,
    CHANNELS_MAX = 6,
};

struct BVHJointData
{
    int parent = -1;
    std::string name;
    Vector3 offset{};
    int channelCount = 0;
    std::array<char, CHANNELS_MAX> channels{};
    bool endSite = false;
};

struct BVHData
{
    int jointCount = 0;
    std::vector<BVHJointData> joints;
    int frameCount = 0;
    int channelCount = 0;
    float frameTime = 0.0f;
    std::vector<float> motionData;
};

void BVHDataInit(BVHData* bvh);
void BVHDataFree(BVHData* bvh);
bool BVHDataLoad(BVHData* bvh, const char* filename, char* errMsg, int errMsgSize);
}
