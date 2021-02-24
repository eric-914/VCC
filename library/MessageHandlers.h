#pragma once

#include <windows.h>

extern "C" __declspec(dllexport) void __cdecl CreateMainMenu(HWND hWnd);
extern "C" __declspec(dllexport) void __cdecl EmuExit();
extern "C" __declspec(dllexport) void __cdecl EmuReset(unsigned char state);
extern "C" __declspec(dllexport) void __cdecl EmuRun();
extern "C" __declspec(dllexport) void __cdecl HelpAbout(HWND hWnd);
extern "C" __declspec(dllexport) void __cdecl KeyDown(WPARAM wParam, LPARAM lParam);
extern "C" __declspec(dllexport) void __cdecl KeyUp(WPARAM wParam, LPARAM lParam);
extern "C" __declspec(dllexport) void __cdecl MouseMove(LPARAM lParam);
extern "C" __declspec(dllexport) void __cdecl ShowConfiguration();

extern "C" __declspec(dllexport) void __cdecl ToggleMonitorType();
extern "C" __declspec(dllexport) void __cdecl ToggleThrottle();
extern "C" __declspec(dllexport) void __cdecl ToggleFullScreen();
extern "C" __declspec(dllexport) void __cdecl ToggleOnOff();
extern "C" __declspec(dllexport) void __cdecl ToggleInfoBand();

extern "C" __declspec(dllexport) void __cdecl SlowDown();
extern "C" __declspec(dllexport) void __cdecl SpeedUp();
