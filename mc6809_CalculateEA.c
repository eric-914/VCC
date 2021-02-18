#include "mc6809state.h"
#include "MemRead8.h"
#include "MemRead16.h"

/* _inline */ unsigned short mc6809_CalculateEA(unsigned char postbyte)
{
  static unsigned short int ea = 0;
  static signed char byte = 0;
  static unsigned char reg;

  MC6809State* mc6809State = GetMC6809State();

  reg = ((postbyte >> 5) & 3) + 1;

  if (postbyte & 0x80)
  {
    switch (postbyte & 0x1F)
    {
    case 0:
      ea = PXF(reg);
      PXF(reg)++;
      mc6809State->CycleCounter += 2;
      break;

    case 1:
      ea = PXF(reg);
      PXF(reg) += 2;
      mc6809State->CycleCounter += 3;
      break;

    case 2:
      PXF(reg) -= 1;
      ea = PXF(reg);
      mc6809State->CycleCounter += 2;
      break;

    case 3:
      PXF(reg) -= 2;
      ea = PXF(reg);
      mc6809State->CycleCounter += 3;
      break;

    case 4:
      ea = PXF(reg);
      break;

    case 5:
      ea = PXF(reg) + ((signed char)B_REG);
      mc6809State->CycleCounter += 1;
      break;

    case 6:
      ea = PXF(reg) + ((signed char)A_REG);
      mc6809State->CycleCounter += 1;
      break;

    case 7:
      mc6809State->CycleCounter += 1;
      break;

    case 8:
      ea = PXF(reg) + (signed char)MemRead8(PC_REG++);
      mc6809State->CycleCounter += 1;
      break;

    case 9:
      ea = PXF(reg) + MemRead16(PC_REG);
      mc6809State->CycleCounter += 4;
      PC_REG += 2;
      break;

    case 10:
      mc6809State->CycleCounter += 1;
      break;

    case 11:
      ea = PXF(reg) + D_REG;
      mc6809State->CycleCounter += 4;
      break;

    case 12:
      ea = (signed short)PC_REG + (signed char)MemRead8(PC_REG) + 1;
      mc6809State->CycleCounter += 1;
      PC_REG++;
      break;

    case 13: //MM
      ea = PC_REG + MemRead16(PC_REG) + 2;
      mc6809State->CycleCounter += 5;
      PC_REG += 2;
      break;

    case 14:
      mc6809State->CycleCounter += 4;
      break;

    case 15: //01111
      byte = (postbyte >> 5) & 3;

      switch (byte)
      {
      case 0:
        break;

      case 1:
        PC_REG += 2;
        break;

      case 2:
        break;

      case 3:
        break;
      }
      break;

    case 16: //10000
      byte = (postbyte >> 5) & 3;

      switch (byte)
      {
      case 0:
        break;

      case 1:
        PC_REG += 2;
        break;

      case 2:
        break;

      case 3:
        break;
      }
      break;

    case 17: //10001
      ea = PXF(reg);
      PXF(reg) += 2;
      ea = MemRead16(ea);
      mc6809State->CycleCounter += 6;
      break;

    case 18: //10010
      mc6809State->CycleCounter += 6;
      break;

    case 19: //10011
      PXF(reg) -= 2;
      ea = PXF(reg);
      ea = MemRead16(ea);
      mc6809State->CycleCounter += 6;
      break;

    case 20: //10100
      ea = PXF(reg);
      ea = MemRead16(ea);
      mc6809State->CycleCounter += 3;
      break;

    case 21: //10101
      ea = PXF(reg) + ((signed char)B_REG);
      ea = MemRead16(ea);
      mc6809State->CycleCounter += 4;
      break;

    case 22: //10110
      ea = PXF(reg) + ((signed char)A_REG);
      ea = MemRead16(ea);
      mc6809State->CycleCounter += 4;
      break;

    case 23: //10111
      ea = MemRead16(ea);
      mc6809State->CycleCounter += 4;
      break;

    case 24: //11000
      ea = PXF(reg) + (signed char)MemRead8(PC_REG++);
      ea = MemRead16(ea);
      mc6809State->CycleCounter += 4;
      break;

    case 25: //11001
      ea = PXF(reg) + MemRead16(PC_REG);
      ea = MemRead16(ea);
      mc6809State->CycleCounter += 7;
      PC_REG += 2;
      break;

    case 26: //11010
      ea = MemRead16(ea);
      mc6809State->CycleCounter += 4;
      break;

    case 27: //11011
      ea = PXF(reg) + D_REG;
      ea = MemRead16(ea);
      mc6809State->CycleCounter += 7;
      break;

    case 28: //11100
      ea = (signed short)PC_REG + (signed char)MemRead8(PC_REG) + 1;
      ea = MemRead16(ea);
      mc6809State->CycleCounter += 4;
      PC_REG++;
      break;

    case 29: //11101
      ea = PC_REG + MemRead16(PC_REG) + 2;
      ea = MemRead16(ea);
      mc6809State->CycleCounter += 8;
      PC_REG += 2;
      break;

    case 30: //11110
      ea = MemRead16(ea);
      mc6809State->CycleCounter += 7;
      break;

    case 31: //11111
      ea = MemRead16(PC_REG);
      ea = MemRead16(ea);
      mc6809State->CycleCounter += 8;
      PC_REG += 2;
      break;
    }
  }
  else
  {
    byte = (postbyte & 31);
    byte = (byte << 3);
    byte = byte / 8;
    ea = PXF(reg) + byte; //Was signed

    mc6809State->CycleCounter += 1;
  }

  return(ea);
}
