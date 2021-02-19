#include "library/vccstate.h"

#include "SetCPUMultiplayerFlag.h"

unsigned char SetCPUMultiplayer(unsigned char multiplayer)
{
  VccState* vccState = GetVccState();

  if (multiplayer != QUERY)
  {
    vccState->EmuState.DoubleSpeedMultiplyer = multiplayer;

    SetCPUMultiplayerFlag(vccState->EmuState.DoubleSpeedFlag);
  }

  return(vccState->EmuState.DoubleSpeedMultiplyer);
}
