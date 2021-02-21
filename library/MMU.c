//#include <stdio.h>

#include "MMU.h"

#include "macros.h"

const unsigned int MemConfig[4] = { 0x20000, 0x80000, 0x200000, 0x800000 };
const unsigned short RamMask[4] = { 15, 63, 255, 1023 };
const unsigned char StateSwitch[4] = { 8, 56, 56, 56 };
const unsigned char VectorMask[4] = { 15, 63, 63, 63 };
const unsigned char VectorMaska[4] = { 12, 60, 60, 60 };
const unsigned int VidMask[4] = { 0x1FFFF, 0x7FFFF, 0x1FFFFF, 0x7FFFFF };

MmuState* InitializeInstance(MmuState*);

static MmuState* instance = InitializeInstance(new MmuState());

extern "C" {
  __declspec(dllexport) MmuState* __cdecl GetMmuState() {
    return instance;
  }
}

MmuState* InitializeInstance(MmuState* p) {
  p->MmuState = 0;
  p->MmuTask = 0;
  p->MmuEnabled = 0;
  p->RamVectors = 0;
  p->RomMap = 0;
  p->MapType = 0;
  p->CurrentRamConfig = 1;
  p->MmuPrefix = 0;

  p->Memory = NULL;
  p->InternalRomBuffer = NULL;

  ARRAYCOPY(MemConfig);
  ARRAYCOPY(RamMask);
  ARRAYCOPY(StateSwitch);
  ARRAYCOPY(VectorMask);
  ARRAYCOPY(VectorMaska);
  ARRAYCOPY(VidMask);

  return p;
}

extern "C" {
  __declspec(dllexport) unsigned short __cdecl GetMem(long address) {
    MmuState* mmuState = GetMmuState();

    return(mmuState->Memory[address]);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char* __cdecl GetInternalRomPointer(void)
  {
    MmuState* mmuState = GetMmuState();

    return(mmuState->InternalRomBuffer);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetMmuEnabled(unsigned char usingmmu)
  {
    MmuState* mmuState = GetMmuState();

    mmuState->MmuEnabled = usingmmu;
    mmuState->MmuState = (!mmuState->MmuEnabled) << 1 | mmuState->MmuTask;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetMmuPrefix(unsigned char data)
  {
    MmuState* mmuState = GetMmuState();

    mmuState->MmuPrefix = (data & 3) << 8;
  }
}

