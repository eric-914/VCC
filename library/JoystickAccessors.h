#pragma once

extern "C" __declspec(dllexport) void __cdecl SetJoystick(unsigned short x, unsigned short y);
extern "C" __declspec(dllexport) void __cdecl SetStickNumbers(unsigned char leftStickNumber, unsigned char rightStickNumber);
extern "C" __declspec(dllexport) unsigned short __cdecl get_pot_value(unsigned char pot);
extern "C" __declspec(dllexport) void __cdecl SetButtonStatus(unsigned char side, unsigned char state);
extern "C" __declspec(dllexport) char __cdecl SetMouseStatus(char scanCode, unsigned char phase);
