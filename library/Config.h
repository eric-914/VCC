#pragma once

#include <richedit.h>

#include "SoundCardList.h"
#include "ConfigModel.h"
#include "defines.h"
#include "Joystick.h"
#include "SystemState.h"

#include "commandline.h"

#define SCAN_TRANS_COUNT	84
#define TABS 8
#define MAXCARDS 12

typedef struct
{
  CHARFORMAT CounterText;
  CHARFORMAT ModeText;

  HICON CpuIcons[2];
  HICON MonIcons[2];
  HICON JoystickIcons[4];

  HWND hDlgBar;
  HWND hDlgTape;

  JoyStick Left;
  JoyStick Right;

  short int Cpuchoice[2];
  short int Monchoice[2];
  short int PaletteChoice[2];

  unsigned short int Ramchoice[4];
  unsigned int LeftJoystickEmulation[3];
  unsigned int RightJoystickEmulation[3];

  ConfigModel CurrentConfig;
  ConfigModel TempConfig;

  TCHAR AppDataPath[MAX_PATH];

  char TextMode;
  char PrtMon;
  unsigned char NumberofJoysticks;

  char IniFilePath[MAX_PATH];
  char TapeFileName[MAX_PATH];
  char ExecDirectory[MAX_PATH];
  char SerialCaptureFile[MAX_PATH];
  char OutBuffer[MAX_PATH];
  char AppName[MAX_LOADSTRING];

  unsigned int TapeCounter;
  unsigned char Tmode;
  char Tmodes[4][10];
  int NumberOfSoundCards;

  SoundCardList SoundCards[MAXCARDS];
  HWND hWndConfig[TABS];

  unsigned char TranslateDisp2Scan[SCAN_TRANS_COUNT];
  unsigned char TranslateScan2Disp[SCAN_TRANS_COUNT];
} ConfigState;

extern "C" __declspec(dllexport) ConfigState * __cdecl GetConfigState();

extern "C" __declspec(dllexport) POINT __cdecl GetIniWindowSize();

extern "C" __declspec(dllexport) char* __cdecl AppDirectory();
extern "C" __declspec(dllexport) char* __cdecl BasicRomName(void);

extern "C" __declspec(dllexport) int __cdecl GetCurrentKeyboardLayout();
extern "C" __declspec(dllexport) int __cdecl GetPaletteType();
extern "C" __declspec(dllexport) int __cdecl GetRememberSize();
extern "C" __declspec(dllexport) int __cdecl SelectFile(SystemState*, char*);

extern "C" __declspec(dllexport) unsigned char __cdecl GetProfileByte(LPCSTR, LPCSTR, int);
extern "C" __declspec(dllexport) unsigned char __cdecl ReadIniFile(SystemState*);
extern "C" __declspec(dllexport) unsigned char __cdecl TranslateDisplay2Scan(LRESULT);
extern "C" __declspec(dllexport) unsigned char __cdecl TranslateScan2Display(int);
extern "C" __declspec(dllexport) unsigned char __cdecl WriteIniFile(void);

extern "C" __declspec(dllexport) unsigned short __cdecl GetProfileShort(LPCSTR, LPCSTR, int);

extern "C" __declspec(dllexport) void __cdecl BuildTransDisp2ScanTable();
extern "C" __declspec(dllexport) void __cdecl DecreaseOverclockSpeed(SystemState*);
extern "C" __declspec(dllexport) void __cdecl GetIniFilePath(char*);
extern "C" __declspec(dllexport) void __cdecl GetProfileText(LPCSTR, LPCSTR, LPCSTR, LPSTR);
extern "C" __declspec(dllexport) void __cdecl IncreaseOverclockSpeed(SystemState*);
extern "C" __declspec(dllexport) void __cdecl LoadConfig(SystemState*, CmdLineArguments);
extern "C" __declspec(dllexport) void __cdecl RefreshJoystickStatus();
extern "C" __declspec(dllexport) void __cdecl SetIniFilePath(char*);
extern "C" __declspec(dllexport) void __cdecl SetProfileText(LPCSTR, LPCSTR, LPCSTR);
extern "C" __declspec(dllexport) void __cdecl UpdateConfig(SystemState*);
extern "C" __declspec(dllexport) void __cdecl UpdateSoundBar(unsigned short, unsigned short);
extern "C" __declspec(dllexport) void __cdecl UpdateTapeCounter(unsigned int, unsigned char);
