#pragma once

#include <windows.h>

extern "C" __declspec(dllexport) void __cdecl ProcessMessage(HWND, UINT, WPARAM, LPARAM);
