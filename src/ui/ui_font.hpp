#pragma once

// Declares the embedded UI font lifecycle

#include "raylib.h"

namespace bvhview
{
struct UiFont
{
    Font guiFont{};
    Font overlayFont{};
    bool guiOwned = false;
    bool overlayOwned = false;
};

UiFont UiFontLoad();
void UiFontUnload(UiFont* font);
void UiDrawText(const UiFont& font, const char* text, Vector2 position, float size, Color color);
}