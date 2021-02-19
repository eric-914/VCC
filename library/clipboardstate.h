#pragma once

using namespace std;

#include <string>

#include "ConfigModel.h"

typedef struct
{
  bool CodePaste;
  bool PasteWithNew;
  int CurrentKeyMap;

  string ClipboardText;
  ConfigModel ClipConfig;
} ClipboardState;

extern "C" __declspec(dllexport) ClipboardState * __cdecl GetClipboardState();