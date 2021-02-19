#include <windows.h>

#include "library/pakinterfacestate.h"

#include "library/cpudef.h"

void UpdateBusPointer(void)
{
  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  if (pakInterfaceState->SetInterruptCallPointer != NULL) {
    pakInterfaceState->SetInterruptCallPointer(GetCPU()->CPUAssertInterrupt);
  }
}
