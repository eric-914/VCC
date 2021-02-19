#include "vccdef.h"
#include "vccstate.h"

#include "defines.h"

const char CpuName[20] = "CPUNAME";
const char AppName[MAX_LOADSTRING] = "";
const unsigned char FlagEmuStop = TH_RUNNING;
const bool DialogOpen = false;
const unsigned char Throttle = 0;
const unsigned char AutoStart = 1;
const unsigned char Qflag = 0;
const unsigned char SC_save1 = 0;
const unsigned char SC_save2 = 0;
const unsigned char KB_save1 = 0;
const unsigned char KB_save2 = 0;
const int KeySaveToggle = 0;

VccState* InitializeInstance(VccState*);

static VccState* instance = InitializeInstance(new VccState());

extern "C" {
  __declspec(dllexport) VccState* __cdecl GetVccState() {
    return instance;
  }
}

VccState* InitializeInstance(VccState* v) {
  v->hEMUThread = NULL;
  v->DialogOpen = DialogOpen;
  v->Throttle = Throttle;
  v->AutoStart = AutoStart;
  v->Qflag = Qflag;
  v->FlagEmuStop = FlagEmuStop;
  v->SC_save1 = SC_save1;
  v->SC_save2 = SC_save2;
  v->KB_save1 = KB_save1;
  v->KB_save2 = KB_save2;
  v->KeySaveToggle = KeySaveToggle;

  strcpy(v->CpuName, CpuName);
  strcpy(v->AppName, AppName);

  return v;
}
