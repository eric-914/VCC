#include "pakinterfacestate.h"

#include "UnloadDll.h"

#include "library/systemstate.h"

extern void SetCart(unsigned char);
extern void DynamicMenuCallback(SystemState* systemState, char* menuName, int menuId, int type);

void UnloadPack(SystemState* systemState)
{
  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  UnloadDll(systemState);

  strcpy(pakInterfaceState->DllPath, "");
  strcpy(pakInterfaceState->Modname, "Blank");

  pakInterfaceState->RomPackLoaded = false;

  SetCart(0);

  if (pakInterfaceState->ExternalRomBuffer != nullptr) {
    free(pakInterfaceState->ExternalRomBuffer);
  }

  pakInterfaceState->ExternalRomBuffer = nullptr;

  systemState->ResetPending = 2;

  DynamicMenuCallback(systemState, "", 0, 0); //Refresh Menus
  DynamicMenuCallback(systemState, "", 1, 0);
}
