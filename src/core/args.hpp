#pragma once

// Declares command-line argument parsing functions

#include "raylib.h"

namespace bvhview
{
char* ArgFind(int argc, char** argv, const char* name);
float ArgFloat(int argc, char** argv, const char* name, float defaultValue);
int ArgInt(int argc, char** argv, const char* name, int defaultValue);
int ArgBool(int argc, char** argv, const char* name, bool defaultValue);
int ArgEnum(int argc, char** argv, const char* name, int optionCount, const char* options[], int defaultValue);
const char* ArgStr(int argc, char** argv, const char* name, const char* defaultValue);
Color ArgColor(int argc, char** argv, const char* name, Color defaultValue);
Vector3 ArgVector3(int argc, char** argv, const char* name, Vector3 defaultValue);
}
