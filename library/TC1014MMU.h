#pragma once

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

} TC1014MmuState;

extern "C" __declspec(dllexport) TC1014MmuState * __cdecl GetTC1014MmuState();

extern "C" __declspec(dllexport) unsigned short __cdecl GetMem(long);
extern "C" __declspec(dllexport) unsigned char* __cdecl GetInternalRomPointer(void);
extern "C" __declspec(dllexport) void __cdecl SetMmuEnabled(unsigned char);
extern "C" __declspec(dllexport) void __cdecl SetMmuPrefix(unsigned char);
extern "C" __declspec(dllexport) void __cdecl UpdateMmuArray(void);
extern "C" __declspec(dllexport) void __cdecl SetVectors(unsigned char data);
extern "C" __declspec(dllexport) void __cdecl SetMmuRegister(unsigned char, unsigned char);
extern "C" __declspec(dllexport) void __cdecl SetMmuTask(unsigned char task);
extern "C" __declspec(dllexport) void __cdecl SetRomMap(unsigned char data);
extern "C" __declspec(dllexport) void __cdecl SetMapType(unsigned char type);

extern "C" __declspec(dllexport) void __cdecl MmuReset(void);
extern "C" __declspec(dllexport) void __cdecl SetDistoRamBank(unsigned char data);
extern "C" __declspec(dllexport) int __cdecl LoadInternalRom(char* filename);

extern "C" __declspec(dllexport) void __cdecl CopyRom(void);
extern "C" __declspec(dllexport) unsigned char* __cdecl MmuInit(unsigned char ramConfig);

extern "C" __declspec(dllexport) unsigned char __cdecl MemRead8(unsigned short address);
extern "C" __declspec(dllexport) unsigned short __cdecl MemRead16(unsigned short addr);
extern "C" __declspec(dllexport) unsigned int __cdecl MemRead32(unsigned short addr);
extern "C" __declspec(dllexport) void __cdecl MemWrite8(unsigned char data, unsigned short address);
extern "C" __declspec(dllexport) void __cdecl MemWrite16(unsigned short data, unsigned short addr);
extern "C" __declspec(dllexport) void __cdecl MemWrite32(unsigned int data, unsigned short addr);
