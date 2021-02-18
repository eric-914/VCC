#include "mc6809state.h"

MC6809State* InitializeInstance(MC6809State* chip);

static MC6809State* instance; // = InitializeInstance(new MC6809State());

extern "C" {
  __declspec(dllexport) MC6809State* __cdecl GetMC6809State() {
    //TODO: Not sure why the pattern won't initialize properly here.
    return (instance ? instance : (instance = InitializeInstance(new MC6809State())));
  }
}

MC6809State* InitializeInstance(MC6809State* c) {
  c->InInterrupt = 0;
  c->IRQWaiter = 0;
  c->PendingInterrupts = 0;
  c->SyncWaiting = 0;
  c->CycleCounter = 0;

  return c;
}
