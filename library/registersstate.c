#include "registersstate.h"

RegistersState* InitializeInstance(RegistersState*);

static RegistersState* instance = InitializeInstance(new RegistersState());

extern "C" {
  __declspec(dllexport) RegistersState* __cdecl GetRegistersState() {
    return instance;
  }
}

RegistersState* InitializeInstance(RegistersState* r) {
  r->EnhancedFIRQFlag = 0;
  r->EnhancedIRQFlag = 0;
  r->VDG_Mode = 0;
  r->Dis_Offset = 0;
  r->MPU_Rate = 0;
  r->LastIrq = 0;
  r->LastFirq = 0;
  r->VerticalOffsetRegister = 0;
  r->InterruptTimer = 0;

  return r;
}