#include "vccstate.h"

#include "LoadCart.h"

unsigned __stdcall CartLoad(void* dummy)
{
  VccState* vccState = GetVccState();

  LoadCart(&(vccState->EmuState));

  vccState->EmuState.EmulationRunning = TRUE;
  vccState->DialogOpen = false;

  return(NULL);
}
