#pragma once

#include <windows.h>

extern "C" __declspec(dllexport) void __cdecl ProcessKeyDownMessage(WPARAM wParam, LPARAM lParam);
extern "C" __declspec(dllexport) void __cdecl ProcessSysKeyDownMessage(WPARAM wParam, LPARAM lParam);
