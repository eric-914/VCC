#pragma once

#include <windows.h>

extern "C" __declspec(dllexport) void __cdecl ProcessCommandMessage(HWND hWnd, WPARAM wParam);
extern "C" __declspec(dllexport) void __cdecl ProcessSysCommandMessage(HWND hWnd, WPARAM wParam);

