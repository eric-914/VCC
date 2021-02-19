#pragma once

#include "windows.h"

#include "systemstate.h"

extern "C" __declspec(dllexport) POINT __cdecl GetCurrentWindowSize();
extern "C" __declspec(dllexport) void __cdecl CheckSurfaces();
extern "C" __declspec(dllexport) void __cdecl SetStatusBarText(char*, SystemState*);
extern "C" __declspec(dllexport) void __cdecl Cls(unsigned int, SystemState*);
extern "C" __declspec(dllexport) unsigned char __cdecl SetInfoBand(unsigned char);
extern "C" __declspec(dllexport) unsigned char __cdecl SetResize(unsigned char);
extern "C" __declspec(dllexport) unsigned char __cdecl SetAspect(unsigned char);
