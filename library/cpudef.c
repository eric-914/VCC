#include <windows.h>

#include "cpudef.h"

CPU* InitializeInstance(CPU* cpu);

static CPU* instance = InitializeInstance(new CPU());

extern "C" {
  __declspec(dllexport) CPU* __cdecl GetCPU() {
    return instance;
  }
}

CPU* InitializeInstance(CPU* cpu) {
  cpu->CPUAssertInterrupt = NULL;
  cpu->CPUDeAssertInterrupt = NULL;
  cpu->CPUExec = NULL;
  cpu->CPUForcePC = NULL;
  cpu->CPUInit = NULL;
  cpu->CPUReset = NULL;

  return cpu;
}