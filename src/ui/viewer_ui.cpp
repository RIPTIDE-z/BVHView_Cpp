// Draws the original BVHView interface

#include <cstdlib>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "ui/raygui_bridge.hpp"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#include "ui/viewer_ui.hpp"
#include "core/args.hpp"
#include "core/math_utils.hpp"
#include "render/drawing.hpp"
#include "ui/file_dialog_scale.hpp"
#include "ui/ui_scale.hpp"
#include "raymath.h"
#include <cstring>

namespace bvhview
{
void GuiOrbitCamera(OrbitCamera* camera, CharacterData* characterData, int argc, char** argv)
{
    GuiGroupBox(UiRectangle(20, 10, 190, 260), "Camera");

    GuiLabel(UiRectangle(30, 20, 150, 20), "Ctrl + Left Click - Rotate");
    GuiLabel(UiRectangle(30, 40, 150, 20), "Ctrl + Right Click - Pan");
    GuiLabel(UiRectangle(30, 60, 150, 20), "Mouse Scroll - Zoom");
    GuiLabel(UiRectangle(30, 80, 225, 20), TextFormat("Target: [% 5.3f % 5.3f % 5.3f]", camera->cam3d.target.x, camera->cam3d.target.y, camera->cam3d.target.z));
    GuiLabel(UiRectangle(30, 100, 225, 20), TextFormat("Offset: [% 5.3f % 5.3f % 5.3f]", camera->offset.x, camera->offset.y, camera->offset.z));
    GuiLabel(UiRectangle(30, 120, 150, 20), TextFormat("Azimuth: %5.3f", camera->azimuth));
    GuiLabel(UiRectangle(30, 140, 150, 20), TextFormat("Altitude: %5.3f", camera->altitude));
    GuiLabel(UiRectangle(30, 160, 150, 20), TextFormat("Distance: %5.3f", camera->distance));

    if (GuiButton(UiRectangle(30, 180, 100, 20), "Reset"))
    {
        camera->azimuth = ArgFloat(argc, argv, "cameraAzimuth", 0.0f);
        camera->altitude = ArgFloat(argc, argv, "cameraAltitude", 0.4f);
        camera->distance = ArgFloat(argc, argv, "cameraDistance", 4.0f);
        camera->offset = ArgVector3(argc, argv, "cameraOffset", Vector3Zero());
        camera->track = ArgBool(argc, argv, "cameraTrack", true);
        camera->trackBone = ArgInt(argc, argv, "cameraTrackBone", 0);
    }

    if (characterData->count > 0)
    {
        GuiToggle(UiRectangle(30, 210, 100, 20), "Track", &camera->track);
        UiComboBox(UiRectangle(30, 240, 150, 20), characterData->jointNamesCombo[characterData->active].c_str(), &camera->trackBone);
    }
}

void GuiRenderSettings(RenderSettings* settings, CapsuleData* capsuleData, int windowWidth, int)
{
    const float screenWidth = UiLogicalSize(static_cast<float>(windowWidth));

    GuiGroupBox(UiRectangle(screenWidth - 260, 10, 240, 430), "Rendering");

    GuiSliderBar(
        UiRectangle(screenWidth - 160, 20, 100, 20),
        "Exposure",
        TextFormat("%5.2f", settings->exposure),
        &settings->exposure,
        0.0f, 3.0f);

    GuiSliderBar(
        UiRectangle(screenWidth - 160, 50, 100, 20),
        "Sun Light",
        TextFormat("%5.2f", settings->sunLightStrength),
        &settings->sunLightStrength,
        0.0f, 1.0f);

    if (GuiSliderBar(
        UiRectangle(screenWidth - 160, 80, 100, 20),
        "Sun Softness",
        TextFormat("%5.2f", settings->sunLightConeAngle),
        &settings->sunLightConeAngle,
        0.02f, PI / 4.0f))
    {
        CapsuleDataUpdateShadowLookupTable(capsuleData, settings->sunLightConeAngle);
    }

    GuiSliderBar(
        UiRectangle(screenWidth - 160, 110, 100, 20),
        "Sky Light",
        TextFormat("%5.2f", settings->skyLightStrength),
        &settings->skyLightStrength,
        0.0f, 1.0f);

    GuiSliderBar(
        UiRectangle(screenWidth - 160, 140, 100, 20),
        "Ambient Light",
        TextFormat("%5.2f", settings->ambientLightStrength),
        &settings->ambientLightStrength,
        0.0f, 2.0f);

    GuiSliderBar(
        UiRectangle(screenWidth - 160, 170, 100, 20),
        "Ground Light",
        TextFormat("%5.2f", settings->groundLightStrength),
        &settings->groundLightStrength,
        0.0f, 0.5f);

    GuiSliderBar(
        UiRectangle(screenWidth - 160, 200, 100, 20),
        "Sun Azimuth",
        TextFormat("%5.2f", settings->sunAzimuth),
        &settings->sunAzimuth,
        -PI, PI);

    GuiSliderBar(
        UiRectangle(screenWidth - 160, 230, 100, 20),
        "Sun Altitude",
        TextFormat("%5.2f", settings->sunAltitude),
        &settings->sunAltitude,
        0.0f, 0.49f * PI);

    GuiCheckBox(UiRectangle(screenWidth - 250, 260, 20, 20), "Draw Origin", &settings->drawOrigin);
    GuiCheckBox(UiRectangle(screenWidth - 130, 260, 20, 20), "Draw Grid", &settings->drawGrid);
    GuiCheckBox(UiRectangle(screenWidth - 250, 290, 20, 20), "Draw Checker", &settings->drawChecker);
    GuiCheckBox(UiRectangle(screenWidth - 130, 290, 20, 20), "Draw Capsules", &settings->drawCapsules);
    GuiCheckBox(UiRectangle(screenWidth - 250, 320, 20, 20), "Draw Wireframes", &settings->drawWireframes);
    GuiCheckBox(UiRectangle(screenWidth - 130, 320, 20, 20), "Draw Skeleton", &settings->drawSkeleton);
    GuiCheckBox(UiRectangle(screenWidth - 250, 350, 20, 20), "Draw Transforms", &settings->drawTransforms);
    GuiCheckBox(UiRectangle(screenWidth - 130, 350, 20, 20), "Draw AO", &settings->drawAO);
    GuiCheckBox(UiRectangle(screenWidth - 250, 380, 20, 20), "Draw Shadows", &settings->drawShadows);
    GuiCheckBox(UiRectangle(screenWidth - 130, 380, 20, 20), "Draw End Sites", &settings->drawEndSites);
    GuiCheckBox(UiRectangle(screenWidth - 250, 410, 20, 20), "Draw FPS", &settings->drawFPS);
    GuiLabel(UiRectangle(screenWidth - 130, 410, 100, 20), "H Key - Hide UI");
}

void GuiCharacterData(
    CharacterData* characterData,
    GuiWindowFileDialogState* fileDialogState,
    ScrubberSettings* scrubberSettings,
    char* errMsg,
    int argc,
    char** argv)
{
    int offsetHeight = 280;

    GuiGroupBox(UiRectangle(20, offsetHeight, 190, (CHARACTERS_MAX - 1) * 30 + 150), "Characters");

    if (GuiButton(UiRectangle(30, offsetHeight + 10, 110, 20), "Open"))
    {
        UiCenterFileDialog(fileDialogState);
        fileDialogState->windowActive = true;
    }

    if (GuiButton(UiRectangle(150, offsetHeight + 10, 50, 20), "Clear"))
    {
        CharacterDataClear(characterData);
        errMsg[0] = '\0';
        ScrubberSettingsInit(scrubberSettings, argc, argv);
        SetWindowTitle("BVHView");
   }

    for (int i = 0; i < characterData->count; i++)
    {
        char bvhNameShort[20];
        bvhNameShort[0] = '\0';
        if (strlen(characterData->names[i].data()) + 1 <= 20)
        {
            strcat(bvhNameShort, characterData->names[i].data());
        }
        else
        {
            memcpy(bvhNameShort, characterData->names[i].data(), 16);
            memcpy(bvhNameShort + 16, "...", 4);
        }

        bool bvhSelected = i == characterData->active;
        GuiToggle(UiRectangle(30, offsetHeight + 40 + i * 30, 140, 20), bvhNameShort, &bvhSelected);

        if (bvhSelected && (characterData->active != i))
        {
            characterData->active = i;
            ScrubberSettingsClamp(scrubberSettings, characterData);

            char windowTitle[512];
            snprintf(windowTitle, 512, "%s - BVHView", characterData->filePaths[characterData->active].data());
            SetWindowTitle(windowTitle);
        }

        const Rectangle colorBounds = UiRectangle(180, offsetHeight + 40 + i * 30, 20, 20);
        DrawRectangleRec(colorBounds, characterData->colors[i]);
        DrawRectangleLinesEx(colorBounds, UiScaleInt(1), GRAY);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            CheckCollisionPointRec(GetMousePosition(), colorBounds))
        {
            characterData->colorPickerActive = !characterData->colorPickerActive;
        }
    }

    if (characterData->count > 0)
    {
        bool scaleM = characterData->scales[characterData->active] == 1.0f;
        GuiToggle(UiRectangle(30, offsetHeight + 60 + (CHARACTERS_MAX - 1) * 30, 30, 20), "m", &scaleM);
        if (scaleM) { characterData->scales[characterData->active] = 1.0f; }

        bool scaleCM = characterData->scales[characterData->active] == 0.01f;
        GuiToggle(UiRectangle(65, offsetHeight + 60 + (CHARACTERS_MAX - 1) * 30, 30, 20), "cm", &scaleCM);
        if (scaleCM) { characterData->scales[characterData->active] = 0.01f; }

        bool scaleInches = characterData->scales[characterData->active] == 0.0254f;
        GuiToggle(UiRectangle(100, offsetHeight + 60 + (CHARACTERS_MAX - 1) * 30, 30, 20), "inch", &scaleInches);
        if (scaleInches) { characterData->scales[characterData->active] = 0.0254f; }

        bool scaleFeet = characterData->scales[characterData->active] == 0.3048f;
        GuiToggle(UiRectangle(135, offsetHeight + 60 + (CHARACTERS_MAX - 1) * 30, 30, 20), "feet", &scaleFeet);
        if (scaleFeet) { characterData->scales[characterData->active] = 0.3048f; }

        bool scaleAuto = characterData->scales[characterData->active] == characterData->autoScales[characterData->active];
        GuiToggle(UiRectangle(170, offsetHeight + 60 + (CHARACTERS_MAX - 1) * 30, 30, 20), "auto", &scaleAuto);
        if (scaleAuto) { characterData->scales[characterData->active] = characterData->autoScales[characterData->active]; }

        GuiSliderBar(
            UiRectangle(70, offsetHeight + 90 + (CHARACTERS_MAX - 1) * 30, 100, 20),
            "Radius",
            TextFormat("%5.2f", characterData->radii[characterData->active]),
            &characterData->radii[characterData->active],
            0.01f, 0.1f);

        GuiSliderBar(
            UiRectangle(70, offsetHeight + 120 + (CHARACTERS_MAX - 1) * 30, 100, 20),
            "Opacity",
            TextFormat("%5.2f", characterData->opacities[characterData->active]),
            &characterData->opacities[characterData->active],
            0.0f, 1.0f);
    }
}

void GuiScrubberSettings(
    ScrubberSettings* settings,
    CharacterData* characterData,
    int windowWidth,
    int windowHeight)
{
    if (characterData->count == 0) { return; }

    const float screenWidth = UiLogicalSize(static_cast<float>(windowWidth));
    const float screenHeight = UiLogicalSize(static_cast<float>(windowHeight));
    float frameTime = characterData->bvhData[characterData->active].frameTime;

    GuiGroupBox(UiRectangle(screenWidth / 2 - 600, screenHeight - 100, 1200, 90), "Scrubber");

    GuiLabel(UiRectangle(screenWidth / 2 - 480, screenHeight - 80, 150, 20), TextFormat("Frame Time: %f", frameTime));
    GuiCheckBox(UiRectangle(screenWidth / 2 - 350, screenHeight - 80, 20, 20), "Snap to Frame", &settings->frameSnap);
    UiComboBox(UiRectangle(screenWidth / 2 - 240, screenHeight - 80, 100, 20), "Nearest;Linear;Cubic", &settings->sampleMode);

    GuiToggle(UiRectangle(screenWidth / 2 - 130, screenHeight - 80, 50, 20), "Inplace", &settings->inplace);
    GuiToggle(UiRectangle(screenWidth / 2 - 70, screenHeight - 80, 50, 20), "Loop", &settings->looping);
    GuiToggle(UiRectangle(screenWidth / 2 - 10, screenHeight - 80, 50, 20), "Play", &settings->playing);

    bool speed01x = settings->playSpeed == 0.1f;
    GuiToggle(UiRectangle(screenWidth / 2 + 50, screenHeight - 80, 30, 20), "0.1x", &speed01x); if (speed01x) { settings->playSpeed = 0.1f; }
    bool speed05x = settings->playSpeed == 0.5f;
    GuiToggle(UiRectangle(screenWidth / 2 + 90, screenHeight - 80, 30, 20), "0.5x", &speed05x); if (speed05x) { settings->playSpeed = 0.5f; }
    bool speed1x = settings->playSpeed == 1.0f;
    GuiToggle(UiRectangle(screenWidth / 2 + 130, screenHeight - 80, 30, 20), "1x", &speed1x); if (speed1x) { settings->playSpeed = 1.0f; }
    bool speed2x = settings->playSpeed == 2.0f;
    GuiToggle(UiRectangle(screenWidth / 2 + 170, screenHeight - 80, 30, 20), "2x", &speed2x); if (speed2x) { settings->playSpeed = 2.0f; }
    bool speed4x = settings->playSpeed == 4.0f;
    GuiToggle(UiRectangle(screenWidth / 2 + 210, screenHeight - 80, 30, 20), "4x", &speed4x); if (speed4x) { settings->playSpeed = 4.0f; }
    GuiSliderBar(UiRectangle(screenWidth / 2 + 250, screenHeight - 80, 70, 20), "", TextFormat("%5.2fx", settings->playSpeed), &settings->playSpeed, 0.0f, 4.0f);

    int frame = ClampInt((int)(settings->playTime / frameTime + 0.5f), settings->frameMin, settings->frameMax);

    if (GuiValueBox(
        UiRectangle(screenWidth / 2 - 540, screenHeight - 80, 50, 20),
        "Min   ", &settings->frameMinSelect, 0, settings->frameLimit, settings->frameMinEdit))
    {
        settings->frameMinEdit = !settings->frameMinEdit;
        if (!settings->frameMinEdit)
        {
            settings->frameMin = settings->frameMinSelect;
            ScrubberSettingsClamp(settings, characterData);
        }
    }

    if (GuiValueBox(
        UiRectangle(screenWidth / 2 + 470, screenHeight - 80, 50, 20),
        "Max   ", &settings->frameMaxSelect, 0, settings->frameLimit, settings->frameMaxEdit))
    {
        settings->frameMaxEdit = !settings->frameMaxEdit;

        if (!settings->frameMaxEdit)
        {
            settings->frameMax = settings->frameMaxSelect;
            ScrubberSettingsClamp(settings, characterData);
        }
    }

    GuiLabel(
        UiRectangle(screenWidth / 2 + 530, screenHeight - 80, 100, 20),
        TextFormat("of %i", settings->frameLimit));

    float frameFloatPrev = settings->frameSnap ? (float)frame : settings->playTime / frameTime;
    float frameFloat = frameFloatPrev;

    GuiSliderBar(
        UiRectangle(screenWidth / 2 - 540, screenHeight - 50, 1080, 20),
        TextFormat("%5.2f", settings->playTime),
        TextFormat("%i", frame),
        &frameFloat,
        (float)settings->frameMin, (float)settings->frameMax);

    if (frameFloat != frameFloatPrev)
    {
        if (settings->frameSnap)
        {
            frame = ClampInt((int)(frameFloat + 0.5f), settings->frameMin, settings->frameMax);
            settings->playTime = Clamp(frame * frameTime, settings->timeMin, settings->timeMax);
        }
        else
        {
            settings->playTime = Clamp(frameFloat * frameTime, settings->timeMin, settings->timeMax);
        }
    }
}
}
