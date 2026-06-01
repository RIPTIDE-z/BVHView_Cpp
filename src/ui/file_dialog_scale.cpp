// Implements file-dialog scaling helpers

#include "ui/file_dialog_scale.hpp"

#include "ui/ui_scale.hpp"

#include <algorithm>
#include <cmath>

namespace bvhview
{
namespace
{
RenderTexture2D fileDialogTarget{};

void UiClampFileDialog(GuiWindowFileDialogState* state)
{
    const float screenWidth = UiLogicalSize(static_cast<float>(GetScreenWidth()));
    const float screenHeight = UiLogicalSize(static_cast<float>(GetScreenHeight()));
    const float maxX = std::max(0.0f, screenWidth - state->windowBounds.width);
    const float maxY = std::max(0.0f, screenHeight - state->windowBounds.height);

    state->windowBounds.x = std::clamp(state->windowBounds.x, 0.0f, maxX);
    state->windowBounds.y = std::clamp(state->windowBounds.y, 0.0f, maxY);
}

void UiUpdateFileDialogDrag(GuiWindowFileDialogState* state, float scale)
{
    if (!state->supportDrag) { return; }

    Vector2 mousePosition = GetMousePosition();
    mousePosition.x /= scale;
    mousePosition.y /= scale;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(
            mousePosition,
            Rectangle{
                state->windowBounds.x,
                state->windowBounds.y,
                state->windowBounds.width,
                RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT}))
    {
        state->dragMode = true;
        state->panOffset.x = mousePosition.x - state->windowBounds.x;
        state->panOffset.y = mousePosition.y - state->windowBounds.y;
    }

    if (state->dragMode)
    {
        state->windowBounds.x = mousePosition.x - state->panOffset.x;
        state->windowBounds.y = mousePosition.y - state->panOffset.y;
        UiClampFileDialog(state);

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) { state->dragMode = false; }
    }
}

void UiDrawFileDialogControls(GuiWindowFileDialogState* state)
{
    const bool supportDrag = state->supportDrag;
    state->supportDrag = false;
    GuiWindowFileDialog(state);
    state->supportDrag = supportDrag;

    if (!state->windowActive) { state->dragMode = false; }
}

void UiEnsureFileDialogTarget(int width, int height)
{
    if ((fileDialogTarget.id != 0) &&
        ((fileDialogTarget.texture.width != width) || (fileDialogTarget.texture.height != height)))
    {
        UnloadRenderTexture(fileDialogTarget);
        fileDialogTarget = {};
    }

    if (fileDialogTarget.id == 0)
    {
        fileDialogTarget = LoadRenderTexture(width, height);
        SetTextureFilter(fileDialogTarget.texture, TEXTURE_FILTER_BILINEAR);
    }
}
}

void UiCenterFileDialog(GuiWindowFileDialogState* state)
{
    const float screenWidth = UiLogicalSize(static_cast<float>(GetScreenWidth()));
    const float screenHeight = UiLogicalSize(static_cast<float>(GetScreenHeight()));

    state->windowBounds.x = screenWidth / 2.0f - state->windowBounds.width / 2.0f;
    state->windowBounds.y = screenHeight / 2.0f - state->windowBounds.height / 2.0f;
    UiClampFileDialog(state);
}

void UiDrawFileDialog(GuiWindowFileDialogState* state)
{
    if (!state->windowActive) { return; }

    const float scale = UiScale();
    UiUpdateFileDialogDrag(state, scale);

    if (scale <= 1.0f)
    {
        UiDrawFileDialogControls(state);
    }
    else
    {
        const int screenWidth = GetScreenWidth();
        const int screenHeight = GetScreenHeight();
        const int logicalWidth = std::max(1, static_cast<int>(std::ceil(UiLogicalSize(static_cast<float>(screenWidth)))));
        const int logicalHeight = std::max(1, static_cast<int>(std::ceil(UiLogicalSize(static_cast<float>(screenHeight)))));

        UiEnsureFileDialogTarget(logicalWidth, logicalHeight);

        SetMouseScale(1.0f / scale, 1.0f / scale);
        BeginTextureMode(fileDialogTarget);
        ClearBackground(BLANK);
        UiDrawFileDialogControls(state);
        EndTextureMode();
        SetMouseScale(1.0f, 1.0f);

        DrawTexturePro(
            fileDialogTarget.texture,
            Rectangle{0.0f, 0.0f, static_cast<float>(logicalWidth), -static_cast<float>(logicalHeight)},
            Rectangle{0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight)},
            Vector2{},
            0.0f,
            WHITE);
    }

    UiClampFileDialog(state);
}

void UiShutdownFileDialog()
{
    if (fileDialogTarget.id != 0)
    {
        UnloadRenderTexture(fileDialogTarget);
        fileDialogTarget = {};
    }
}
}
