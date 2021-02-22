#include "HD6309.h"

HD6309State* InitializeInstance(HD6309State*);

static HD6309State* instance;// = InitializeInstance(new HD6309State());

extern "C" {
  __declspec(dllexport) HD6309State* __cdecl GetHD6309State() {
    //TODO: Not sure why the pattern won't initialize properly here. -- Assuming same reason as MC6809
    return (instance ? instance : (instance = InitializeInstance(new HD6309State())));
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

  return p;
}
