#include "library/registersstate.h"

#include "library/cpudef.h"
#include "library/defines.h"

unsigned char GimeRead(unsigned char port)
{
  static unsigned char temp;

  RegistersState* registersState = GetRegistersState();

  switch (port)
  {
  case 0x92:
    temp = registersState->LastIrq;
    registersState->LastIrq = 0;

    return(temp);

  case 0x93:
    temp = registersState->LastFirq;
    registersState->LastFirq = 0;

    return(temp);

  case 0x94:
  case 0x95:
    return(126);

  default:
    return(registersState->GimeRegisters[port]);
  }
}

void GimeAssertKeyboardInterrupt(void)
{
  RegistersState* registersState = GetRegistersState();

  CPU* cpu = GetCPU();

  if (((registersState->GimeRegisters[0x93] & 2) != 0) && (registersState->EnhancedFIRQFlag == 1)) {
    cpu->CPUAssertInterrupt(FIRQ, 0);

    registersState->LastFirq = registersState->LastFirq | 2;
  }
  else if (((registersState->GimeRegisters[0x92] & 2) != 0) && (registersState->EnhancedIRQFlag == 1)) {
    cpu->CPUAssertInterrupt(IRQ, 0);

    registersState->LastIrq = registersState->LastIrq | 2;
  }
}

void GimeAssertVertInterrupt(void)
{
  RegistersState* registersState = GetRegistersState();

  CPU* cpu = GetCPU();

  if (((registersState->GimeRegisters[0x93] & 8) != 0) && (registersState->EnhancedFIRQFlag == 1)) {
    cpu->CPUAssertInterrupt(FIRQ, 0); //FIRQ

    registersState->LastFirq = registersState->LastFirq | 8;
  }
  else if (((registersState->GimeRegisters[0x92] & 8) != 0) && (registersState->EnhancedIRQFlag == 1)) {
    cpu->CPUAssertInterrupt(IRQ, 0); //IRQ moon patrol demo using this

    registersState->LastIrq = registersState->LastIrq | 8;
  }
}

void GimeAssertTimerInterrupt(void)
{
  RegistersState* registersState = GetRegistersState();

  CPU* cpu = GetCPU();

  if (((registersState->GimeRegisters[0x93] & 32) != 0) && (registersState->EnhancedFIRQFlag == 1)) {
    cpu->CPUAssertInterrupt(FIRQ, 0);

    registersState->LastFirq = registersState->LastFirq | 32;
  }
  else if (((registersState->GimeRegisters[0x92] & 32) != 0) && (registersState->EnhancedIRQFlag == 1)) {
    cpu->CPUAssertInterrupt(IRQ, 0);

    registersState->LastIrq = registersState->LastIrq | 32;
  }
}

void GimeAssertHorzInterrupt(void)
{
  RegistersState* registersState = GetRegistersState();

  CPU* cpu = GetCPU();

  if (((registersState->GimeRegisters[0x93] & 16) != 0) && (registersState->EnhancedFIRQFlag == 1)) {
    cpu->CPUAssertInterrupt(FIRQ, 0);

    registersState->LastFirq = registersState->LastFirq | 16;
  }
  else if (((registersState->GimeRegisters[0x92] & 16) != 0) && (registersState->EnhancedIRQFlag == 1)) {
    cpu->CPUAssertInterrupt(IRQ, 0);

    registersState->LastIrq = registersState->LastIrq | 16;
  }
}
