#pragma once

#include <windows.h>

extern "C" __declspec(dllexport) void __cdecl ProcessKeyDownMessage(WPARAM, LPARAM);
extern "C" __declspec(dllexport) void __cdecl ProcessSysKeyDownMessage(WPARAM, LPARAM);
