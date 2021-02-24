#pragma once

#include "systemstate.h"

typedef struct
{
  double SoundInterrupt;
  double PicosToSoundSample;
  double CycleDrift;
  double CyclesThisLine;
  double PicosToInterrupt;
  double OldMaster;
  double MasterTickCounter;
  double UnxlatedTickCounter;
  double PicosThisLine;
  double CyclesPerSecord;
  double LinesPerSecond;
  double PicosPerLine;
  double CyclesPerLine;

  unsigned char SoundOutputMode;
  unsigned char HorzInterruptEnabled;
  unsigned char VertInterruptEnabled;
  unsigned char TopBorder;
  unsigned char BottomBorder;
  unsigned char LinesperScreen;
  unsigned char TimerInterruptEnabled;
  unsigned char BlinkPhase;

  unsigned short TimerClockRate;
  unsigned short SoundRate;
  unsigned short AudioIndex;

  unsigned int StateSwitch;

  int MasterTimer;
  int TimerCycleCount;
  int ClipCycle;
  int WaitCycle;
  int IntEnable;
  int SndEnable;
  int OverClock;

  char Throttle;

  unsigned int AudioBuffer[16384];
  unsigned char CassBuffer[8192];

  void (*AudioEvent)(void);
  void (*DrawTopBorder[4]) (SystemState*);
  void (*DrawBottomBorder[4]) (SystemState*);
  void (*UpdateScreen[4]) (SystemState*);
} CoCoState;

extern "C" __declspec(dllexport) CoCoState * __cdecl GetCoCoState();

extern "C" __declspec(dllexport) /* _inline */ int __cdecl CPUCycle();

extern "C" __declspec(dllexport) float __cdecl RenderFrame(SystemState*);

extern "C" __declspec(dllexport) unsigned char __cdecl SetSndOutMode(unsigned char);
extern "C" __declspec(dllexport) unsigned short __cdecl SetAudioRate(unsigned short);

extern "C" __declspec(dllexport) void __cdecl AudioOut();
extern "C" __declspec(dllexport) void __cdecl CassIn();
extern "C" __declspec(dllexport) void __cdecl CassOut();
extern "C" __declspec(dllexport) void __cdecl CocoReset();
extern "C" __declspec(dllexport) void __cdecl SetClockSpeed(unsigned short);
extern "C" __declspec(dllexport) void __cdecl SetHorzInterruptState(unsigned char);
extern "C" __declspec(dllexport) void __cdecl SetInterruptTimer(unsigned short);
extern "C" __declspec(dllexport) void __cdecl SetLinesperScreen(unsigned char);
extern "C" __declspec(dllexport) void __cdecl SetMasterTickCounter();
extern "C" __declspec(dllexport) void __cdecl SetTimerClockRate(unsigned char);
extern "C" __declspec(dllexport) void __cdecl SetTimerInterruptState(unsigned char);
extern "C" __declspec(dllexport) void __cdecl SetVertInterruptState(unsigned char);
