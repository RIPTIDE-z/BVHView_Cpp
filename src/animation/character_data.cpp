// Manages loaded BVH characters

#include "animation/character_data.hpp"

#include "core/args.hpp"
#include "core/math_utils.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace bvhview
{
void CharacterDataInit(CharacterData* data, int argc, char** argv)
{
    *data = {};
    data->colors[0] = ArgColor(argc, argv, "color0", ORANGE);
    data->colors[1] = ArgColor(argc, argv, "color1", Color{38, 134, 157, 255});
    data->colors[2] = ArgColor(argc, argv, "color2", PINK);
    data->colors[3] = ArgColor(argc, argv, "color3", LIME);
    data->colors[4] = ArgColor(argc, argv, "color4", VIOLET);
    data->colors[5] = ArgColor(argc, argv, "color5", MAROON);

    std::srand(1234);
    for (int i = 0; i < CHARACTERS_MAX; i++)
    {
        data->scales[i] = 1.0f;
        data->autoScales[i] = 1.0f;
        data->opacities[i] = ArgFloat(argc, argv, "capsuleOpacity", 1.0f);
        data->radii[i] = ArgFloat(argc, argv, "maxCapsuleRadius", 0.04f);
    }
    data->colorPickerActive = ArgBool(argc, argv, "colorPickerActive", false);
}

void CharacterDataClear(CharacterData* data)
{
    for (int i = 0; i < data->count; i++)
    {
        data->bvhData[i] = {};
        data->xformData[i] = {};
        data->xformTmp0[i] = {};
        data->xformTmp1[i] = {};
        data->xformTmp2[i] = {};
        data->xformTmp3[i] = {};
        data->jointNamesCombo[i].clear();
        data->names[i][0] = '\0';
        data->filePaths[i][0] = '\0';
    }
    data->count = 0;
    data->active = 0;
}

void CharacterDataFree(CharacterData* data)
{
    CharacterDataClear(data);
}

bool CharacterDataLoadFromFile(CharacterData* data, const char* path, char* errMsg, int errMsgSize)
{
    std::printf("INFO: Loading '%s'\n", path);
    if (data->count == CHARACTERS_MAX)
    {
        std::snprintf(errMsg, errMsgSize, "Error: Maximum number of BVH files loaded (%i)", CHARACTERS_MAX);
        return false;
    }

    const int index = data->count;
    if (!BVHDataLoad(&data->bvhData[index], path, errMsg, errMsgSize))
    {
        std::printf("INFO: Failed to Load '%s'\n", path);
        return false;
    }

    TransformDataResize(&data->xformData[index], &data->bvhData[index]);
    TransformDataResize(&data->xformTmp0[index], &data->bvhData[index]);
    TransformDataResize(&data->xformTmp1[index], &data->bvhData[index]);
    TransformDataResize(&data->xformTmp2[index], &data->bvhData[index]);
    TransformDataResize(&data->xformTmp3[index], &data->bvhData[index]);
    std::snprintf(data->filePaths[index].data(), data->filePaths[index].size(), "%s", path);

    const char* filename = path;
    while (std::strchr(filename, '/')) { filename = std::strchr(filename, '/') + 1; }
    while (std::strchr(filename, '\\')) { filename = std::strchr(filename, '\\') + 1; }
    std::snprintf(data->names[index].data(), data->names[index].size(), "%s", filename);
    data->scales[index] = 1.0f;

    if (data->bvhData[index].frameCount > 0)
    {
        TransformDataSampleFrame(&data->xformData[index], &data->bvhData[index], 0, 1.0f);
        TransformDataForwardKinematics(&data->xformData[index]);
        float height = 1e-8f;
        for (int j = 0; j < data->xformData[index].jointCount; j++)
        {
            height = Max(height, data->xformData[index].globalPositions[j].y);
        }
        data->scales[index] = height > 10.0f ? 0.01f : 1.0f;
        data->autoScales[index] = 1.8f / height;
    }
    else
    {
        data->autoScales[index] = 1.0f;
    }

    std::string& combo = data->jointNamesCombo[index];
    combo.clear();
    for (int i = 0; i < data->bvhData[index].jointCount; i++)
    {
        if (i > 0) { combo += ';'; }
        combo += data->bvhData[index].joints[i].name;
    }

    data->count++;
    return true;
}
}
