#pragma once

#include <windows.h>

#include "defines.h"
#include "systemstate.h"
#include "commandline.h"

#define TH_RUNNING	0
#define TH_REQWAIT	1
#define TH_WAITING	2

//Type 0= HEAD TAG 1= Slave Tag 2=StandAlone
#define	HEAD 0
#define SLAVE 1
#define STANDALONE 2

typedef struct
{
  HANDLE hEMUThread;  // Message handlers

  char CpuName[20];
  char AppName[MAX_LOADSTRING];
  unsigned char FlagEmuStop;

  char QuickLoadFile[256];
  bool BinaryRunning;
  bool DialogOpen;
  unsigned char Throttle;
  unsigned char AutoStart;
  unsigned char Qflag;

  //--------------------------------------------------------------------------
  // When the main window is about to lose keyboard focus there are one
  // or two keys down in the emulation that must be raised.  These routines
  // track the last two key down events so they can be raised when needed.
  //--------------------------------------------------------------------------
  unsigned char SC_save1;
  unsigned char SC_save2;
  unsigned char KB_save1;
  unsigned char KB_save2;
  int KeySaveToggle;

  struct CmdLineArguments CmdArg;

  SystemState EmuState;
} VccState;

extern "C" __declspec(dllexport) VccState * __cdecl GetVccState();

extern "C" __declspec(dllexport) void __cdecl Reboot(void);
extern "C" __declspec(dllexport) void __cdecl SaveConfig(void);
extern "C" __declspec(dllexport) void __cdecl SaveLastTwoKeyDownEvents(unsigned char kb_char, unsigned char oemScan);
extern "C" __declspec(dllexport) unsigned char __cdecl SetAutoStart(unsigned char autostart);
extern "C" __declspec(dllexport) void __cdecl SetCPUMultiplayerFlag(unsigned char double_speed);
extern "C" __declspec(dllexport) unsigned char __cdecl SetCPUMultiplayer(unsigned char multiplayer);
extern "C" __declspec(dllexport) unsigned char __cdecl SetCpuType(unsigned char cpuType);
extern "C" __declspec(dllexport) unsigned char __cdecl SetFrameSkip(unsigned char skip);
extern "C" __declspec(dllexport) unsigned char __cdecl SetRamSize(unsigned char size);
extern "C" __declspec(dllexport) unsigned char __cdecl SetSpeedThrottle(unsigned char throttle);
extern "C" __declspec(dllexport) void __cdecl SetTurboMode(unsigned char data);

extern "C" __declspec(dllexport) unsigned __cdecl CartLoad(void* dummy);
extern "C" __declspec(dllexport) void __cdecl LoadPack(void);

extern "C" __declspec(dllexport) void __cdecl LoadIniFile(void);
extern "C" __declspec(dllexport) void __cdecl SendSavedKeyEvents();
