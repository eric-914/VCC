#pragma once

#include <windows.h>

#define FALLING 0
#define RISING	1
#define ANY		  2

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

extern "C" __declspec(dllexport) void __cdecl SetCart(unsigned char);

extern "C" __declspec(dllexport) int __cdecl MC6821_OpenPrintFile(char*);

extern "C" __declspec(dllexport) unsigned char __cdecl MC6821_DACState();
extern "C" __declspec(dllexport) unsigned char __cdecl MC6821_GetCasSample();
extern "C" __declspec(dllexport) unsigned char __cdecl MC6821_GetMuxState();
extern "C" __declspec(dllexport) unsigned char __cdecl MC6821_SetCartAutoStart(unsigned char);
extern "C" __declspec(dllexport) unsigned char __cdecl MC6821_VDG_Mode();
extern "C" __declspec(dllexport) unsigned char __cdecl MC6821_pia0_read(unsigned char);
extern "C" __declspec(dllexport) unsigned char __cdecl MC6821_pia1_read(unsigned char);

extern "C" __declspec(dllexport) unsigned int __cdecl MC6821_GetDACSample();

extern "C" __declspec(dllexport) void __cdecl MC6821_AssertCart();
extern "C" __declspec(dllexport) void __cdecl MC6821_CaptureBit(unsigned char);
extern "C" __declspec(dllexport) void __cdecl MC6821_ClosePrintFile();
extern "C" __declspec(dllexport) void __cdecl MC6821_PiaReset();
extern "C" __declspec(dllexport) void __cdecl MC6821_SetCassetteSample(unsigned char);
extern "C" __declspec(dllexport) void __cdecl MC6821_SetMonState(BOOL);
extern "C" __declspec(dllexport) void __cdecl MC6821_SetSerialParams(unsigned char);
extern "C" __declspec(dllexport) void __cdecl MC6821_WritePrintMon(char*);
extern "C" __declspec(dllexport) void __cdecl MC6821_irq_fs(int);
extern "C" __declspec(dllexport) void __cdecl MC6821_irq_hs(int);
extern "C" __declspec(dllexport) void __cdecl MC6821_pia0_write(unsigned char, unsigned char);
extern "C" __declspec(dllexport) void __cdecl MC6821_pia1_write(unsigned char, unsigned char);
