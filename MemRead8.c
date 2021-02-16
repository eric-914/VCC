#include "mmustate.h"

#include "PakInterfaceAccessors.h"
#include "port_read.h"

unsigned char MemRead8(unsigned short address)
{
  MmuState* mmuState = GetMmuState();

  if (address < 0xFE00)
  {
    if (mmuState->MemPageOffsets[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]] == 1) {
      return(mmuState->MemPages[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]][address & 0x1FFF]);
    }

    return(PackMem8Read(mmuState->MemPageOffsets[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]] + (address & 0x1FFF)));
  }

  if (address > 0xFEFF) {
    return (port_read(address));
  }

  if (mmuState->RamVectors) { //Address must be $FE00 - $FEFF
    return(mmuState->Memory[(0x2000 * mmuState->VectorMask[mmuState->CurrentRamConfig]) | (address & 0x1FFF)]);
  }

  if (mmuState->MemPageOffsets[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]] == 1) {
    return(mmuState->MemPages[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]][address & 0x1FFF]);
  }

  return(PackMem8Read(mmuState->MemPageOffsets[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]] + (address & 0x1FFF)));
}
