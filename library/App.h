#pragma once

#include <windows.h>

extern "C" __declspec(dllexport) void __cdecl AppStartup(HINSTANCE hInstance, PSTR lpCmdLine, INT nCmdShow);
extern "C" __declspec(dllexport) void __cdecl AppRun();
extern "C" __declspec(dllexport) INT __cdecl AppShutdown();

extern "C" __declspec(dllexport) INT __cdecl AppExec(HINSTANCE hInstance, PSTR lpCmdLine, INT nCmdShow);
