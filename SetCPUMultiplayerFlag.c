#include "library/vccstate.h"

#include "SetClockSpeed.h"

void SetCPUMultiplayerFlag(unsigned char double_speed)
{
  VccState* vccState = GetVccState();

  SetClockSpeed(1);

  vccState->EmuState.DoubleSpeedFlag = double_speed;

  if (vccState->EmuState.DoubleSpeedFlag) {
    SetClockSpeed(vccState->EmuState.DoubleSpeedMultiplyer * vccState->EmuState.TurboSpeedFlag);
  }

  vccState->EmuState.CPUCurrentSpeed = .894;

  if (vccState->EmuState.DoubleSpeedFlag) {
    vccState->EmuState.CPUCurrentSpeed *= ((double)vccState->EmuState.DoubleSpeedMultiplyer * (double)vccState->EmuState.TurboSpeedFlag);
  }
}
