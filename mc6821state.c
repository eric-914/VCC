#include "mc6821state.h"

const unsigned char rega[4] = { 0,0,0,0 };
const unsigned char regb[4] = { 0,0,0,0 };
const unsigned char rega_dd[4] = { 0,0,0,0 };
const unsigned char regb_dd[4] = { 0,0,0,0 };

MC6821State* InitializeInstance(MC6821State*);

static MC6821State* instance = InitializeInstance(new MC6821State());

extern "C" {
  __declspec(dllexport) MC6821State* __cdecl GetMC6821State() {
    return instance;
  }
}

MC6821State* InitializeInstance(MC6821State* s) {
  s->LeftChannel = 0;
  s->RightChannel = 0;
  s->Asample = 0;
  s->Ssample = 0;
  s->Csample = 0;
  s->CartInserted = 0;
  s->CartAutoStart = 1;
  s->AddLF = 0;

  s->hPrintFile = INVALID_HANDLE_VALUE;
  s->hOut = NULL;
  s->MonState = FALSE;

  for (int i = 0; i < sizeof(rega); i++) {
    s->rega[i] = rega[i];
  }

  for (int i = 0; i < sizeof(regb); i++) {
    s->regb[i] = regb[i];
  }

  for (int i = 0; i < sizeof(rega_dd); i++) {
    s->rega_dd[i] = rega_dd[i];
  }

  for (int i = 0; i < sizeof(regb_dd); i++) {
    s->regb_dd[i] = regb_dd[i];
  }

  return s;
}