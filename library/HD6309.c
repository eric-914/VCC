#include "HD6309.h"
#include "MMU.h"

#include "HD6309Macros.h"

HD6309State* InitializeInstance(HD6309State*);

//TODO: Startup doesn't initialize this instance in the expected order

HD6309State* GetInstance();

static HD6309State* instance = GetInstance();

static unsigned char* NatEmuCycles[] =
{
  &(instance->NatEmuCycles65),
  &(instance->NatEmuCycles64),
  &(instance->NatEmuCycles32),
  &(instance->NatEmuCycles21),
  &(instance->NatEmuCycles54),
  &(instance->NatEmuCycles97),
  &(instance->NatEmuCycles85),
  &(instance->NatEmuCycles51),
  &(instance->NatEmuCycles31),
  &(instance->NatEmuCycles1110),
  &(instance->NatEmuCycles76),
  &(instance->NatEmuCycles75),
  &(instance->NatEmuCycles43),
  &(instance->NatEmuCycles87),
  &(instance->NatEmuCycles86),
  &(instance->NatEmuCycles98),
  &(instance->NatEmuCycles2726),
  &(instance->NatEmuCycles3635),
  &(instance->NatEmuCycles3029),
  &(instance->NatEmuCycles2827),
  &(instance->NatEmuCycles3726),
  &(instance->NatEmuCycles3130),
  &(instance->NatEmuCycles42),
  &(instance->NatEmuCycles53)
};

HD6309State* GetInstance() {
  return (instance ? instance : (instance = InitializeInstance(new HD6309State())));
}

extern "C" {
  __declspec(dllexport) HD6309State* __cdecl GetHD6309State() {
    return GetInstance();
  }
}

HD6309State* InitializeInstance(HD6309State* p) {
  p->InInterrupt = 0;
  p->CycleCounter = 0;
  p->SyncWaiting = 0;

  p->NatEmuCycles65 = 6;
  p->NatEmuCycles64 = 6;
  p->NatEmuCycles32 = 3;
  p->NatEmuCycles21 = 2;
  p->NatEmuCycles54 = 5;
  p->NatEmuCycles97 = 9;
  p->NatEmuCycles85 = 8;
  p->NatEmuCycles51 = 5;
  p->NatEmuCycles31 = 3;
  p->NatEmuCycles1110 = 11;
  p->NatEmuCycles76 = 7;
  p->NatEmuCycles75 = 7;
  p->NatEmuCycles43 = 4;
  p->NatEmuCycles87 = 8;
  p->NatEmuCycles86 = 8;
  p->NatEmuCycles98 = 9;
  p->NatEmuCycles2726 = 27;
  p->NatEmuCycles3635 = 36;
  p->NatEmuCycles3029 = 30;
  p->NatEmuCycles2827 = 28;
  p->NatEmuCycles3726 = 37;
  p->NatEmuCycles3130 = 31;
  p->NatEmuCycles42 = 4;
  p->NatEmuCycles53 = 5;

  p->IRQWaiter = 0;
  p->PendingInterrupts = 0;

  return p;
}

extern "C" {
  __declspec(dllexport) void __cdecl setcc(unsigned char bincc)
  {
    HD6309State* hd63096State = GetHD6309State();

    hd63096State->ccbits = bincc;

    CC_E = !!(bincc & (1 << E));
    CC_F = !!(bincc & (1 << F));
    CC_H = !!(bincc & (1 << H));
    CC_I = !!(bincc & (1 << I));
    CC_N = !!(bincc & (1 << N));
    CC_Z = !!(bincc & (1 << Z));
    CC_V = !!(bincc & (1 << V));
    CC_C = !!(bincc & (1 << C));
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl getcc(void)
  {
    unsigned char bincc = 0;

    HD6309State* hd63096State = GetHD6309State();

#define TST(_CC, _F) if (_CC) { bincc |= (1 << _F); }

    TST(CC_E, E);
    TST(CC_F, F);
    TST(CC_H, H);
    TST(CC_I, I);
    TST(CC_N, N);
    TST(CC_Z, Z);
    TST(CC_V, V);
    TST(CC_C, C);

    return(bincc);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl setmd(unsigned char binmd)
  {
    HD6309State* hd63096State = instance;

    MD_NATIVE6309 = !!(binmd & (1 << NATIVE6309));
    MD_FIRQMODE = !!(binmd & (1 << FIRQMODE));
    MD_UNDEFINED2 = !!(binmd & (1 << MD_UNDEF2));
    MD_UNDEFINED3 = !!(binmd & (1 << MD_UNDEF3));
    MD_UNDEFINED4 = !!(binmd & (1 << MD_UNDEF4));
    MD_UNDEFINED5 = !!(binmd & (1 << MD_UNDEF5));
    MD_ILLEGAL = !!(binmd & (1 << ILLEGAL));
    MD_ZERODIV = !!(binmd & (1 << ZERODIV));

    for (short i = 0; i < 24; i++)
    {
      *NatEmuCycles[i] = hd63096State->InsCycles[MD_NATIVE6309][i];
    }
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl getmd(void)
  {
    unsigned char binmd = 0;

    HD6309State* hd63096State = instance;

#define TSM(_MD, _F) if (_MD) { binmd |= (1 << _F); } //--Can't use the same macro name

    TSM(MD_NATIVE6309, NATIVE6309);
    TSM(MD_FIRQMODE, FIRQMODE);
    TSM(MD_UNDEFINED2, MD_UNDEF2);
    TSM(MD_UNDEFINED3, MD_UNDEF3);
    TSM(MD_UNDEFINED4, MD_UNDEF4);
    TSM(MD_UNDEFINED5, MD_UNDEF5);
    TSM(MD_ILLEGAL, ILLEGAL);
    TSM(MD_ZERODIV, ZERODIV);

    return(binmd);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl HD6309_cpu_firq(void)
  {
    HD6309State* hd63096State = GetHD6309State();

    if (!CC_F)
    {
      hd63096State->InInterrupt = 1; //Flag to indicate FIRQ has been asserted

      switch (MD_FIRQMODE)
      {
      case 0:
        CC_E = 0; // Turn E flag off

        MemWrite8(PC_L, --S_REG);
        MemWrite8(PC_H, --S_REG);
        MemWrite8(getcc(), --S_REG);

        CC_I = 1;
        CC_F = 1;
        PC_REG = MemRead16(VFIRQ);

        break;

      case 1:		//6309
        CC_E = 1;

        MemWrite8(PC_L, --S_REG);
        MemWrite8(PC_H, --S_REG);
        MemWrite8(U_L, --S_REG);
        MemWrite8(U_H, --S_REG);
        MemWrite8(Y_L, --S_REG);
        MemWrite8(Y_H, --S_REG);
        MemWrite8(X_L, --S_REG);
        MemWrite8(X_H, --S_REG);
        MemWrite8(DPA, --S_REG);

        if (MD_NATIVE6309)
        {
          MemWrite8((F_REG), --S_REG);
          MemWrite8((E_REG), --S_REG);
        }

        MemWrite8(B_REG, --S_REG);
        MemWrite8(A_REG, --S_REG);
        MemWrite8(getcc(), --S_REG);

        CC_I = 1;
        CC_F = 1;
        PC_REG = MemRead16(VFIRQ);

        break;
      }
    }

    hd63096State->PendingInterrupts &= 253;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl HD6309_cpu_irq(void)
  {
    HD6309State* hd63096State = GetHD6309State();

    if (hd63096State->InInterrupt == 1) { //If FIRQ is running postpone the IRQ
      return;
    }

    if (!CC_I) {
      CC_E = 1;

      MemWrite8(PC_L, --S_REG);
      MemWrite8(PC_H, --S_REG);
      MemWrite8(U_L, --S_REG);
      MemWrite8(U_H, --S_REG);
      MemWrite8(Y_L, --S_REG);
      MemWrite8(Y_H, --S_REG);
      MemWrite8(X_L, --S_REG);
      MemWrite8(X_H, --S_REG);
      MemWrite8(DPA, --S_REG);

      if (MD_NATIVE6309)
      {
        MemWrite8((F_REG), --S_REG);
        MemWrite8((E_REG), --S_REG);
      }

      MemWrite8(B_REG, --S_REG);
      MemWrite8(A_REG, --S_REG);
      MemWrite8(getcc(), --S_REG);

      PC_REG = MemRead16(VIRQ);
      CC_I = 1;
    }

    hd63096State->PendingInterrupts &= 254;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl HD6309_cpu_nmi(void)
  {
    HD6309State* hd63096State = GetHD6309State();

    CC_E = 1;

    MemWrite8(PC_L, --S_REG);
    MemWrite8(PC_H, --S_REG);
    MemWrite8(U_L, --S_REG);
    MemWrite8(U_H, --S_REG);
    MemWrite8(Y_L, --S_REG);
    MemWrite8(Y_H, --S_REG);
    MemWrite8(X_L, --S_REG);
    MemWrite8(X_H, --S_REG);
    MemWrite8(DPA, --S_REG);

    if (MD_NATIVE6309)
    {
      MemWrite8((F_REG), --S_REG);
      MemWrite8((E_REG), --S_REG);
    }

    MemWrite8(B_REG, --S_REG);
    MemWrite8(A_REG, --S_REG);
    MemWrite8(getcc(), --S_REG);

    CC_I = 1;
    CC_F = 1;
    PC_REG = MemRead16(VNMI);

    hd63096State->PendingInterrupts &= 251;
  }
}

extern "C" {
  __declspec(dllexport) unsigned short __cdecl hd6309_CalculateEA(unsigned char postbyte)
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
}

extern "C" {
  __declspec(dllexport) void __cdecl HD6309Init(void)
  {	//Call this first or RESET will core!
    // reg pointers for TFR and EXG and LEA ops
    HD6309State* hd63096State = GetHD6309State();

    hd63096State->xfreg16[0] = &D_REG;
    hd63096State->xfreg16[1] = &X_REG;
    hd63096State->xfreg16[2] = &Y_REG;
    hd63096State->xfreg16[3] = &U_REG;
    hd63096State->xfreg16[4] = &S_REG;
    hd63096State->xfreg16[5] = &PC_REG;
    hd63096State->xfreg16[6] = &W_REG;
    hd63096State->xfreg16[7] = &V_REG;

    hd63096State->ureg8[0] = (unsigned char*)&A_REG;
    hd63096State->ureg8[1] = (unsigned char*)&B_REG;
    hd63096State->ureg8[2] = (unsigned char*)&(hd63096State->ccbits);
    hd63096State->ureg8[3] = (unsigned char*)&DPA;
    hd63096State->ureg8[4] = (unsigned char*)&Z_H;
    hd63096State->ureg8[5] = (unsigned char*)&Z_L;
    hd63096State->ureg8[6] = (unsigned char*)&E_REG;
    hd63096State->ureg8[7] = (unsigned char*)&F_REG;

    hd63096State->NatEmuCycles[0] = &(hd63096State->NatEmuCycles65);
    hd63096State->NatEmuCycles[1] = &(hd63096State->NatEmuCycles64);
    hd63096State->NatEmuCycles[2] = &(hd63096State->NatEmuCycles32);
    hd63096State->NatEmuCycles[3] = &(hd63096State->NatEmuCycles21);
    hd63096State->NatEmuCycles[4] = &(hd63096State->NatEmuCycles54);
    hd63096State->NatEmuCycles[5] = &(hd63096State->NatEmuCycles97);
    hd63096State->NatEmuCycles[6] = &(hd63096State->NatEmuCycles85);
    hd63096State->NatEmuCycles[7] = &(hd63096State->NatEmuCycles51);
    hd63096State->NatEmuCycles[8] = &(hd63096State->NatEmuCycles31);
    hd63096State->NatEmuCycles[9] = &(hd63096State->NatEmuCycles1110);
    hd63096State->NatEmuCycles[10] = &(hd63096State->NatEmuCycles76);
    hd63096State->NatEmuCycles[11] = &(hd63096State->NatEmuCycles75);
    hd63096State->NatEmuCycles[12] = &(hd63096State->NatEmuCycles43);
    hd63096State->NatEmuCycles[13] = &(hd63096State->NatEmuCycles87);
    hd63096State->NatEmuCycles[14] = &(hd63096State->NatEmuCycles86);
    hd63096State->NatEmuCycles[15] = &(hd63096State->NatEmuCycles98);
    hd63096State->NatEmuCycles[16] = &(hd63096State->NatEmuCycles2726);
    hd63096State->NatEmuCycles[17] = &(hd63096State->NatEmuCycles3635);
    hd63096State->NatEmuCycles[18] = &(hd63096State->NatEmuCycles3029);
    hd63096State->NatEmuCycles[19] = &(hd63096State->NatEmuCycles2827);
    hd63096State->NatEmuCycles[20] = &(hd63096State->NatEmuCycles3726);
    hd63096State->NatEmuCycles[21] = &(hd63096State->NatEmuCycles3130);
    hd63096State->NatEmuCycles[22] = &(hd63096State->NatEmuCycles42);
    hd63096State->NatEmuCycles[23] = &(hd63096State->NatEmuCycles53);

    //This handles the disparity between 6309 and 6809 Instruction timing
    hd63096State->InsCycles[0][M65] = 6;	//6-5
    hd63096State->InsCycles[1][M65] = 5;
    hd63096State->InsCycles[0][M64] = 6;	//6-4
    hd63096State->InsCycles[1][M64] = 4;
    hd63096State->InsCycles[0][M32] = 3;	//3-2
    hd63096State->InsCycles[1][M32] = 2;
    hd63096State->InsCycles[0][M21] = 2;	//2-1
    hd63096State->InsCycles[1][M21] = 1;
    hd63096State->InsCycles[0][M54] = 5;	//5-4
    hd63096State->InsCycles[1][M54] = 4;
    hd63096State->InsCycles[0][M97] = 9;	//9-7
    hd63096State->InsCycles[1][M97] = 7;
    hd63096State->InsCycles[0][M85] = 8;	//8-5
    hd63096State->InsCycles[1][M85] = 5;
    hd63096State->InsCycles[0][M51] = 5;	//5-1
    hd63096State->InsCycles[1][M51] = 1;
    hd63096State->InsCycles[0][M31] = 3;	//3-1
    hd63096State->InsCycles[1][M31] = 1;
    hd63096State->InsCycles[0][M1110] = 11;	//11-10
    hd63096State->InsCycles[1][M1110] = 10;
    hd63096State->InsCycles[0][M76] = 7;	//7-6
    hd63096State->InsCycles[1][M76] = 6;
    hd63096State->InsCycles[0][M75] = 7;	//7-5
    hd63096State->InsCycles[1][M75] = 5;
    hd63096State->InsCycles[0][M43] = 4;	//4-3
    hd63096State->InsCycles[1][M43] = 3;
    hd63096State->InsCycles[0][M87] = 8;	//8-7
    hd63096State->InsCycles[1][M87] = 7;
    hd63096State->InsCycles[0][M86] = 8;	//8-6
    hd63096State->InsCycles[1][M86] = 6;
    hd63096State->InsCycles[0][M98] = 9;	//9-8
    hd63096State->InsCycles[1][M98] = 8;
    hd63096State->InsCycles[0][M2726] = 27;	//27-26
    hd63096State->InsCycles[1][M2726] = 26;
    hd63096State->InsCycles[0][M3635] = 36;	//36-25
    hd63096State->InsCycles[1][M3635] = 35;
    hd63096State->InsCycles[0][M3029] = 30;	//30-29
    hd63096State->InsCycles[1][M3029] = 29;
    hd63096State->InsCycles[0][M2827] = 28;	//28-27
    hd63096State->InsCycles[1][M2827] = 27;
    hd63096State->InsCycles[0][M3726] = 37;	//37-26
    hd63096State->InsCycles[1][M3726] = 26;
    hd63096State->InsCycles[0][M3130] = 31;	//31-30
    hd63096State->InsCycles[1][M3130] = 30;
    hd63096State->InsCycles[0][M42] = 4;	//4-2
    hd63096State->InsCycles[1][M42] = 2;
    hd63096State->InsCycles[0][M53] = 5;	//5-3
    hd63096State->InsCycles[1][M53] = 3;
  }
}
