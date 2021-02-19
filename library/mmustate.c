#include <windows.h>

#include "mmustate.h"

#define SIZEOF(x)  (sizeof(x) / sizeof((x)[0]))

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

MmuState* InitializeInstance(MmuState* m) {
  m->MmuState = 0;
  m->MmuTask = 0;
  m->MmuEnabled = 0;
  m->RamVectors = 0;
  m->RomMap = 0;
  m->MapType = 0;
  m->CurrentRamConfig = 1;
  m->MmuPrefix = 0;

  m->Memory = NULL;
  m->InternalRomBuffer = NULL;

  for (int i = 0; i < SIZEOF(MemConfig); i++) {
    m->MemConfig[i] = MemConfig[i];
  }

  for (int i = 0; i < SIZEOF(RamMask); i++) {
    m->RamMask[i] = RamMask[i];
  }

  for (int i = 0; i < SIZEOF(StateSwitch); i++) {
    m->StateSwitch[i] = StateSwitch[i];
  }

  for (int i = 0; i < SIZEOF(VectorMask); i++) {
    m->VectorMask[i] = VectorMask[i];
  }

  for (int i = 0; i < SIZEOF(VectorMaska); i++) {
    m->VectorMaska[i] = VectorMaska[i];
  }

  for (int i = 0; i < SIZEOF(VidMask); i++) {
    m->VidMask[i] = VidMask[i];
  }

  return m;
}
