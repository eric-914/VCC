#pragma once

#include <windows.h>

#include "Audio.h"

typedef struct
{
  _LARGE_INTEGER StartTime;
  _LARGE_INTEGER EndTime;
  _LARGE_INTEGER OneFrame;
  _LARGE_INTEGER CurrentTime;
  _LARGE_INTEGER SleepRes;
  _LARGE_INTEGER TargetTime, OneMs;
  _LARGE_INTEGER MasterClock;
  _LARGE_INTEGER Now;

  unsigned char FrameSkip;
  float fMasterClock;
} ThrottleState;

extern "C" __declspec(dllexport) ThrottleState* __cdecl GetThrottleState();

extern "C" __declspec(dllexport) float __cdecl CalculateFPS(void);
extern "C" __declspec(dllexport) void __cdecl CalibrateThrottle(void);
extern "C" __declspec(dllexport) void __cdecl EndRender(unsigned char skip);
extern "C" __declspec(dllexport) void __cdecl FrameWait(void);
extern "C" __declspec(dllexport) void __cdecl StartRender(void);
