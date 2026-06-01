#pragma once

// Declares rendering and timeline settings

#include "raylib.h"

namespace bvhview
{
struct CapsuleData;
struct CharacterData;

struct RenderSettings
{
    Color backgroundColor;
    float sunLightConeAngle;
    float sunLightStrength;
    float sunAzimuth;
    float sunAltitude;
    Color sunColor;
    float skyLightStrength;
    Color skyColor;
    float groundLightStrength;
    float ambientLightStrength;
    float exposure;
    bool drawOrigin;
    bool drawGrid;
    bool drawChecker;
    bool drawCapsules;
    bool drawWireframes;
    bool drawSkeleton;
    bool drawTransforms;
    bool drawAO;
    bool drawShadows;
    bool drawEndSites;
    bool drawFPS;
    bool drawUI;
};

struct ScrubberSettings
{
    bool playing;
    bool looping;
    bool inplace;
    float playTime;
    float playSpeed;
    bool frameSnap;
    int sampleMode;
    float timeLimit;
    int frameLimit;
    int frameMin;
    int frameMax;
    int frameMinSelect;
    int frameMaxSelect;
    bool frameMinEdit;
    bool frameMaxEdit;
    float timeMin;
    float timeMax;
};

void RenderSettingsInit(RenderSettings* settings, int argc, char** argv);
void ScrubberSettingsInit(ScrubberSettings* settings, int argc, char** argv);
void ScrubberSettingsRecomputeLimits(ScrubberSettings* settings, CharacterData* characterData);
void ScrubberSettingsInitMaxs(ScrubberSettings* settings, CharacterData* characterData);
void ScrubberSettingsClamp(ScrubberSettings* settings, CharacterData* characterData);
}
