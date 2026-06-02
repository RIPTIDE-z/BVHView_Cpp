// Implements 720p-based UI scaling

#include "ui/ui_scale.hpp"

#include "ui/raygui_bridge.hpp"

#include <algorithm>
#include <cmath>

namespace bvhview
{
namespace
{
constexpr float ReferenceScreenWidth = 1280.0f;
constexpr float ReferenceScreenHeight = 720.0f;
constexpr int DefaultTextSize = 10;
constexpr int DefaultComboButtonWidth = 32;
constexpr int DefaultComboButtonSpacing = 2;

float uiScale = 1.0f;
}

void UiUpdateScale(int screenWidth, int screenHeight)
{
    const float widthScale = static_cast<float>(screenWidth) / ReferenceScreenWidth;
    const float heightScale = static_cast<float>(screenHeight) / ReferenceScreenHeight;
    uiScale = std::max(1.0f, std::min(widthScale, heightScale));
}

float UiScale()
{
    return uiScale;
}

float UiLogicalSize(float value)
{
    return value / uiScale;
}

int UiScaleInt(float value)
{
    return static_cast<int>(std::lround(value * uiScale));
}

Rectangle UiRectangle(float x, float y, float width, float height)
{
    return Rectangle{x * uiScale, y * uiScale, width * uiScale, height * uiScale};
}

void UiApplyRayguiScale()
{
    GuiSetStyle(DEFAULT, TEXT_SIZE, UiScaleInt(DefaultTextSize));
    GuiSetStyle(COMBOBOX, COMBO_BUTTON_WIDTH, UiScaleInt(DefaultComboButtonWidth));
    GuiSetStyle(COMBOBOX, COMBO_BUTTON_SPACING, UiScaleInt(DefaultComboButtonSpacing));
}

void UiResetRayguiScale()
{
    GuiSetStyle(DEFAULT, TEXT_SIZE, DefaultTextSize);
    GuiSetStyle(COMBOBOX, COMBO_BUTTON_WIDTH, DefaultComboButtonWidth);
    GuiSetStyle(COMBOBOX, COMBO_BUTTON_SPACING, DefaultComboButtonSpacing);
}
}
