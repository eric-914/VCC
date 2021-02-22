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

extern "C" __declspec(dllexport) bool __cdecl ClipboardEmpty();
extern "C" __declspec(dllexport) int __cdecl GetCurrentKeyMap();
extern "C" __declspec(dllexport) char __cdecl PeekClipboard();
extern "C" __declspec(dllexport) void __cdecl PopClipboard();
extern "C" __declspec(dllexport) bool __cdecl SetClipboard(const char*);

extern "C" __declspec(dllexport) void __cdecl PasteText();
extern "C" __declspec(dllexport) void __cdecl PasteBASIC();
extern "C" __declspec(dllexport) void __cdecl PasteBASICWithNew();

extern "C" __declspec(dllexport) void CopyText();
