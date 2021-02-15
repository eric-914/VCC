#include <stdint.h>

#include "pakinterfacestate.h"

const unsigned int BankedCartOffset = 0;
const char DllPath[256] = "";
const unsigned short ModualParms = 0;
const bool DialogOpen = false;
const bool RomPackLoaded = false;
const unsigned char MenuIndex = 0;
const char Modname[MAX_PATH] = "Blank";

PakInterfaceState* InitializeInstance(PakInterfaceState* pakInterfaceState);

static PakInterfaceState* instance = InitializeInstance(new PakInterfaceState());

extern "C" {
  __declspec(dllexport) PakInterfaceState* __cdecl GetPakInterfaceState() {
    return instance;
  }
}

PakInterfaceState* InitializeInstance(PakInterfaceState* p) {
  p->BankedCartOffset = BankedCartOffset;
  p->ModualParms = ModualParms;
  p->DialogOpen = DialogOpen;
  p->RomPackLoaded = RomPackLoaded;
  p->MenuIndex = MenuIndex;

  strcpy(p->DllPath, DllPath);
  strcpy(p->Modname, Modname);

  p->ExternalRomBuffer = nullptr;
  p->hMenu = NULL;

  p->GetModuleName = NULL;
  p->ConfigModule = NULL;
  p->SetInterruptCallPointer  = NULL;
  p->DmaMemPointer  = NULL;
  p->HeartBeat = NULL;
  p->PakPortWrite = NULL;
  p->PakPortRead = NULL;
  p->PakMemWrite8 = NULL;
  p->PakMemRead8 = NULL;
  p->ModuleStatus = NULL;
  p->ModuleAudioSample = NULL;
  p->ModuleReset = NULL;
  p->SetIniPath = NULL;
  p->PakSetCart = NULL;

  return p;
}