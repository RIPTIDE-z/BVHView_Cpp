// Manages the application lifecycle and per-frame updates

#include "app/application.hpp"
#include "core/args.hpp"
#include "core/math_utils.hpp"
#include "core/profile.hpp"
#include "raymath.h"
#include "rcamera.h"
#include "render/drawing.hpp"
#include "render/geometry.hpp"
#include "render/model.hpp"
#include "rlgl.h"
#include "ui/file_dialog_scale.hpp"
#include "ui/ui_scale.hpp"
#include "ui/viewer_ui.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <memory>

namespace bvhview
{
namespace
{
constexpr int MinimumScreenWidth = 1280;
constexpr int MinimumScreenHeight = 720;
}

void ApplicationUpdate(void* voidApplicationState)
{
    auto* app = static_cast<ApplicationState*>(voidApplicationState);

    if (IsKeyPressed(KEY_F11))
    {
        ToggleFullscreen();
    }

    // Synchronizes drawing dimensions after resizing or toggling fullscreen
    app->screenWidth = GetScreenWidth();
    app->screenHeight = GetScreenHeight();
    UiUpdateScale(app->screenWidth, app->screenHeight);

    if (app->fileDialogState.SelectFilePressed)
    {
        if (IsFileExtension(app->fileDialogState.fileNameText, ".bvh"))
        {
            char fileNameToLoad[512];
            snprintf(fileNameToLoad, 512, "%s/%s", app->fileDialogState.dirPathText, app->fileDialogState.fileNameText);

            if (CharacterDataLoadFromFile(&app->characterData, fileNameToLoad, app->errMsg, 512))
            {
                app->characterData.active = app->characterData.count - 1;

                CapsuleDataUpdateForCharacters(&app->capsuleData, &app->characterData);
                ScrubberSettingsRecomputeLimits(&app->scrubberSettings, &app->characterData);
                ScrubberSettingsInitMaxs(&app->scrubberSettings, &app->characterData);

                char windowTitle[512];
                snprintf(windowTitle, 512, "%s - BVHView",
                         app->characterData.filePaths[app->characterData.active].data());
                SetWindowTitle(windowTitle);
            }
        }
        else
        {
            snprintf(app->errMsg, 512, "Error: File '%s' is not a BVH file.", app->fileDialogState.fileNameText);
        }

        app->fileDialogState.SelectFilePressed = false;
    }

    if (IsFileDropped())
    {
        FilePathList droppedFiles = LoadDroppedFiles();

        int prevBvhCount = app->characterData.count;

        for (unsigned int i = 0; i < droppedFiles.count; i++)
        {
            if (CharacterDataLoadFromFile(&app->characterData, droppedFiles.paths[i], app->errMsg, 512))
            {
                app->characterData.active = app->characterData.count - 1;
            }
        }

        UnloadDroppedFiles(droppedFiles);

        if (app->characterData.count > prevBvhCount)
        {
            CapsuleDataUpdateForCharacters(&app->capsuleData, &app->characterData);
            ScrubberSettingsRecomputeLimits(&app->scrubberSettings, &app->characterData);
            ScrubberSettingsInitMaxs(&app->scrubberSettings, &app->characterData);

            char windowTitle[512];
            snprintf(windowTitle, 512, "%s - BVHView", app->characterData.filePaths[app->characterData.active].data());
            SetWindowTitle(windowTitle);
        }
    }

    if (IsKeyPressed(KEY_H) && !app->fileDialogState.windowActive)
    {
        app->renderSettings.drawUI = !app->renderSettings.drawUI;
    }

    PROFILE_BEGIN(Update);

    if (app->scrubberSettings.playing)
    {
        app->scrubberSettings.playTime += app->scrubberSettings.playSpeed * GetFrameTime();

        if (app->scrubberSettings.playTime >= app->scrubberSettings.timeMax)
        {
            app->scrubberSettings.playTime = (app->scrubberSettings.looping && app->scrubberSettings.timeMax >= 1e-8f)
                                                 ? fmod(app->scrubberSettings.playTime, app->scrubberSettings.timeMax) +
                                                       app->scrubberSettings.timeMin
                                                 : app->scrubberSettings.timeMax;
        }
    }

    for (int i = 0; i < app->characterData.count; i++)
    {
        if (app->scrubberSettings.sampleMode == 0)
        {
            TransformDataSampleFrameNearest(&app->characterData.xformData[i], &app->characterData.bvhData[i],
                                            app->scrubberSettings.playTime, app->characterData.scales[i]);
        }
        else if (app->scrubberSettings.sampleMode == 1)
        {
            TransformDataSampleFrameLinear(&app->characterData.xformData[i], &app->characterData.xformTmp0[i],
                                           &app->characterData.xformTmp1[i], &app->characterData.bvhData[i],
                                           app->scrubberSettings.playTime, app->characterData.scales[i]);
        }
        else
        {
            TransformDataSampleFrameCubic(&app->characterData.xformData[i], &app->characterData.xformTmp0[i],
                                          &app->characterData.xformTmp1[i], &app->characterData.xformTmp2[i],
                                          &app->characterData.xformTmp3[i], &app->characterData.bvhData[i],
                                          app->scrubberSettings.playTime, app->characterData.scales[i]);
        }

        if (app->scrubberSettings.inplace)
        {

            app->characterData.xformData[i].localPositions[0].x = 0.0f;
            app->characterData.xformData[i].localPositions[0].z = 0.0f;

            Quaternion verticalRotation = QuaternionInvert(QuaternionNormalize(Quaternion{
                0.0f,
                app->characterData.xformData[i].localRotations[0].y,
                0.0f,
                app->characterData.xformData[i].localRotations[0].w,
            }));

            app->characterData.xformData[i].localRotations[0] =
                QuaternionMultiply(verticalRotation, app->characterData.xformData[i].localRotations[0]);
        }

        TransformDataForwardKinematics(&app->characterData.xformData[i]);
    }

    Vector3 cameraTarget = Vector3{0.0f, 1.0f, 0.0f};

    if (app->characterData.count > 0 && app->camera.track &&
        app->camera.trackBone < app->characterData.xformData[app->characterData.active].jointCount)
    {
        cameraTarget = app->characterData.xformData[app->characterData.active].globalPositions[app->camera.trackBone];
    }

    if (!app->fileDialogState.windowActive)
    {
        OrbitCameraUpdate(&app->camera, cameraTarget,
                          (IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonDown(0)) ? GetMouseDelta().x : 0.0f,
                          (IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonDown(0)) ? GetMouseDelta().y : 0.0f,
                          (IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonDown(1)) ? GetMouseDelta().x : 0.0f,
                          (IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonDown(1)) ? GetMouseDelta().y : 0.0f,
                          GetMouseWheelMove(), GetFrameTime());
    }

    CapsuleDataReset(&app->capsuleData);
    for (int i = 0; i < app->characterData.count; i++)
    {
        CapsuleDataAppendFromTransformData(&app->capsuleData, &app->characterData.xformData[i],
                                           app->characterData.radii[i], app->characterData.colors[i],
                                           app->characterData.opacities[i], !app->renderSettings.drawEndSites);
    }

    PROFILE_END(Update);

    Frustum frustum = FrustumFromCameraMatrices(
        GetCameraProjectionMatrix(&app->camera.cam3d,
                                  static_cast<float>(app->screenWidth) / static_cast<float>(app->screenHeight)),
        GetCameraViewMatrix(&app->camera.cam3d));

    BeginDrawing();

    PROFILE_BEGIN(Rendering);

    ClearBackground(app->renderSettings.backgroundColor);

    BeginMode3D(app->camera.cam3d);

    Vector3 sunColorValue = {app->renderSettings.sunColor.r / 255.0f, app->renderSettings.sunColor.g / 255.0f,
                             app->renderSettings.sunColor.b / 255.0f};
    Vector3 skyColorValue = {app->renderSettings.skyColor.r / 255.0f, app->renderSettings.skyColor.g / 255.0f,
                             app->renderSettings.skyColor.b / 255.0f};
    float objectSpecularity = 0.5f;
    float objectGlossiness = 10.0f;
    float objectOpacity = 1.0f;

    Vector3 sunLightPosition = Vector3RotateByQuaternion(
        Vector3{0.0f, 0.0f, 1.0f}, QuaternionFromAxisAngle(Vector3{0.0f, 1.0f, 0.0f}, app->renderSettings.sunAzimuth));
    Vector3 sunLightAxis = Vector3Normalize(Vector3CrossProduct(sunLightPosition, Vector3{0.0f, 1.0f, 0.0f}));
    Vector3 sunLightDir = Vector3Negate(Vector3RotateByQuaternion(
        sunLightPosition, QuaternionFromAxisAngle(sunLightAxis, app->renderSettings.sunAltitude)));

    SetShaderValue(app->shader, app->uniforms.cameraPosition, &app->camera.cam3d.position, SHADER_UNIFORM_VEC3);
    SetShaderValue(app->shader, app->uniforms.exposure, &app->renderSettings.exposure, SHADER_UNIFORM_FLOAT);
    SetShaderValue(app->shader, app->uniforms.sunDir, &sunLightDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(app->shader, app->uniforms.sunStrength, &app->renderSettings.sunLightStrength, SHADER_UNIFORM_FLOAT);
    SetShaderValue(app->shader, app->uniforms.sunColor, &sunColorValue, SHADER_UNIFORM_VEC3);
    SetShaderValue(app->shader, app->uniforms.skyStrength, &app->renderSettings.skyLightStrength, SHADER_UNIFORM_FLOAT);
    SetShaderValue(app->shader, app->uniforms.skyColor, &skyColorValue, SHADER_UNIFORM_VEC3);
    SetShaderValue(app->shader, app->uniforms.ambientStrength, &app->renderSettings.ambientLightStrength,
                   SHADER_UNIFORM_FLOAT);
    SetShaderValue(app->shader, app->uniforms.groundStrength, &app->renderSettings.groundLightStrength,
                   SHADER_UNIFORM_FLOAT);
    SetShaderValue(app->shader, app->uniforms.objectSpecularity, &objectSpecularity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(app->shader, app->uniforms.objectGlossiness, &objectGlossiness, SHADER_UNIFORM_FLOAT);
    SetShaderValue(app->shader, app->uniforms.objectOpacity, &objectOpacity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(app->shader, app->uniforms.aoLookupResolution, &app->capsuleData.aoLookupResolution,
                   SHADER_UNIFORM_VEC2);
    SetShaderValue(app->shader, app->uniforms.shadowLookupResolution, &app->capsuleData.shadowLookupResolution,
                   SHADER_UNIFORM_VEC2);
    BindViewerLookupTextures(app->shader, app->uniforms, app->capsuleData.aoLookupTable,
                             app->capsuleData.shadowLookupTable);

    PROFILE_BEGIN(RenderingGround);

    if (app->renderSettings.drawChecker)
    {
        int groundIsCapsule = 0;
        Vector3 groundColor = {0.75f, 0.75f, 0.75f};

        SetShaderValue(app->shader, app->uniforms.isCapsule, &groundIsCapsule, SHADER_UNIFORM_INT);
        SetShaderValue(app->shader, app->uniforms.objectColor, &groundColor, SHADER_UNIFORM_VEC3);

        for (int i = 0; i < 11; i++)
        {
            for (int j = 0; j < 11; j++)
            {

                Vector3 groundSegmentPosition = {
                    (((float)i / 10) - 0.5f) * 20.0f,
                    0.0f,
                    (((float)j / 10) - 0.5f) * 20.0f,
                };

                if (!FrustumContainsSphere(frustum, groundSegmentPosition, sqrtf(2.0f)))
                {
                    continue;
                }

                PROFILE_BEGIN(RenderingGroundSegment);

                PROFILE_BEGIN(RenderingGroundSegmentAO);

                app->capsuleData.aoCapsuleCount = 0;
                if (app->renderSettings.drawCapsules && app->renderSettings.drawAO)
                {
                    CapsuleDataUpdateAOCapsulesForGroundSegment(&app->capsuleData, groundSegmentPosition);
                }
                int aoCapsuleCount = MinInt(app->capsuleData.aoCapsuleCount, AO_CAPSULES_MAX);

                PROFILE_END(RenderingGroundSegmentAO);

                SetShaderValue(app->shader, app->uniforms.aoCapsuleCount, &aoCapsuleCount, SHADER_UNIFORM_INT);
                SetShaderValueV(app->shader, app->uniforms.aoCapsuleStarts, app->capsuleData.aoCapsuleStarts.data(),
                                SHADER_UNIFORM_VEC3, aoCapsuleCount);
                SetShaderValueV(app->shader, app->uniforms.aoCapsuleVectors, app->capsuleData.aoCapsuleVectors.data(),
                                SHADER_UNIFORM_VEC3, aoCapsuleCount);
                SetShaderValueV(app->shader, app->uniforms.aoCapsuleRadii, app->capsuleData.aoCapsuleRadii.data(),
                                SHADER_UNIFORM_FLOAT, aoCapsuleCount);

                PROFILE_BEGIN(RenderingGroundSegmentShadow);

                app->capsuleData.shadowCapsuleCount = 0;
                if (app->renderSettings.drawCapsules && app->renderSettings.drawShadows)
                {
                    CapsuleDataUpdateShadowCapsulesForGroundSegment(&app->capsuleData, groundSegmentPosition,
                                                                    sunLightDir, app->renderSettings.sunLightConeAngle);
                }
                int shadowCapsuleCount = MinInt(app->capsuleData.shadowCapsuleCount, SHADOW_CAPSULES_MAX);

                PROFILE_END(RenderingGroundSegmentShadow);

                SetShaderValue(app->shader, app->uniforms.shadowCapsuleCount, &shadowCapsuleCount, SHADER_UNIFORM_INT);
                SetShaderValueV(app->shader, app->uniforms.shadowCapsuleStarts,
                                app->capsuleData.shadowCapsuleStarts.data(), SHADER_UNIFORM_VEC3, shadowCapsuleCount);
                SetShaderValueV(app->shader, app->uniforms.shadowCapsuleVectors,
                                app->capsuleData.shadowCapsuleVectors.data(), SHADER_UNIFORM_VEC3, shadowCapsuleCount);
                SetShaderValueV(app->shader, app->uniforms.shadowCapsuleRadii,
                                app->capsuleData.shadowCapsuleRadii.data(), SHADER_UNIFORM_FLOAT, shadowCapsuleCount);

                DrawModel(app->groundPlaneModel, groundSegmentPosition, 1.0f, WHITE);

                PROFILE_END(RenderingGroundSegment);
            }
        }
    }

    PROFILE_END(RenderingGround);

    PROFILE_BEGIN(RenderingCapsules);

    if (app->renderSettings.drawCapsules)
    {

        for (int i = 0; i < app->capsuleData.capsuleCount; i++)
        {
            app->capsuleData.capsuleSort[i].index = i;
            app->capsuleData.capsuleSort[i].value =
                Vector3DistanceSqr(app->camera.cam3d.position, app->capsuleData.capsulePositions[i]);
        }

        std::sort(app->capsuleData.capsuleSort.begin(),
                  app->capsuleData.capsuleSort.begin() + app->capsuleData.capsuleCount,
                  [](const CapsuleSort& lhs, const CapsuleSort& rhs) { return lhs.value > rhs.value; });

        int capsuleIsCapsule = 1;
        SetShaderValue(app->shader, app->uniforms.isCapsule, &capsuleIsCapsule, SHADER_UNIFORM_INT);

        for (int i = 0; i < app->capsuleData.capsuleCount; i++)
        {
            int j = app->capsuleData.capsuleSort[i].index;

            Vector3 capsulePosition = app->capsuleData.capsulePositions[j];
            float capsuleHalfLength = app->capsuleData.capsuleHalfLengths[j];
            float capsuleRadius = app->capsuleData.capsuleRadii[j];

            if (!FrustumContainsSphere(frustum, capsulePosition, capsuleHalfLength + capsuleRadius))
            {
                continue;
            }

            PROFILE_BEGIN(RenderingCapsulesCapsule);

            if (app->capsuleData.capsuleOpacities[j] < 1.0f)
            {
                rlDrawRenderBatchActive();
                rlDisableDepthMask();
            }

            const Vector3 capsuleStart = app->capsuleData.capsuleStarts[j];
            const Vector3 capsuleVector = app->capsuleData.capsuleVectors[j];

            SetShaderValue(app->shader, app->uniforms.objectColor, &app->capsuleData.capsuleColors[j],
                           SHADER_UNIFORM_VEC3);
            SetShaderValue(app->shader, app->uniforms.objectOpacity, &app->capsuleData.capsuleOpacities[j],
                           SHADER_UNIFORM_FLOAT);
            SetShaderValue(app->shader, app->uniforms.capsulePosition, &app->capsuleData.capsulePositions[j],
                           SHADER_UNIFORM_VEC3);
            SetShaderValue(app->shader, app->uniforms.capsuleRotation, &app->capsuleData.capsuleRotations[j],
                           SHADER_UNIFORM_VEC4);
            SetShaderValue(app->shader, app->uniforms.capsuleHalfLength, &app->capsuleData.capsuleHalfLengths[j],
                           SHADER_UNIFORM_FLOAT);
            SetShaderValue(app->shader, app->uniforms.capsuleRadius, &app->capsuleData.capsuleRadii[j],
                           SHADER_UNIFORM_FLOAT);
            SetShaderValue(app->shader, app->uniforms.capsuleStart, &capsuleStart, SHADER_UNIFORM_VEC3);
            SetShaderValue(app->shader, app->uniforms.capsuleVector, &capsuleVector, SHADER_UNIFORM_VEC3);

            PROFILE_BEGIN(RenderingCapsulesCapsuleAO);

            app->capsuleData.aoCapsuleCount = 0;
            if (app->renderSettings.drawAO)
            {
                CapsuleDataUpdateAOCapsulesForCapsule(&app->capsuleData, j);
            }
            int aoCapsuleCount = MinInt(app->capsuleData.aoCapsuleCount, AO_CAPSULES_MAX);

            PROFILE_END(RenderingCapsulesCapsuleAO);

            SetShaderValue(app->shader, app->uniforms.aoCapsuleCount, &aoCapsuleCount, SHADER_UNIFORM_INT);
            SetShaderValueV(app->shader, app->uniforms.aoCapsuleStarts, app->capsuleData.aoCapsuleStarts.data(),
                            SHADER_UNIFORM_VEC3, aoCapsuleCount);
            SetShaderValueV(app->shader, app->uniforms.aoCapsuleVectors, app->capsuleData.aoCapsuleVectors.data(),
                            SHADER_UNIFORM_VEC3, aoCapsuleCount);
            SetShaderValueV(app->shader, app->uniforms.aoCapsuleRadii, app->capsuleData.aoCapsuleRadii.data(),
                            SHADER_UNIFORM_FLOAT, aoCapsuleCount);

            PROFILE_BEGIN(RenderingCapsulesCapsuleShadow);

            app->capsuleData.shadowCapsuleCount = 0;
            if (app->renderSettings.drawShadows)
            {
                CapsuleDataUpdateShadowCapsulesForCapsule(&app->capsuleData, j, sunLightDir,
                                                          app->renderSettings.sunLightConeAngle);
            }
            int shadowCapsuleCount = MinInt(app->capsuleData.shadowCapsuleCount, SHADOW_CAPSULES_MAX);

            PROFILE_END(RenderingCapsulesCapsuleShadow);

            SetShaderValue(app->shader, app->uniforms.shadowCapsuleCount, &shadowCapsuleCount, SHADER_UNIFORM_INT);
            SetShaderValueV(app->shader, app->uniforms.shadowCapsuleStarts, app->capsuleData.shadowCapsuleStarts.data(),
                            SHADER_UNIFORM_VEC3, shadowCapsuleCount);
            SetShaderValueV(app->shader, app->uniforms.shadowCapsuleVectors,
                            app->capsuleData.shadowCapsuleVectors.data(), SHADER_UNIFORM_VEC3, shadowCapsuleCount);
            SetShaderValueV(app->shader, app->uniforms.shadowCapsuleRadii, app->capsuleData.shadowCapsuleRadii.data(),
                            SHADER_UNIFORM_FLOAT, shadowCapsuleCount);

            DrawModel(app->capsuleModel, Vector3Zero(), 1.0f, WHITE);

            if (app->capsuleData.capsuleOpacities[j] < 1.0f)
            {
                rlDrawRenderBatchActive();
                rlEnableDepthMask();
            }

            PROFILE_END(RenderingCapsulesCapsule);
        }
    }

    PROFILE_END(RenderingCapsules);

    if (app->renderSettings.drawGrid)
    {
        DrawViewerGrid();
    }

    rlDrawRenderBatchActive();
    rlDisableDepthTest();

    UnbindViewerLookupTextures();

    if (app->renderSettings.drawOrigin)
    {
        DrawViewerOrigin();
    }

    if (app->renderSettings.drawWireframes)
    {
        DrawWireFrames(&app->capsuleData, DARKGRAY);
    }

    if (app->renderSettings.drawSkeleton)
    {
        for (int i = 0; i < app->characterData.count; i++)
        {
            DrawSkeleton(&app->characterData.xformData[i], app->renderSettings.drawEndSites, DARKGRAY, GRAY);
        }
    }

    if (app->renderSettings.drawTransforms)
    {
        for (int i = 0; i < app->characterData.count; i++)
        {
            DrawTransforms(&app->characterData.xformData[i]);
        }
    }

    rlDrawRenderBatchActive();
    rlEnableDepthTest();

    EndMode3D();

    PROFILE_END(Rendering);

    PROFILE_BEGIN(Gui);

    if (app->renderSettings.drawUI)
    {
        UiApplyRayguiScale();

        if (app->fileDialogState.windowActive)
        {
            GuiLock();
        }

        UiDrawText(app->uiFont, app->errMsg,
                   Vector2{static_cast<float>(UiScaleInt(250)), static_cast<float>(UiScaleInt(20))},
                   static_cast<float>(UiScaleInt(15)), RED);

        if (app->characterData.count == 0)
        {
            constexpr const char* DropMessage = "Drag and Drop .bvh files to open them.";
            const float dropMessageSize = static_cast<float>(UiScaleInt(30));
            const Vector2 dropMessageBounds =
                MeasureTextEx(app->uiFont.overlayFont, DropMessage, dropMessageSize, 1.0f);
            UiDrawText(app->uiFont, DropMessage,
                       Vector2{(static_cast<float>(app->screenWidth) - dropMessageBounds.x) / 2.0f,
                               (static_cast<float>(app->screenHeight) - dropMessageBounds.y) / 2.0f},
                       dropMessageSize, DARKGRAY);
        }

        GuiRenderSettings(&app->renderSettings, &app->capsuleData, app->screenWidth, app->screenHeight);

        if (app->renderSettings.drawFPS)
        {
            const int fps = GetFPS();
            const Color fpsColor = fps >= 30 ? LIME : fps >= 15 ? ORANGE : RED;
            UiDrawText(app->uiFont, TextFormat("%2i FPS", fps),
                       Vector2{static_cast<float>(UiScaleInt(230)), static_cast<float>(UiScaleInt(10))},
                       static_cast<float>(UiScaleInt(20)), fpsColor);
        }

        GuiOrbitCamera(&app->camera, &app->characterData, app->argc, app->argv);

        GuiCharacterData(&app->characterData, &app->fileDialogState, &app->scrubberSettings, app->errMsg, app->argc,
                         app->argv);

        if (app->characterData.colorPickerActive)
        {
            const float screenWidth = UiLogicalSize(static_cast<float>(app->screenWidth));
            GuiGroupBox(UiRectangle(screenWidth - 180, 450, 160, 140), "Color Picker");
            GuiColorPicker(UiRectangle(screenWidth - 165, 465, 110, 110), NULL,
                           &app->characterData.colors[app->characterData.active]);
        }

        GuiScrubberSettings(&app->scrubberSettings, &app->characterData, app->screenWidth, app->screenHeight);

        if (app->fileDialogState.windowActive)
        {
            GuiUnlock();
        }

        UiResetRayguiScale();
        UiDrawFileDialog(&app->fileDialogState);
    }

    PROFILE_END(Gui);

#if defined(ENABLE_PROFILE) && defined(_WIN32)

    PROFILE_TICKERS_UPDATE();
    UiApplyRayguiScale();

    for (int i = 0; i < globalProfileRecords.num; i++)
    {
        GuiLabel(UiRectangle(260, 10 + (float)i * 20, 200, 20), globalProfileRecords.records[i]->name);
        GuiLabel(UiRectangle(450, 10 + (float)i * 20, 100, 20), TextFormat("%6.1f us", globalProfileTickers.times[i]));
        GuiLabel(UiRectangle(550, 10 + (float)i * 20, 100, 20),
                 TextFormat("%i calls", globalProfileTickers.samples[i]));
    }
    UiResetRayguiScale();
#endif

    EndDrawing();
}

void ApplicationInit(ApplicationState* app, int argc, char** argv)
{
    app->argc = argc;
    app->argv = argv;
    app->screenWidth = MaxInt(ArgInt(argc, argv, "screenWidth", MinimumScreenWidth), MinimumScreenWidth);
    app->screenHeight = MaxInt(ArgInt(argc, argv, "screenHeight", MinimumScreenHeight), MinimumScreenHeight);

    SetConfigFlags(FLAG_VSYNC_HINT);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(app->screenWidth, app->screenHeight, "BVHView");
    SetWindowMinSize(MinimumScreenWidth, MinimumScreenHeight);
    SetTargetFPS(60);
    UiUpdateScale(app->screenWidth, app->screenHeight);
    app->uiFont = UiFontLoad();

    OrbitCameraInit(&app->camera, argc, argv);
    app->shader = LoadViewerShader();
    ShaderUniformsInit(&app->uniforms, app->shader);

    app->groundPlaneMesh = GenMeshPlane(2.0f, 2.0f, 1, 1);
    app->groundPlaneModel = LoadModelFromMesh(app->groundPlaneMesh);
    app->groundPlaneModel.materials[0].shader = app->shader;
    app->capsuleModel = LoadCapsuleModel();
    app->capsuleModel.materials[0].shader = app->shader;

    CharacterDataInit(&app->characterData, argc, argv);
    CapsuleDataInit(&app->capsuleData);
    ScrubberSettingsInit(&app->scrubberSettings, argc, argv);
    RenderSettingsInit(&app->renderSettings, argc, argv);
    CapsuleDataUpdateShadowLookupTable(&app->capsuleData, app->renderSettings.sunLightConeAngle);

    app->fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
    app->errMsg[0] = '\0';

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            continue;
        }
        CharacterDataLoadFromFile(&app->characterData, argv[i], app->errMsg, 512);
    }

    if (app->characterData.count > 0)
    {
        app->characterData.active = app->characterData.count - 1;
        CapsuleDataUpdateForCharacters(&app->capsuleData, &app->characterData);
        ScrubberSettingsRecomputeLimits(&app->scrubberSettings, &app->characterData);
        ScrubberSettingsInitMaxs(&app->scrubberSettings, &app->characterData);

        char windowTitle[512];
        std::snprintf(windowTitle, sizeof(windowTitle), "%s - BVHView",
                      app->characterData.filePaths[app->characterData.active].data());
        SetWindowTitle(windowTitle);
    }
}

void ApplicationShutdown(ApplicationState* app)
{
    UiShutdownFileDialog();
    CapsuleDataFree(&app->capsuleData);
    CharacterDataFree(&app->characterData);
    UnloadModel(app->capsuleModel);
    UnloadModel(app->groundPlaneModel);
    UnloadShader(app->shader);
    UiFontUnload(&app->uiFont);
    CloseWindow();
}
}
