// Implements 720p-based UI scaling

#include "ui/ui_scale.hpp"

#include "ui/raygui_bridge.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>

namespace bvhview
{
namespace
{
constexpr float ReferenceScreenWidth = 1280.0f;
constexpr float ReferenceScreenHeight = 720.0f;
constexpr int ComboButtonPadding = 8;
constexpr int GuiLogicalTextSize = 12;

struct StyleMetric
{
    int control;
    int property;
};

constexpr std::array StyleMetrics{
    StyleMetric{DEFAULT, TEXT_SIZE},
    StyleMetric{DEFAULT, TEXT_SPACING},
    StyleMetric{DEFAULT, TEXT_LINE_SPACING},
    StyleMetric{DEFAULT, BORDER_WIDTH},
    StyleMetric{DEFAULT, TEXT_PADDING},
    StyleMetric{LABEL, BORDER_WIDTH},
    StyleMetric{LABEL, TEXT_PADDING},
    StyleMetric{BUTTON, BORDER_WIDTH},
    StyleMetric{BUTTON, TEXT_PADDING},
    StyleMetric{TOGGLE, BORDER_WIDTH},
    StyleMetric{TOGGLE, TEXT_PADDING},
    StyleMetric{SLIDER, BORDER_WIDTH},
    StyleMetric{SLIDER, TEXT_PADDING},
    StyleMetric{PROGRESSBAR, BORDER_WIDTH},
    StyleMetric{PROGRESSBAR, TEXT_PADDING},
    StyleMetric{CHECKBOX, BORDER_WIDTH},
    StyleMetric{CHECKBOX, TEXT_PADDING},
    StyleMetric{COMBOBOX, BORDER_WIDTH},
    StyleMetric{COMBOBOX, TEXT_PADDING},
    StyleMetric{DROPDOWNBOX, BORDER_WIDTH},
    StyleMetric{DROPDOWNBOX, TEXT_PADDING},
    StyleMetric{TEXTBOX, BORDER_WIDTH},
    StyleMetric{TEXTBOX, TEXT_PADDING},
    StyleMetric{VALUEBOX, BORDER_WIDTH},
    StyleMetric{VALUEBOX, TEXT_PADDING},
    StyleMetric{LISTVIEW, BORDER_WIDTH},
    StyleMetric{LISTVIEW, TEXT_PADDING},
    StyleMetric{COLORPICKER, BORDER_WIDTH},
    StyleMetric{COLORPICKER, TEXT_PADDING},
    StyleMetric{SCROLLBAR, BORDER_WIDTH},
    StyleMetric{SCROLLBAR, TEXT_PADDING},
    StyleMetric{STATUSBAR, BORDER_WIDTH},
    StyleMetric{STATUSBAR, TEXT_PADDING},
    StyleMetric{TOGGLE, GROUP_PADDING},
    StyleMetric{SLIDER, SLIDER_WIDTH},
    StyleMetric{SLIDER, SLIDER_PADDING},
    StyleMetric{PROGRESSBAR, PROGRESS_PADDING},
    StyleMetric{CHECKBOX, CHECK_PADDING},
    StyleMetric{COMBOBOX, COMBO_BUTTON_WIDTH},
    StyleMetric{COMBOBOX, COMBO_BUTTON_SPACING},
    StyleMetric{DROPDOWNBOX, ARROW_PADDING},
    StyleMetric{DROPDOWNBOX, DROPDOWN_ITEMS_SPACING},
    StyleMetric{VALUEBOX, SPINNER_BUTTON_WIDTH},
    StyleMetric{VALUEBOX, SPINNER_BUTTON_SPACING},
    StyleMetric{SCROLLBAR, ARROWS_SIZE},
    StyleMetric{SCROLLBAR, SCROLL_SLIDER_PADDING},
    StyleMetric{SCROLLBAR, SCROLL_SLIDER_SIZE},
    StyleMetric{SCROLLBAR, SCROLL_PADDING},
    StyleMetric{LISTVIEW, LIST_ITEMS_HEIGHT},
    StyleMetric{LISTVIEW, LIST_ITEMS_SPACING},
    StyleMetric{LISTVIEW, SCROLLBAR_WIDTH},
    StyleMetric{LISTVIEW, LIST_ITEMS_BORDER_WIDTH},
    StyleMetric{COLORPICKER, COLOR_SELECTOR_SIZE},
    StyleMetric{COLORPICKER, HUEBAR_WIDTH},
    StyleMetric{COLORPICKER, HUEBAR_PADDING},
    StyleMetric{COLORPICKER, HUEBAR_SELECTOR_HEIGHT},
    StyleMetric{COLORPICKER, HUEBAR_SELECTOR_OVERFLOW},
};

std::array<int, StyleMetrics.size()> defaultStyleValues{};
bool styleDefaultsInitialized = false;
float uiScale = 1.0f;

void UiInitializeStyleDefaults()
{
    if (styleDefaultsInitialized)
    {
        return;
    }

    for (std::size_t i = 0; i < StyleMetrics.size(); i++)
    {
        defaultStyleValues[i] = ((StyleMetrics[i].control == DEFAULT) && (StyleMetrics[i].property == TEXT_SIZE))
                                    ? GuiLogicalTextSize
                                    : GuiGetStyle(StyleMetrics[i].control, StyleMetrics[i].property);
    }
    styleDefaultsInitialized = true;
}
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
    UiInitializeStyleDefaults();
    for (std::size_t i = 0; i < StyleMetrics.size(); i++)
    {
        GuiSetStyle(StyleMetrics[i].control, StyleMetrics[i].property,
                    UiScaleInt(static_cast<float>(defaultStyleValues[i])));
    }
}

void UiResetRayguiScale()
{
    UiInitializeStyleDefaults();
    for (std::size_t i = 0; i < StyleMetrics.size(); i++)
    {
        GuiSetStyle(StyleMetrics[i].control, StyleMetrics[i].property, defaultStyleValues[i]);
    }
}

int UiComboBox(Rectangle bounds, const char* text, int* active)
{
    int itemCount = 1;
    for (const char* cursor = text; *cursor != '\0'; cursor++)
    {
        if (*cursor == ';')
        {
            itemCount++;
        }
    }

    char selectorText[32];
    std::snprintf(selectorText, sizeof(selectorText), "%i/%i", itemCount, itemCount);
    const int previousWidth = GuiGetStyle(COMBOBOX, COMBO_BUTTON_WIDTH);
    const int selectorWidth = GuiGetTextWidth(selectorText) + UiScaleInt(ComboButtonPadding);
    GuiSetStyle(COMBOBOX, COMBO_BUTTON_WIDTH, std::max(previousWidth, selectorWidth));
    const int result = GuiComboBox(bounds, text, active);
    GuiSetStyle(COMBOBOX, COMBO_BUTTON_WIDTH, previousWidth);
    return result;
}

}