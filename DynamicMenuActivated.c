#include "pakinterfacestate.h"

#include "UnloadPack.h"
#include "LoadPack.h"

#include "library/systemstate.h"

void DynamicMenuActivated(SystemState* systemState, unsigned char menuItem)
{
  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  switch (menuItem)
  {
  case 1:
    LoadPack();
    break;

  case 2:
    UnloadPack(systemState);
    break;

  default:
    if (pakInterfaceState->ConfigModule != NULL) {
      pakInterfaceState->ConfigModule(menuItem);
    }

    break;
  }
}
