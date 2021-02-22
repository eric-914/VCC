#include "MC6809.h"

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
