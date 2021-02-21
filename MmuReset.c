#include "library/MMU.h"

void MmuReset(void)
{
  unsigned int index1 = 0, index2 = 0;

  MmuState* mmuState = GetMmuState();

  mmuState->MmuTask = 0;
  mmuState->MmuEnabled = 0;
  mmuState->RamVectors = 0;
  mmuState->MmuState = 0;
  mmuState->RomMap = 0;
  mmuState->MapType = 0;
  mmuState->MmuPrefix = 0;

  for (index1 = 0;index1 < 8;index1++) {
    for (index2 = 0;index2 < 4;index2++) {
      mmuState->MmuRegisters[index2][index1] = index1 + mmuState->StateSwitch[mmuState->CurrentRamConfig];
    }
  }

  for (index1 = 0;index1 < 1024;index1++)
  {
    mmuState->MemPages[index1] = mmuState->Memory + ((index1 & mmuState->RamMask[mmuState->CurrentRamConfig]) * 0x2000);
    mmuState->MemPageOffsets[index1] = 1;
  }

  SetRomMap(0);
  SetMapType(0);
}
