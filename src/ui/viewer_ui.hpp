#pragma once

// Declares the original BVHView interface

#include "animation/character_data.hpp"
#include "camera/orbit_camera.hpp"
#include "render/capsule_data.hpp"
#include "ui/raygui_bridge.hpp"
#include "ui/viewer_settings.hpp"

namespace bvhview
{
void GuiOrbitCamera(OrbitCamera* camera, CharacterData* characterData, int argc, char** argv);
void GuiRenderSettings(RenderSettings* settings, CapsuleData* capsuleData, int screenWidth, int screenHeight);
void GuiCharacterData(CharacterData* characterData, GuiWindowFileDialogState* fileDialogState,
                      ScrubberSettings* scrubberSettings, char* errMsg, int argc, char** argv);
void GuiScrubberSettings(ScrubberSettings* settings, CharacterData* characterData, int screenWidth, int screenHeight);
}
