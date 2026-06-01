#pragma once

// Declares file-dialog scaling helpers

#include "ui/raygui_bridge.hpp"

namespace bvhview
{
void UiCenterFileDialog(GuiWindowFileDialogState* state);
void UiDrawFileDialog(GuiWindowFileDialogState* state);
void UiShutdownFileDialog();
}
