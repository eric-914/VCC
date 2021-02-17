#include "hd6309state.h"

HD6309State* InitializeInstance(HD6309State*);

static HD6309State* instance = InitializeInstance(new HD6309State());

extern "C" {
  __declspec(dllexport) HD6309State* __cdecl GetHD6309State() {
    return instance;
  }
}

HD6309State* InitializeInstance(HD6309State* c) {

  return c;
}
