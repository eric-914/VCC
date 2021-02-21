#pragma once

#include <windows.h>

typedef struct {
  unsigned char MmuState;	// Composite variable handles MmuTask and MmuEnabled
  unsigned char* MemPages[1024];
  unsigned short MemPageOffsets[1024];
  unsigned char* Memory;	//Emulated RAM
  unsigned char* InternalRomBuffer;

  unsigned char MmuTask;		// $FF91 bit 0
  unsigned char MmuEnabled;	// $FF90 bit 6
  unsigned char RamVectors;	// $FF90 bit 3

  unsigned char RomMap;		  // $FF90 bit 1-0
  unsigned char MapType;		// $FFDE/FFDF toggle Map type 0 = ram/rom
  unsigned short MmuRegisters[4][8];	// $FFA0 - FFAF

  unsigned char CurrentRamConfig;
  unsigned short MmuPrefix;

  unsigned int MemConfig[4];
  unsigned short RamMask[4];
  unsigned char StateSwitch[4];
  unsigned char VectorMask[4];
  unsigned char VectorMaska[4];
  unsigned int VidMask[4];

} MmuState;

extern "C" __declspec(dllexport) MmuState * __cdecl GetMmuState();

extern "C" __declspec(dllexport) unsigned short __cdecl GetMem(long);
extern "C" __declspec(dllexport) unsigned char* __cdecl GetInternalRomPointer(void);
extern "C" __declspec(dllexport) void __cdecl SetMmuEnabled(unsigned char);
extern "C" __declspec(dllexport) void __cdecl SetMmuPrefix(unsigned char);
