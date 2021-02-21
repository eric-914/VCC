#pragma once

extern "C" __declspec(dllexport) void __cdecl SetMmuRegister(unsigned char, unsigned char);
extern "C" __declspec(dllexport) void __cdecl SetMmuTask(unsigned char task);
extern "C" __declspec(dllexport) void __cdecl SetMapType(unsigned char type);
extern "C" __declspec(dllexport) void __cdecl SetRomMap(unsigned char data);
extern "C" __declspec(dllexport) void __cdecl SetVectors(unsigned char data);
