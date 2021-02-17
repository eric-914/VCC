#include "mc6809state.h"

MC6809State* InitializeInstance(MC6809State*);

static MC6809State* instance = InitializeInstance(new MC6809State());

extern "C" {
  __declspec(dllexport) MC6809State* __cdecl GetMC6809State() {
    return instance;
  }
}

MC6809State* InitializeInstance(MC6809State* c) {

  return c;
}
