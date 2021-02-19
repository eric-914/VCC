#pragma once

extern "C" __declspec(dllexport) unsigned char __cdecl GimeRead(unsigned char port);
extern "C" __declspec(dllexport) void __cdecl GimeAssertKeyboardInterrupt(void);
extern "C" __declspec(dllexport) void __cdecl GimeAssertVertInterrupt(void);
extern "C" __declspec(dllexport) void __cdecl GimeAssertTimerInterrupt(void);
extern "C" __declspec(dllexport) void __cdecl GimeAssertHorzInterrupt(void);
