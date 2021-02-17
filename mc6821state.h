#pragma once

#include <windows.h>

typedef struct {
  unsigned char LeftChannel;
  unsigned char RightChannel;
  unsigned char Asample;
  unsigned char Ssample;
  unsigned char Csample;
  unsigned char CartInserted;
  unsigned char CartAutoStart;
  unsigned char AddLF;

  unsigned char rega[4];
  unsigned char regb[4];
  unsigned char rega_dd[4];
  unsigned char regb_dd[4];

  HANDLE hPrintFile;
  HANDLE hOut;
  BOOL MonState;

} MC6821State;

extern "C" __declspec(dllexport) MC6821State * __cdecl GetMC6821State();