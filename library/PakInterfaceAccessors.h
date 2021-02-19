#pragma once

#include "systemstate.h"

extern "C" __declspec(dllexport) void __cdecl GetCurrentModule(char* defaultModule);
extern "C" __declspec(dllexport) int __cdecl FileID(char* filename);
extern "C" __declspec(dllexport) void __cdecl PakTimer(void);
extern "C" __declspec(dllexport) void __cdecl ResetBus(void);
extern "C" __declspec(dllexport) void __cdecl GetModuleStatus(SystemState* systemState);
extern "C" __declspec(dllexport) unsigned char __cdecl PakPortRead(unsigned char port);
extern "C" __declspec(dllexport) void __cdecl PakPortWrite(unsigned char port, unsigned char data);
extern "C" __declspec(dllexport) unsigned char __cdecl PakMem8Read(unsigned short address);
extern "C" __declspec(dllexport) void __cdecl PakMem8Write(unsigned char port, unsigned char data);
extern "C" __declspec(dllexport) unsigned short __cdecl PakAudioSample(void);
