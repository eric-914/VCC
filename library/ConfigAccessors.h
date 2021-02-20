#pragma once

#include <windows.h>

extern "C" __declspec(dllexport) char* __cdecl AppDirectory();
extern "C" __declspec(dllexport) char* __cdecl BasicRomName(void);

extern "C" __declspec(dllexport) void __cdecl GetIniFilePath(char* path);
extern "C" __declspec(dllexport) void __cdecl SetIniFilePath(char* path);

extern "C" __declspec(dllexport) int __cdecl GetCurrentKeyboardLayout();
extern "C" __declspec(dllexport) int __cdecl GetPaletteType();
extern "C" __declspec(dllexport) int __cdecl GetRememberSize();

extern "C" __declspec(dllexport) POINT __cdecl GetIniWindowSize();
