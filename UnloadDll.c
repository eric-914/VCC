#include "library/pakinterfacestate.h"

#include "library/systemstate.h"

#include "DynamicMenuCallback.h"

void UnloadDll(SystemState* systemState)
{
  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  if ((pakInterfaceState->DialogOpen == true) && (systemState->EmulationRunning == 1))
  {
    MessageBox(0, "Close Configuration Dialog before unloading", "Ok", 0);

    return;
  }

  pakInterfaceState->GetModuleName = NULL;
  pakInterfaceState->ConfigModule = NULL;
  pakInterfaceState->PakPortWrite = NULL;
  pakInterfaceState->PakPortRead = NULL;
  pakInterfaceState->SetInterruptCallPointer = NULL;
  pakInterfaceState->DmaMemPointer = NULL;
  pakInterfaceState->HeartBeat = NULL;
  pakInterfaceState->PakMemWrite8 = NULL;
  pakInterfaceState->PakMemRead8 = NULL;
  pakInterfaceState->ModuleStatus = NULL;
  pakInterfaceState->ModuleAudioSample = NULL;
  pakInterfaceState->ModuleReset = NULL;

  if (pakInterfaceState->hInstLib != NULL) {
    FreeLibrary(pakInterfaceState->hInstLib);
  }

  pakInterfaceState->hInstLib = NULL;

  DynamicMenuCallback(systemState, "", 0, 0); //Refresh Menus
  DynamicMenuCallback(systemState, "", 1, 0);
}
