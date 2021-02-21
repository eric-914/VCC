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

extern "C" {
  __declspec(dllexport) void __cdecl UpdateMmuArray(void)
  {
    MmuState* mmuState = GetMmuState();

    if (mmuState->MapType) {
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 3] = mmuState->Memory + (0x2000 * (mmuState->VectorMask[mmuState->CurrentRamConfig] - 3));
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 2] = mmuState->Memory + (0x2000 * (mmuState->VectorMask[mmuState->CurrentRamConfig] - 2));
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 1] = mmuState->Memory + (0x2000 * (mmuState->VectorMask[mmuState->CurrentRamConfig] - 1));
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig]] = mmuState->Memory + (0x2000 * mmuState->VectorMask[mmuState->CurrentRamConfig]);

      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 3] = 1;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 2] = 1;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 1] = 1;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig]] = 1;

      return;
    }

    switch (mmuState->RomMap)
    {
    case 0:
    case 1:	//16K Internal 16K External
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 3] = mmuState->InternalRomBuffer;
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 2] = mmuState->InternalRomBuffer + 0x2000;
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 1] = NULL;
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig]] = NULL;

      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 3] = 1;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 2] = 1;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 1] = 0;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig]] = 0x2000;

      return;

    case 2:	// 32K Internal
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 3] = mmuState->InternalRomBuffer;
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 2] = mmuState->InternalRomBuffer + 0x2000;
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 1] = mmuState->InternalRomBuffer + 0x4000;
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig]] = mmuState->InternalRomBuffer + 0x6000;

      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 3] = 1;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 2] = 1;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 1] = 1;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig]] = 1;

      return;

    case 3:	//32K External
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 1] = NULL;
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig]] = NULL;
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 3] = NULL;
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 2] = NULL;

      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 1] = 0;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig]] = 0x2000;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 3] = 0x4000;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 2] = 0x6000;

      return;
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetVectors(unsigned char data)
  {
    MmuState* mmuState = GetMmuState();

    mmuState->RamVectors = !!data; //Bit 3 of $FF90 MC3
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetMmuRegister(unsigned char Register, unsigned char data)
  {
    unsigned char BankRegister, Task;

    MmuState* mmuState = GetMmuState();

    BankRegister = Register & 7;
    Task = !!(Register & 8);

    mmuState->MmuRegisters[Task][BankRegister] = mmuState->MmuPrefix | (data & mmuState->RamMask[mmuState->CurrentRamConfig]); //gime.c returns what was written so I can get away with this
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetMmuTask(unsigned char task)
  {
    MmuState* mmuState = GetMmuState();

    mmuState->MmuTask = task;
    mmuState->MmuState = (!mmuState->MmuEnabled) << 1 | mmuState->MmuTask;
  }
}
