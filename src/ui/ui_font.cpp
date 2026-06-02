// Loads the embedded Inter Regular font

#include "ui/ui_font.hpp"

#include "ui/raygui_bridge.hpp"
#include "ui/resource_ids.h"

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>

namespace bvhview
{
namespace
{
constexpr int GuiFontAtlasSize = 24;
constexpr int OverlayFontAtlasSize = 64;

Font LoadEmbeddedFont(const unsigned char* data, int dataSize, int atlasSize, bool* owned)
{
    Font font = LoadFontFromMemory(".ttf", data, dataSize, atlasSize, nullptr, 0);
    *owned = (font.texture.id != 0) && (font.texture.id != GetFontDefault().texture.id);
    if (*owned) { SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR); }
    return font;
}
}

UiFont UiFontLoad()
{
    UiFont result{GetFontDefault(), GetFontDefault(), false, false};
    const HMODULE module = GetModuleHandleW(nullptr);
    const auto resourceName = reinterpret_cast<LPCWSTR>(static_cast<ULONG_PTR>(IDR_INTER_REGULAR_TTF));
    const auto resourceType = reinterpret_cast<LPCWSTR>(static_cast<ULONG_PTR>(10));
    const HRSRC resource = FindResourceW(module, resourceName, resourceType);
    if (resource == nullptr) { return result; }

    const HGLOBAL resourceData = LoadResource(module, resource);
    if (resourceData == nullptr) { return result; }

    const auto* data = static_cast<const unsigned char*>(LockResource(resourceData));
    const DWORD dataSize = SizeofResource(module, resource);
    if ((data == nullptr) || (dataSize == 0)) { return result; }

    result.guiFont = LoadEmbeddedFont(data, static_cast<int>(dataSize), GuiFontAtlasSize, &result.guiOwned);
    result.overlayFont = LoadEmbeddedFont(data, static_cast<int>(dataSize), OverlayFontAtlasSize, &result.overlayOwned);
    GuiSetFont(result.guiFont);
    return result;
}

void UiFontUnload(UiFont* font)
{
    GuiSetFont(GetFontDefault());
    if (font->guiOwned) { UnloadFont(font->guiFont); }
    if (font->overlayOwned) { UnloadFont(font->overlayFont); }
    *font = {};
}

void UiDrawText(const UiFont& font, const char* text, Vector2 position, float size, Color color)
{
    DrawTextEx(font.overlayFont, text, position, size, 1.0f, color);
}
}