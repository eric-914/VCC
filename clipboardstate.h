#pragma once

using namespace std;

#include <string>

#include "library/STRConfig.h"

typedef struct
{
  bool CodePaste;
  bool PasteWithNew;
  int CurrentKeyMap;

  string ClipboardText;
  STRConfig ClipConfig;
} ClipboardState;

extern "C" __declspec(dllexport) ClipboardState * __cdecl GetClipboardState();