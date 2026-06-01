#pragma once

// Declares character collection and BVH loading functions

#include "animation/transform_data.hpp"

#include <array>
#include <string>

namespace bvhview
{
constexpr int CHARACTERS_MAX = 6;

struct CharacterData
{
    int count = 0;
    int active = 0;
    std::array<BVHData, CHARACTERS_MAX> bvhData;
    std::array<float, CHARACTERS_MAX> scales{};
    std::array<std::array<char, 128>, CHARACTERS_MAX> names{};
    std::array<float, CHARACTERS_MAX> autoScales{};
    std::array<Color, CHARACTERS_MAX> colors{};
    std::array<float, CHARACTERS_MAX> opacities{};
    std::array<float, CHARACTERS_MAX> radii{};
    std::array<std::array<char, 512>, CHARACTERS_MAX> filePaths{};
    std::array<TransformData, CHARACTERS_MAX> xformData;
    std::array<TransformData, CHARACTERS_MAX> xformTmp0;
    std::array<TransformData, CHARACTERS_MAX> xformTmp1;
    std::array<TransformData, CHARACTERS_MAX> xformTmp2;
    std::array<TransformData, CHARACTERS_MAX> xformTmp3;
    std::array<std::string, CHARACTERS_MAX> jointNamesCombo;
    bool colorPickerActive = false;
};

void CharacterDataInit(CharacterData* data, int argc, char** argv);
void CharacterDataClear(CharacterData* data);
void CharacterDataFree(CharacterData* data);
bool CharacterDataLoadFromFile(CharacterData* data, const char* path, char* errMsg, int errMsgSize);
}
