#pragma once

extern "C" __declspec(dllexport) unsigned short __cdecl GetMem(long);
extern "C" __declspec(dllexport) unsigned char* __cdecl GetInternalRomPointer(void);
extern "C" __declspec(dllexport) void __cdecl SetMmuEnabled(unsigned char);
extern "C" __declspec(dllexport) void __cdecl SetMmuPrefix(unsigned char);
extern "C" __declspec(dllexport) void __cdecl SetMmuRegister(unsigned char, unsigned char);
extern "C" __declspec(dllexport) void __cdecl SetMmuTask(unsigned char task);
extern "C" __declspec(dllexport) void __cdecl UpdateMmuArray(void);
extern "C" __declspec(dllexport) void __cdecl SetMapType(unsigned char type);
extern "C" __declspec(dllexport) void __cdecl SetRomMap(unsigned char data);
extern "C" __declspec(dllexport) void __cdecl SetVectors(unsigned char data);
