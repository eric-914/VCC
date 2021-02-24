#pragma once

#include <windows.h>

extern "C" __declspec(dllexport) void __cdecl ProcessCommandMessage(HWND, WPARAM);
extern "C" __declspec(dllexport) void __cdecl ProcessSysCommandMessage(HWND, WPARAM);

