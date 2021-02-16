#include "vccstate.h"

#include "SetClockSpeed.h"

void SetTurboMode(unsigned char data)
{
  VccState* vccState = GetVccState();

  vccState->EmuState.TurboSpeedFlag = (data & 1) + 1;

  SetClockSpeed(1);

  if (vccState->EmuState.DoubleSpeedFlag) {
    SetClockSpeed(vccState->EmuState.DoubleSpeedMultiplyer * vccState->EmuState.TurboSpeedFlag);
  }

  vccState->EmuState.CPUCurrentSpeed = .894;

  if (vccState->EmuState.DoubleSpeedFlag) {
    vccState->EmuState.CPUCurrentSpeed *= ((double)vccState->EmuState.DoubleSpeedMultiplyer * (double)vccState->EmuState.TurboSpeedFlag);
  }
}
