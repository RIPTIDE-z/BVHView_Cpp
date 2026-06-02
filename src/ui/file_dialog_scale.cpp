// Implements file-dialog scaling helpers

#include "ui/file_dialog_scale.hpp"

#include "ui/ui_scale.hpp"

#include "rlgl.h"

#include <algorithm>

namespace bvhview
{
namespace
{
Vector2 UiMouseScale(float uiScale)
{
    const Vector2 dpiScale = GetWindowScaleDPI();
    return Vector2{1.0f / (dpiScale.x * uiScale), 1.0f / (dpiScale.y * uiScale)};
}

void UiSetMouseScale(float uiScale)
{
    const Vector2 mouseScale = UiMouseScale(uiScale);
    SetMouseScale(mouseScale.x, mouseScale.y);
}

void UiClampFileDialog(GuiWindowFileDialogState* state)
{
    const float screenWidth = UiLogicalSize(static_cast<float>(GetScreenWidth()));
    const float screenHeight = UiLogicalSize(static_cast<float>(GetScreenHeight()));
    const float maxX = std::max(0.0f, screenWidth - state->windowBounds.width);
    const float maxY = std::max(0.0f, screenHeight - state->windowBounds.height);

    state->windowBounds.x = std::clamp(state->windowBounds.x, 0.0f, maxX);
    state->windowBounds.y = std::clamp(state->windowBounds.y, 0.0f, maxY);
}

void UiUpdateFileDialogDrag(GuiWindowFileDialogState* state)
{
    if (!state->supportDrag) { return; }

    const Vector2 mousePosition = GetMousePosition();
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
    UiSetMouseScale(scale);
    UiUpdateFileDialogDrag(state);

    rlPushMatrix();
    rlScalef(scale, scale, 1.0f);
    UiDrawFileDialogControls(state);
    rlPopMatrix();

    UiSetMouseScale(1.0f);
    UiClampFileDialog(state);
}

void UiShutdownFileDialog()
{
}
}