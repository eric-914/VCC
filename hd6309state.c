#include "hd6309state.h"

HD6309State* InitializeInstance(HD6309State*);

static HD6309State* instance = InitializeInstance(new HD6309State());

extern "C" {
  __declspec(dllexport) HD6309State* __cdecl GetHD6309State() {
    //TODO: Not sure why the pattern won't initialize properly here. -- Assuming same reason as MC6809
    return (instance ? instance : (instance = InitializeInstance(new HD6309State())));
  }
}

HD6309State* InitializeInstance(HD6309State* c) {
  c->InInterrupt = 0;
  c->CycleCounter = 0;
  c->SyncWaiting = 0;

  c->NatEmuCycles65 = 6;
  c->NatEmuCycles64 = 6;
  c->NatEmuCycles32 = 3;
  c->NatEmuCycles21 = 2;
  c->NatEmuCycles54 = 5;
  c->NatEmuCycles97 = 9;
  c->NatEmuCycles85 = 8;
  c->NatEmuCycles51 = 5;
  c->NatEmuCycles31 = 3;
  c->NatEmuCycles1110 = 11;
  c->NatEmuCycles76 = 7;
  c->NatEmuCycles75 = 7;
  c->NatEmuCycles43 = 4;
  c->NatEmuCycles87 = 8;
  c->NatEmuCycles86 = 8;
  c->NatEmuCycles98 = 9;
  c->NatEmuCycles2726 = 27;
  c->NatEmuCycles3635 = 36;
  c->NatEmuCycles3029 = 30;
  c->NatEmuCycles2827 = 28;
  c->NatEmuCycles3726 = 37;
  c->NatEmuCycles3130 = 31;
  c->NatEmuCycles42 = 4;
  c->NatEmuCycles53 = 5;

  return c;
}
