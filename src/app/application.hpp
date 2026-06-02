#pragma once

// Declares application state and lifecycle functions

#include "animation/character_data.hpp"
#include "camera/orbit_camera.hpp"
#include "render/capsule_data.hpp"
#include "render/shader.hpp"
#include "ui/raygui_bridge.hpp"
#include "ui/ui_font.hpp"
#include "ui/viewer_settings.hpp"

namespace bvhview
{
struct ApplicationState
{
    int argc;
    char** argv;
    int screenWidth;
    int screenHeight;
    OrbitCamera camera;
    Shader shader;
    ShaderUniforms uniforms;
    Mesh groundPlaneMesh;
    Model groundPlaneModel;
    Model capsuleModel;
    CharacterData characterData;
    CapsuleData capsuleData;
    ScrubberSettings scrubberSettings;
    RenderSettings renderSettings;
    GuiWindowFileDialogState fileDialogState;
    UiFont uiFont;
    char errMsg[512];
};

void ApplicationInit(ApplicationState* app, int argc, char** argv);
void ApplicationUpdate(void* voidApplicationState);
void ApplicationShutdown(ApplicationState* app);
}
