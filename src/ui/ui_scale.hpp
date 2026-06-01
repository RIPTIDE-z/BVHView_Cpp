#pragma once

// Declares 720p-based UI scaling helpers

#include "raylib.h"

namespace bvhview
{
void UiUpdateScale(int screenWidth, int screenHeight);
float UiScale();
float UiLogicalSize(float value);
int UiScaleInt(float value);
Rectangle UiRectangle(float x, float y, float width, float height);
void UiApplyRayguiScale();
void UiResetRayguiScale();
}
