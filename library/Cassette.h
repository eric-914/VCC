#pragma once

#include <windows.h>

#define STOP	0
#define PLAY	1
#define REC		2
#define EJECT	3
#define CAS	1
#define WAV 0
#define WRITEBUFFERSIZE	0x1FFFF

typedef struct
{
  HANDLE TapeHandle;

  char CassPath[MAX_PATH];
  char FileType;
  char TapeFileName[MAX_PATH];

  int LastTrans;

  unsigned char Byte;
  unsigned char LastSample;
  unsigned char Mask;
  unsigned char MotorState;
  unsigned char One[21];
  unsigned char Quiet;
  unsigned char TapeMode;
  unsigned char TempBuffer[8192];
  unsigned char WriteProtect;
  unsigned char Zero[40];

  unsigned char* CasBuffer;

  unsigned int TempIndex;

  unsigned long BytesMoved;
  unsigned long TapeOffset;
  unsigned long TotalSize;

} CassetteState;

extern "C" __declspec(dllexport) CassetteState * __cdecl GetCassetteState();

extern "C" __declspec(dllexport) int __cdecl MountTape(char*);

extern "C" __declspec(dllexport) unsigned int __cdecl GetTapeCounter();
extern "C" __declspec(dllexport) unsigned int __cdecl LoadTape();

extern "C" __declspec(dllexport) void __cdecl CastoWav(unsigned char*, unsigned int, unsigned long*);
extern "C" __declspec(dllexport) void __cdecl CloseTapeFile();
extern "C" __declspec(dllexport) void __cdecl FlushCassetteBuffer(unsigned char*, unsigned int);
extern "C" __declspec(dllexport) void __cdecl GetTapeName(char*);
extern "C" __declspec(dllexport) void __cdecl LoadCassetteBuffer(unsigned char*);
extern "C" __declspec(dllexport) void __cdecl Motor(unsigned char);
extern "C" __declspec(dllexport) void __cdecl SetTapeCounter(unsigned int);
extern "C" __declspec(dllexport) void __cdecl SetTapeMode(unsigned char);
extern "C" __declspec(dllexport) void __cdecl SyncFileBuffer();
extern "C" __declspec(dllexport) void __cdecl WavtoCas(unsigned char*, unsigned int);
