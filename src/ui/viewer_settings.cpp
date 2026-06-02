// Manages rendering and timeline settings

#include "ui/viewer_settings.hpp"
#include "animation/character_data.hpp"
#include "core/args.hpp"
#include "core/math_utils.hpp"
#include "raymath.h"
#include "render/capsule_data.hpp"

namespace bvhview
{
void RenderSettingsInit(RenderSettings* settings, int argc, char** argv)
{
    settings->backgroundColor = ArgColor(argc, argv, "backgroundColor", WHITE);

    settings->sunLightConeAngle = ArgFloat(argc, argv, "sunLightConeAngle", 0.2f);
    settings->sunLightStrength = ArgFloat(argc, argv, "sunLightStrength", 0.25f);
    settings->sunAzimuth = ArgFloat(argc, argv, "sunAzimuth", PI / 4.0f);
    settings->sunAltitude = ArgFloat(argc, argv, "sunAltitude", 0.8f);
    settings->sunColor = ArgColor(argc, argv, "sunColor", Color{253, 255, 232});

    settings->skyLightStrength = ArgFloat(argc, argv, "skyLightStrength", 0.15f);
    settings->skyColor = ArgColor(argc, argv, "skyColor", Color{174, 183, 190});

    settings->groundLightStrength = ArgFloat(argc, argv, "groundLightStrength", 0.1f);
    settings->ambientLightStrength = ArgFloat(argc, argv, "ambientLightStrength", 1.0f);

    settings->exposure = ArgFloat(argc, argv, "exposure", 0.9f);

    settings->drawOrigin = ArgBool(argc, argv, "drawOrigin", true);
    settings->drawGrid = ArgBool(argc, argv, "drawGrid", false);
    settings->drawChecker = ArgBool(argc, argv, "drawChecker", true);
    settings->drawCapsules = ArgBool(argc, argv, "drawCapsules", true);
    settings->drawWireframes = ArgBool(argc, argv, "drawWireframes", false);
    settings->drawSkeleton = ArgBool(argc, argv, "drawSkeleton", true);
    settings->drawTransforms = ArgBool(argc, argv, "drawTransforms", false);
    settings->drawAO = ArgBool(argc, argv, "drawAO", true);
    settings->drawShadows = ArgBool(argc, argv, "drawShadows", true);
    settings->drawEndSites = ArgBool(argc, argv, "drawEndSites", true);
    settings->drawFPS = ArgBool(argc, argv, "drawFPS", false);
    settings->drawUI = ArgBool(argc, argv, "drawUI", true);
}

void ScrubberSettingsInit(ScrubberSettings* settings, int argc, char** argv)
{
    static const char* sampleModes[] = {"nearest", "linear", "cubic"};
    settings->playing = ArgBool(argc, argv, "playing", true);
    settings->looping = ArgBool(argc, argv, "looping", false);
    settings->inplace = ArgBool(argc, argv, "inplace", false);
    settings->playTime = ArgFloat(argc, argv, "playTime", 0.0f);
    settings->playSpeed = ArgFloat(argc, argv, "playSpeed", 1.0f);
    settings->frameSnap = ArgBool(argc, argv, "frameSnap", true);
    settings->sampleMode = ArgEnum(argc, argv, "sampleMode", 3, sampleModes, 1);

    settings->timeLimit = 0.0f;
    settings->frameLimit = 0;
    settings->frameMin = 0;
    settings->frameMax = 0;
    settings->frameMinSelect = 0;
    settings->frameMaxSelect = 0;
    settings->frameMinEdit = false;
    settings->frameMaxEdit = false;
    settings->timeMin = 0.0f;
    settings->timeMax = 0.0f;
}

void ScrubberSettingsRecomputeLimits(ScrubberSettings* settings, CharacterData* characterData)
{
    settings->frameLimit = 0;
    settings->timeLimit = 0.0f;
    for (int i = 0; i < characterData->count; i++)
    {
        settings->frameLimit = MaxInt(settings->frameLimit, characterData->bvhData[i].frameCount - 1);
        settings->timeLimit =
            Max(settings->timeLimit, (characterData->bvhData[i].frameCount - 1) * characterData->bvhData[i].frameTime);
    }
}

void ScrubberSettingsInitMaxs(ScrubberSettings* settings, CharacterData* characterData)
{
    if (characterData->count == 0)
    {
        return;
    }

    settings->frameMax = characterData->bvhData[characterData->active].frameCount - 1;
    settings->frameMaxSelect = settings->frameMax;
    settings->timeMax = settings->frameMax * characterData->bvhData[characterData->active].frameTime;

    settings->frameMin = 0;
    settings->frameMinSelect = settings->frameMin;
    settings->timeMin = 0.0f;
}

void ScrubberSettingsClamp(ScrubberSettings* settings, CharacterData* characterData)
{
    if (characterData->count == 0)
    {
        return;
    }

    settings->frameMax = ClampInt(settings->frameMax, 0, settings->frameLimit);
    settings->frameMaxSelect = settings->frameMax;
    settings->timeMax = settings->frameMax * characterData->bvhData[characterData->active].frameTime;

    settings->frameMin = ClampInt(settings->frameMin, 0, settings->frameMax);
    settings->frameMinSelect = settings->frameMin;
    settings->timeMin = settings->frameMin * characterData->bvhData[characterData->active].frameTime;

    settings->playTime = Clamp(settings->playTime, settings->timeMin, settings->timeMax);
}
}
