#pragma once

extern "C" __declspec(dllexport) unsigned char __cdecl GimeGetKeyboardInterruptState();
extern "C" __declspec(dllexport) void __cdecl GimeSetKeyboardInterruptState(unsigned char state);
extern "C" __declspec(dllexport) bool __cdecl GetPaste();
extern "C" __declspec(dllexport) void __cdecl SetPaste(bool flag);
