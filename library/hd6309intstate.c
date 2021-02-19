#include "hd6309intstate.h"

HD6309IntState* InitializeInstance(HD6309IntState*);

static HD6309IntState* instance; // = InitializeInstance(new HD6309IntState());

extern "C" {
  __declspec(dllexport) HD6309IntState* __cdecl GetHD6309IntState() {
    return (instance ? instance : (instance = InitializeInstance(new HD6309IntState())));
  }
}

HD6309IntState* InitializeInstance(HD6309IntState* c) {
  c->IRQWaiter = 0;
  c->PendingInterrupts = 0;

  return c;
}