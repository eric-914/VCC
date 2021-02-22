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

extern "C" __declspec(dllexport) unsigned int __cdecl GetDACSample(void);
extern "C" __declspec(dllexport) void __cdecl AssertCart(void);
extern "C" __declspec(dllexport) void __cdecl ClosePrintFile(void);
extern "C" __declspec(dllexport) unsigned char __cdecl DACState(void);
extern "C" __declspec(dllexport) unsigned char __cdecl GetCasSample(void);
extern "C" __declspec(dllexport) unsigned char __cdecl GetMuxState(void);
extern "C" __declspec(dllexport) void __cdecl irq_fs(int phase);
extern "C" __declspec(dllexport) void __cdecl irq_hs(int phase);
extern "C" __declspec(dllexport) int __cdecl OpenPrintFile(char* filename);
extern "C" __declspec(dllexport) void __cdecl PiaReset();
extern "C" __declspec(dllexport) void __cdecl SetCart(unsigned char cart);
extern "C" __declspec(dllexport) unsigned char __cdecl SetCartAutoStart(unsigned char autostart);
extern "C" __declspec(dllexport) void __cdecl SetCassetteSample(unsigned char sample);
extern "C" __declspec(dllexport) void __cdecl SetMonState(BOOL state);
extern "C" __declspec(dllexport) void __cdecl SetSerialParams(unsigned char textMode);
extern "C" __declspec(dllexport) unsigned char __cdecl VDG_Mode(void);
extern "C" __declspec(dllexport) void __cdecl WritePrintMon(char* data);
extern "C" __declspec(dllexport) void __cdecl CaptureBit(unsigned char sample);

extern "C" __declspec(dllexport) unsigned char __cdecl pia0_read(unsigned char port);
extern "C" __declspec(dllexport) void __cdecl pia0_write(unsigned char data, unsigned char port);
extern "C" __declspec(dllexport) unsigned char __cdecl pia1_read(unsigned char port);
extern "C" __declspec(dllexport) void __cdecl pia1_write(unsigned char data, unsigned char port);
