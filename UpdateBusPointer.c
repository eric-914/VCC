#include <windows.h>

#include "pakinterfacestate.h"

#include "library/cpudef.h"

void UpdateBusPointer(void)
{
  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  if (pakInterfaceState->SetInterruptCallPointer != NULL) {
    pakInterfaceState->SetInterruptCallPointer(GetCPU()->CPUAssertInterrupt);
  }
}
