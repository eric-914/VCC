#include "pakinterfacestate.h"

#include "UnloadDll.h"
#include "DynamicMenuCallback.h"
#include "SetCart.h"

#include "library/systemstate.h"

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
