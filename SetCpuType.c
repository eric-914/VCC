#include "vccstate.h"

unsigned char SetCpuType(unsigned char cpuType)
{
  VccState* vccState = GetVccState();

  switch (cpuType)
  {
  case 0:
    vccState->EmuState.CpuType = 0;

    strcpy(vccState->CpuName, "MC6809");

    break;

  case 1:
    vccState->EmuState.CpuType = 1;

    strcpy(vccState->CpuName, "HD6309");

    break;
  }

  return(vccState->EmuState.CpuType);
}
