#include "library/VCC.h"
#include "library/PAKInterface.h"

unsigned __stdcall CartLoad(void* dummy)
{
  VccState* vccState = GetVccState();

  LoadCart(&(vccState->EmuState));

  vccState->EmuState.EmulationRunning = TRUE;
  vccState->DialogOpen = false;

  return(NULL);
}
