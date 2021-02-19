#pragma once

extern "C" __declspec(dllexport) unsigned short __cdecl GetSoundStatus(void);
extern "C" __declspec(dllexport) unsigned char __cdecl PauseAudio(unsigned char pause);
extern "C" __declspec(dllexport) const char* __cdecl GetRateList(unsigned char index);
extern "C" __declspec(dllexport) int __cdecl SoundDeInit(void);