#include "MC6809.h"
#include "MC6809OpCodes.h"
#include "TC1014MMU.h"

#include "MC6809Macros.h"

MC6809State* InitializeInstance(MC6809State*);

//TODO: Startup doesn't initialize this instance in the expected order

static MC6809State* GetInstance();

static MC6809State* instance = GetInstance();

MC6809State* GetInstance() {
  return (instance ? instance : (instance = InitializeInstance(new MC6809State())));
}

extern "C" {
  __declspec(dllexport) MC6809State* __cdecl GetMC6809State() {
    return GetInstance();
  }
}

MC6809State* InitializeInstance(MC6809State* p) {
  p->InInterrupt = 0;
  p->IRQWaiter = 0;
  p->PendingInterrupts = 0;
  p->SyncWaiting = 0;
  p->CycleCounter = 0;

  return p;
}

extern "C" {
  __declspec(dllexport) void __cdecl mc6809_setcc(unsigned char bincc)
  {
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
  __declspec(dllexport) unsigned char __cdecl MC6809_getcc(void)
  {
    unsigned char bincc = 0;

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
  __declspec(dllexport) void __cdecl MC6809_cpu_firq(void)
  {
    if (!CC_F)
    {
      instance->InInterrupt = 1; //Flag to indicate FIRQ has been asserted
      CC_E = 0; // Turn E flag off

      MemWrite8(PC_L, --S_REG);
      MemWrite8(PC_H, --S_REG);
      MemWrite8(MC6809_getcc(), --S_REG);

      CC_I = 1;
      CC_F = 1;
      PC_REG = MemRead16(VFIRQ);
    }

    instance->PendingInterrupts &= 253;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MC6809_cpu_irq(void)
  {
    if (instance->InInterrupt == 1) { //If FIRQ is running postpone the IRQ
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
      MemWrite8(B_REG, --S_REG);
      MemWrite8(A_REG, --S_REG);
      MemWrite8(MC6809_getcc(), --S_REG);
      PC_REG = MemRead16(VIRQ);
      CC_I = 1;
    }

    instance->PendingInterrupts &= 254;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MC6809_cpu_nmi(void)
  {
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
    MemWrite8(B_REG, --S_REG);
    MemWrite8(A_REG, --S_REG);
    MemWrite8(MC6809_getcc(), --S_REG);

    CC_I = 1;
    CC_F = 1;
    PC_REG = MemRead16(VNMI);

    instance->PendingInterrupts &= 251;
  }
}

extern "C" {
  __declspec(dllexport) /* _inline */ unsigned short __cdecl MC6809_CalculateEA(unsigned char postbyte)
  {
    static unsigned short int ea = 0;
    static signed char byte = 0;
    static unsigned char reg;

    reg = ((postbyte >> 5) & 3) + 1;

    if (postbyte & 0x80)
    {
      switch (postbyte & 0x1F)
      {
      case 0:
        ea = PXF(reg);
        PXF(reg)++;
        instance->CycleCounter += 2;
        break;

      case 1:
        ea = PXF(reg);
        PXF(reg) += 2;
        instance->CycleCounter += 3;
        break;

      case 2:
        PXF(reg) -= 1;
        ea = PXF(reg);
        instance->CycleCounter += 2;
        break;

      case 3:
        PXF(reg) -= 2;
        ea = PXF(reg);
        instance->CycleCounter += 3;
        break;

      case 4:
        ea = PXF(reg);
        break;

      case 5:
        ea = PXF(reg) + ((signed char)B_REG);
        instance->CycleCounter += 1;
        break;

      case 6:
        ea = PXF(reg) + ((signed char)A_REG);
        instance->CycleCounter += 1;
        break;

      case 7:
        instance->CycleCounter += 1;
        break;

      case 8:
        ea = PXF(reg) + (signed char)MemRead8(PC_REG++);
        instance->CycleCounter += 1;
        break;

      case 9:
        ea = PXF(reg) + MemRead16(PC_REG);
        instance->CycleCounter += 4;
        PC_REG += 2;
        break;

      case 10:
        instance->CycleCounter += 1;
        break;

      case 11:
        ea = PXF(reg) + D_REG;
        instance->CycleCounter += 4;
        break;

      case 12:
        ea = (signed short)PC_REG + (signed char)MemRead8(PC_REG) + 1;
        instance->CycleCounter += 1;
        PC_REG++;
        break;

      case 13: //MM
        ea = PC_REG + MemRead16(PC_REG) + 2;
        instance->CycleCounter += 5;
        PC_REG += 2;
        break;

      case 14:
        instance->CycleCounter += 4;
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
        instance->CycleCounter += 6;
        break;

      case 18: //10010
        instance->CycleCounter += 6;
        break;

      case 19: //10011
        PXF(reg) -= 2;
        ea = PXF(reg);
        ea = MemRead16(ea);
        instance->CycleCounter += 6;
        break;

      case 20: //10100
        ea = PXF(reg);
        ea = MemRead16(ea);
        instance->CycleCounter += 3;
        break;

      case 21: //10101
        ea = PXF(reg) + ((signed char)B_REG);
        ea = MemRead16(ea);
        instance->CycleCounter += 4;
        break;

      case 22: //10110
        ea = PXF(reg) + ((signed char)A_REG);
        ea = MemRead16(ea);
        instance->CycleCounter += 4;
        break;

      case 23: //10111
        ea = MemRead16(ea);
        instance->CycleCounter += 4;
        break;

      case 24: //11000
        ea = PXF(reg) + (signed char)MemRead8(PC_REG++);
        ea = MemRead16(ea);
        instance->CycleCounter += 4;
        break;

      case 25: //11001
        ea = PXF(reg) + MemRead16(PC_REG);
        ea = MemRead16(ea);
        instance->CycleCounter += 7;
        PC_REG += 2;
        break;

      case 26: //11010
        ea = MemRead16(ea);
        instance->CycleCounter += 4;
        break;

      case 27: //11011
        ea = PXF(reg) + D_REG;
        ea = MemRead16(ea);
        instance->CycleCounter += 7;
        break;

      case 28: //11100
        ea = (signed short)PC_REG + (signed char)MemRead8(PC_REG) + 1;
        ea = MemRead16(ea);
        instance->CycleCounter += 4;
        PC_REG++;
        break;

      case 29: //11101
        ea = PC_REG + MemRead16(PC_REG) + 2;
        ea = MemRead16(ea);
        instance->CycleCounter += 8;
        PC_REG += 2;
        break;

      case 30: //11110
        ea = MemRead16(ea);
        instance->CycleCounter += 7;
        break;

      case 31: //11111
        ea = MemRead16(PC_REG);
        ea = MemRead16(ea);
        instance->CycleCounter += 8;
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

      instance->CycleCounter += 1;
    }

    return(ea);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MC6809Init(void)
  {	//Call this first or RESET will core!
    // reg pointers for TFR and EXG and LEA ops
    instance->xfreg16[0] = &D_REG;
    instance->xfreg16[1] = &X_REG;
    instance->xfreg16[2] = &Y_REG;
    instance->xfreg16[3] = &U_REG;
    instance->xfreg16[4] = &S_REG;
    instance->xfreg16[5] = &PC_REG;

    instance->ureg8[0] = (unsigned char*)(&A_REG);
    instance->ureg8[1] = (unsigned char*)(&B_REG);
    instance->ureg8[2] = (unsigned char*)(&(instance->ccbits));
    instance->ureg8[3] = (unsigned char*)(&DPA);
    instance->ureg8[4] = (unsigned char*)(&DPA);
    instance->ureg8[5] = (unsigned char*)(&DPA);
    instance->ureg8[6] = (unsigned char*)(&DPA);
    instance->ureg8[7] = (unsigned char*)(&DPA);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MC6809Reset(void)
  {
    char index;

    for (index = 0; index <= 5; index++) {		//Set all register to 0 except V
      PXF(index) = 0;
    }

    for (index = 0; index <= 7; index++) {
      PUR(index) = 0;
    }

    CC_E = 0;
    CC_F = 1;
    CC_H = 0;
    CC_I = 1;
    CC_N = 0;
    CC_Z = 0;
    CC_V = 0;
    CC_C = 0;

    DP_REG = 0;

    instance->SyncWaiting = 0;

    PC_REG = MemRead16(VRESET);	//PC gets its reset vector

    SetMapType(0);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MC6809AssertInterrupt(unsigned char interrupt, unsigned char waiter) // 4 nmi 2 firq 1 irq
  {
    instance->SyncWaiting = 0;
    instance->PendingInterrupts |= (1 << (interrupt - 1));
    instance->IRQWaiter = waiter;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MC6809DeAssertInterrupt(unsigned char interrupt) // 4 nmi 2 firq 1 irq
  {
    instance->PendingInterrupts &= ~(1 << (interrupt - 1));
    instance->InInterrupt = 0;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MC6809ForcePC(unsigned short newPC)
  {
    PC_REG = newPC;

    instance->PendingInterrupts = 0;
    instance->SyncWaiting = 0;
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl MC6809Exec(int cycleFor)
  {
    instance->CycleCounter = 0;

    while (instance->CycleCounter < cycleFor) {

      if (instance->PendingInterrupts)
      {
        if (instance->PendingInterrupts & 4) {
          MC6809_cpu_nmi();
        }

        if (instance->PendingInterrupts & 2) {
          MC6809_cpu_firq();
        }

        if (instance->PendingInterrupts & 1)
        {
          if (instance->IRQWaiter == 0) { // This is needed to fix a subtle timming problem
            MC6809_cpu_irq();		// It allows the CPU to see $FF03 bit 7 high before
          }
          else {				// The IRQ is asserted.
            instance->IRQWaiter -= 1;
          }
        }
      }

      if (instance->SyncWaiting == 1) {
        return(0);
      }

      unsigned char opCode = MemRead8(PC_REG++);

      MC6809ExecOpCode(cycleFor, opCode);
    }

    return(cycleFor - instance->CycleCounter);
  }
}