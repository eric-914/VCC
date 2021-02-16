#include "mmustate.h"

#include "port_write.h"

void MemWrite8(unsigned char data, unsigned short address)
{
  MmuState* mmuState = GetMmuState();

  if (address < 0xFE00)
  {
    if (mmuState->MapType || (mmuState->MmuRegisters[mmuState->MmuState][address >> 13] < mmuState->VectorMaska[mmuState->CurrentRamConfig]) || (mmuState->MmuRegisters[mmuState->MmuState][address >> 13] > mmuState->VectorMask[mmuState->CurrentRamConfig])) {
      mmuState->MemPages[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]][address & 0x1FFF] = data;
    }

    return;
  }

  if (address > 0xFEFF)
  {
    port_write(data, address);

    return;
  }

  if (mmuState->RamVectors) { //Address must be $FE00 - $FEFF
    mmuState->Memory[(0x2000 * mmuState->VectorMask[mmuState->CurrentRamConfig]) | (address & 0x1FFF)] = data;
  }
  else if (mmuState->MapType || (mmuState->MmuRegisters[mmuState->MmuState][address >> 13] < mmuState->VectorMaska[mmuState->CurrentRamConfig]) || (mmuState->MmuRegisters[mmuState->MmuState][address >> 13] > mmuState->VectorMask[mmuState->CurrentRamConfig])) {
    mmuState->MemPages[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]][address & 0x1FFF] = data;
  }
}
