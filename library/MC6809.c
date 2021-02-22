#include "MC6809.h"

#include "MC6809Macros.h"

MC6809State* InitializeInstance(MC6809State*);

static MC6809State* instance; // = InitializeInstance(new MC6809State());

extern "C" {
  __declspec(dllexport) MC6809State* __cdecl GetMC6809State() {
    return (instance ? instance : (instance = InitializeInstance(new MC6809State())));
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
    MC6809State* mc6809State = GetMC6809State();

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
  __declspec(dllexport) unsigned char __cdecl mc6809_getcc(void)
  {
    unsigned char bincc = 0;

    MC6809State* mc6809State = GetMC6809State();

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
  __declspec(dllexport) void __cdecl MC6809Init(void)
  {	//Call this first or RESET will core!
    // reg pointers for TFR and EXG and LEA ops
    MC6809State* mc6809State = GetMC6809State();

    mc6809State->xfreg16[0] = &D_REG;
    mc6809State->xfreg16[1] = &X_REG;
    mc6809State->xfreg16[2] = &Y_REG;
    mc6809State->xfreg16[3] = &U_REG;
    mc6809State->xfreg16[4] = &S_REG;
    mc6809State->xfreg16[5] = &PC_REG;

    mc6809State->ureg8[0] = (unsigned char*)(&A_REG);
    mc6809State->ureg8[1] = (unsigned char*)(&B_REG);
    mc6809State->ureg8[2] = (unsigned char*)(&(mc6809State->ccbits));
    mc6809State->ureg8[3] = (unsigned char*)(&DPA);
    mc6809State->ureg8[4] = (unsigned char*)(&DPA);
    mc6809State->ureg8[5] = (unsigned char*)(&DPA);
    mc6809State->ureg8[6] = (unsigned char*)(&DPA);
    mc6809State->ureg8[7] = (unsigned char*)(&DPA);
  }
}