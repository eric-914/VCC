#include "library/HD6309.h"
#include "library/MMU.h"

unsigned short hd6309_CalculateEA(unsigned char postbyte)
{
  static unsigned short int ea = 0;
  static signed char byte = 0;
  static unsigned char reg;

  HD6309State* hd63096State = GetHD6309State();

  reg = ((postbyte >> 5) & 3) + 1;

  if (postbyte & 0x80)
  {
    switch (postbyte & 0x1F)
    {
    case 0: // Post inc by 1
      ea = PXF(reg);

      PXF(reg)++;

      hd63096State->CycleCounter += hd63096State->NatEmuCycles21;

      break;

    case 1: // post in by 2
      ea = PXF(reg);

      PXF(reg) += 2;

      hd63096State->CycleCounter += hd63096State->NatEmuCycles32;

      break;

    case 2: // pre dec by 1
      PXF(reg) -= 1;

      ea = PXF(reg);

      hd63096State->CycleCounter += hd63096State->NatEmuCycles21;

      break;

    case 3: // pre dec by 2
      PXF(reg) -= 2;

      ea = PXF(reg);

      hd63096State->CycleCounter += hd63096State->NatEmuCycles32;

      break;

    case 4: // no offset
      ea = PXF(reg);

      break;

    case 5: // B reg offset
      ea = PXF(reg) + ((signed char)B_REG);

      hd63096State->CycleCounter += 1;

      break;

    case 6: // A reg offset
      ea = PXF(reg) + ((signed char)A_REG);

      hd63096State->CycleCounter += 1;

      break;

    case 7: // E reg offset 
      ea = PXF(reg) + ((signed char)E_REG);

      hd63096State->CycleCounter += 1;

      break;

    case 8: // 8 bit offset
      ea = PXF(reg) + (signed char)MemRead8(PC_REG++);

      hd63096State->CycleCounter += 1;

      break;

    case 9: // 16 bit offset
      ea = PXF(reg) + IMMADDRESS(PC_REG);

      hd63096State->CycleCounter += hd63096State->NatEmuCycles43;

      PC_REG += 2;

      break;

    case 10: // F reg offset
      ea = PXF(reg) + ((signed char)F_REG);

      hd63096State->CycleCounter += 1;

      break;

    case 11: // D reg offset 
      ea = PXF(reg) + D_REG; //Changed to unsigned 03/14/2005 NG Was signed

      hd63096State->CycleCounter += hd63096State->NatEmuCycles42;

      break;

    case 12: // 8 bit PC relative
      ea = (signed short)PC_REG + (signed char)MemRead8(PC_REG) + 1;

      hd63096State->CycleCounter += 1;

      PC_REG++;

      break;

    case 13: // 16 bit PC relative
      ea = PC_REG + IMMADDRESS(PC_REG) + 2;

      hd63096State->CycleCounter += hd63096State->NatEmuCycles53;

      PC_REG += 2;

      break;

    case 14: // W reg offset
      ea = PXF(reg) + W_REG;

      hd63096State->CycleCounter += 4;

      break;

    case 15: // W reg
      byte = (postbyte >> 5) & 3;

      switch (byte)
      {
      case 0: // No offset from W reg
        ea = W_REG;

        break;

      case 1: // 16 bit offset from W reg
        ea = W_REG + IMMADDRESS(PC_REG);

        PC_REG += 2;

        hd63096State->CycleCounter += 2;

        break;

      case 2: // Post inc by 2 from W reg
        ea = W_REG;

        W_REG += 2;

        hd63096State->CycleCounter += 1;

        break;

      case 3: // Pre dec by 2 from W reg
        W_REG -= 2;

        ea = W_REG;

        hd63096State->CycleCounter += 1;

        break;
      }

      break;

    case 16: // W reg
      byte = (postbyte >> 5) & 3;

      switch (byte)
      {
      case 0: // Indirect no offset from W reg
        ea = MemRead16(W_REG);

        hd63096State->CycleCounter += 3;

        break;

      case 1: // Indirect 16 bit offset from W reg
        ea = MemRead16(W_REG + IMMADDRESS(PC_REG));

        PC_REG += 2;

        hd63096State->CycleCounter += 5;

        break;

      case 2: // Indirect post inc by 2 from W reg
        ea = MemRead16(W_REG);

        W_REG += 2;

        hd63096State->CycleCounter += 4;

        break;

      case 3: // Indirect pre dec by 2 from W reg
        W_REG -= 2;

        ea = MemRead16(W_REG);

        hd63096State->CycleCounter += 4;

        break;
      }
      break;

    case 17: // Indirect post inc by 2 
      ea = PXF(reg);

      PXF(reg) += 2;

      ea = MemRead16(ea);

      hd63096State->CycleCounter += 6;

      break;

    case 18: // possibly illegal instruction
      hd63096State->CycleCounter += 6;

      break;

    case 19: // Indirect pre dec by 2
      PXF(reg) -= 2;

      ea = MemRead16(PXF(reg));

      hd63096State->CycleCounter += 6;

      break;

    case 20: // Indirect no offset 
      ea = MemRead16(PXF(reg));

      hd63096State->CycleCounter += 3;

      break;

    case 21: // Indirect B reg offset
      ea = MemRead16(PXF(reg) + ((signed char)B_REG));

      hd63096State->CycleCounter += 4;

      break;

    case 22: // indirect A reg offset
      ea = MemRead16(PXF(reg) + ((signed char)A_REG));

      hd63096State->CycleCounter += 4;

      break;

    case 23: // indirect E reg offset
      ea = MemRead16(PXF(reg) + ((signed char)E_REG));

      hd63096State->CycleCounter += 4;

      break;

    case 24: // indirect 8 bit offset
      ea = MemRead16(PXF(reg) + (signed char)MemRead8(PC_REG++));

      hd63096State->CycleCounter += 4;

      break;

    case 25: // indirect 16 bit offset
      ea = MemRead16(PXF(reg) + IMMADDRESS(PC_REG));

      hd63096State->CycleCounter += 7;

      PC_REG += 2;

      break;

    case 26: // indirect F reg offset
      ea = MemRead16(PXF(reg) + ((signed char)F_REG));

      hd63096State->CycleCounter += 4;

      break;

    case 27: // indirect D reg offset
      ea = MemRead16(PXF(reg) + D_REG);

      hd63096State->CycleCounter += 7;

      break;

    case 28: // indirect 8 bit PC relative
      ea = MemRead16((signed short)PC_REG + (signed char)MemRead8(PC_REG) + 1);

      hd63096State->CycleCounter += 4;

      PC_REG++;

      break;

    case 29: //indirect 16 bit PC relative
      ea = MemRead16(PC_REG + IMMADDRESS(PC_REG) + 2);

      hd63096State->CycleCounter += 8;

      PC_REG += 2;

      break;

    case 30: // indirect W reg offset
      ea = MemRead16(PXF(reg) + W_REG);

      hd63096State->CycleCounter += 7;

      break;

    case 31: // extended indirect
      ea = MemRead16(IMMADDRESS(PC_REG));

      hd63096State->CycleCounter += 8;

      PC_REG += 2;

      break;
    }
  }
  else // 5 bit offset
  {
    byte = (postbyte & 31);
    byte = (byte << 3);
    byte = byte / 8;

    ea = PXF(reg) + byte; //Was signed

    hd63096State->CycleCounter += 1;
  }

  return(ea);
}
