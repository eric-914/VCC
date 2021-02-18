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

  return c;
}
