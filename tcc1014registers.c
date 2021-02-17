/*
Copyright 2015 by Joseph Forgione
This file is part of VCC (Virtual Color Computer).

    VCC (Virtual Color Computer) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    VCC (Virtual Color Computer) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with VCC (Virtual Color Computer).  If not, see <http://www.gnu.org/licenses/>.
*/

#include <windows.h>

#include "registersstate.h"

#include "SetHorzInterruptState.h"
#include "SetTimerInterruptState.h"
#include "SetInterruptTimer.h"
#include "SetVertInterruptState.h"
#include "SetTimerClockRate.h"
#include "KeyboardAccessors.h"
#include "SetTurboMode.h"
#include "SetCPUMultiplayerFlag.h"
#include "SetGimeVdgOffset.h"
#include "SetGimeVdgMode.h"
#include "SetVerticalOffsetRegister.h"
#include "SetCompatMode.h"
#include "SetGimeVmode.h"
#include "SetGimeVres.h"
#include "SetGimeHorzOffset.h"
#include "SetGimeBorderColor.h"
#include "SetDistoRamBank.h"
#include "MmuAccessors.h"

#include "library/cpudef.h"
#include "library/defines.h"
#include "library/graphicsstate.h"

void SetInit0(unsigned char);
void SetInit1(unsigned char);
void SetGimeIRQStearing(unsigned char);
void SetGimeFIRQStearing(unsigned char);
void SetTimerMSB(unsigned char);
void SetTimerLSB(unsigned char);
unsigned char GetInit0(unsigned char port);

void GimeWrite(unsigned char port, unsigned char data)
{
  RegistersState* registersState = GetRegistersState();

  registersState->GimeRegisters[port] = data;

  switch (port)
  {
  case 0x90:
    SetInit0(data);
    break;

  case 0x91:
    SetInit1(data);
    break;

  case 0x92:
    SetGimeIRQStearing(data);
    break;

  case 0x93:
    SetGimeFIRQStearing(data);
    break;

  case 0x94:
    SetTimerMSB(data);
    break;

  case 0x95:
    SetTimerLSB(data);
    break;

  case 0x96:
    SetTurboMode(data & 1);
    break;

  case 0x97:
    break;

  case 0x98:
    SetGimeVmode(data);
    break;

  case 0x99:
    SetGimeVres(data);
    break;

  case 0x9A:
    SetGimeBorderColor(data);
    break;

  case 0x9B:
    SetDistoRamBank(data);
    break;

  case 0x9C:
    break;

  case 0x9D:
  case 0x9E:
    SetVerticalOffsetRegister((registersState->GimeRegisters[0x9D] << 8) | registersState->GimeRegisters[0x9E]);
    break;

  case 0x9F:
    SetGimeHorzOffset(data);
    break;

  case 0xA0:
  case 0xA1:
  case 0xA2:
  case 0xA3:
  case 0xA4:
  case 0xA5:
  case 0xA6:
  case 0xA7:
  case 0xA8:
  case 0xA9:
  case 0xAA:
  case 0xAB:
  case 0xAC:
  case 0xAD:
  case 0xAE:
  case 0xAF:
    SetMmuRegister(port, data);
    break;

  case 0xB0:
  case 0xB1:
  case 0xB2:
  case 0xB3:
  case 0xB4:
  case 0xB5:
  case 0xB6:
  case 0xB7:
  case 0xB8:
  case 0xB9:
  case 0xBA:
  case 0xBB:
  case 0xBC:
  case 0xBD:
  case 0xBE:
  case 0xBF:
    SetGimePallet(port - 0xB0, data & 63);
    break;
  }
}

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

void SetInit0(unsigned char data)
{
  RegistersState* registersState = GetRegistersState();

  SetCompatMode(!!(data & 128));
  SetMmuEnabled(!!(data & 64)); //MMUEN
  SetRomMap(data & 3);			//MC0-MC1
  SetVectors(data & 8);			//MC3

  registersState->EnhancedFIRQFlag = (data & 16) >> 4;
  registersState->EnhancedIRQFlag = (data & 32) >> 5;
}

void SetInit1(unsigned char data)
{
  SetMmuTask(data & 1);			//TR
  SetTimerClockRate(data & 32);	//TINS
}

unsigned char GetInit0(unsigned char port)
{
  unsigned char data = 0;

  return(data);
}

void SetGimeIRQStearing(unsigned char data) //92
{
  RegistersState* registersState = GetRegistersState();

  if ((registersState->GimeRegisters[0x92] & 2) | (registersState->GimeRegisters[0x93] & 2)) {
    GimeSetKeyboardInterruptState(1);
  }
  else {
    GimeSetKeyboardInterruptState(0);
  }

  if ((registersState->GimeRegisters[0x92] & 8) | (registersState->GimeRegisters[0x93] & 8)) {
    SetVertInterruptState(1);
  }
  else {
    SetVertInterruptState(0);
  }

  if ((registersState->GimeRegisters[0x92] & 16) | (registersState->GimeRegisters[0x93] & 16)) {
    SetHorzInterruptState(1);
  }
  else {
    SetHorzInterruptState(0);
  }

  if ((registersState->GimeRegisters[0x92] & 32) | (registersState->GimeRegisters[0x93] & 32)) {
    SetTimerInterruptState(1);
  }
  else {
    SetTimerInterruptState(0);
  }
}

void SetGimeFIRQStearing(unsigned char data) //93
{
  RegistersState* registersState = GetRegistersState();

  if ((registersState->GimeRegisters[0x92] & 2) | (registersState->GimeRegisters[0x93] & 2)) {
    GimeSetKeyboardInterruptState(1);
  }
  else {
    GimeSetKeyboardInterruptState(0);
  }

  if ((registersState->GimeRegisters[0x92] & 8) | (registersState->GimeRegisters[0x93] & 8)) {
    SetVertInterruptState(1);
  }
  else {
    SetVertInterruptState(0);
  }

  if ((registersState->GimeRegisters[0x92] & 16) | (registersState->GimeRegisters[0x93] & 16)) {
    SetHorzInterruptState(1);
  }
  else {
    SetHorzInterruptState(0);
  }

  // Moon Patrol Demo Using Timer for FIRQ Side Scroll 
  if ((registersState->GimeRegisters[0x92] & 32) | (registersState->GimeRegisters[0x93] & 32)) {
    SetTimerInterruptState(1);
  }
  else {
    SetTimerInterruptState(0);
  }
}

void SetTimerMSB(unsigned char data) //94
{
  unsigned short temp;

  RegistersState* registersState = GetRegistersState();

  temp = ((registersState->GimeRegisters[0x94] << 8) + registersState->GimeRegisters[0x95]) & 4095;

  SetInterruptTimer(temp);
}

void SetTimerLSB(unsigned char data) //95
{
  unsigned short temp;

  RegistersState* registersState = GetRegistersState();

  temp = ((registersState->GimeRegisters[0x94] << 8) + registersState->GimeRegisters[0x95]) & 4095;

  SetInterruptTimer(temp);
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

unsigned char sam_read(unsigned char port) //SAM don't talk much :)
{
  RegistersState* registersState = GetRegistersState();

  if ((port >= 0xF0) && (port <= 0xFF)) { //IRQ vectors from rom
    return(registersState->Rom[0x3F00 + port]);
  }

  return(0);
}

void sam_write(unsigned char data, unsigned char port)
{
  unsigned char mask = 0;
  unsigned char reg = 0;

  RegistersState* registersState = GetRegistersState();

  if ((port >= 0xC6) && (port <= 0xD3))	//VDG Display offset Section
  {
    port = port - 0xC6;
    reg = ((port & 0x0E) >> 1);
    mask = 1 << reg;

    registersState->Dis_Offset = registersState->Dis_Offset & (0xFF - mask); //Shut the bit off

    if (port & 1) {
      registersState->Dis_Offset = registersState->Dis_Offset | mask;
    }

    SetGimeVdgOffset(registersState->Dis_Offset);
  }

  if ((port >= 0xC0) && (port <= 0xC5))	//VDG Mode
  {
    port = port - 0xC0;
    reg = ((port & 0x0E) >> 1);
    mask = 1 << reg;
    registersState->VDG_Mode = registersState->VDG_Mode & (0xFF - mask);

    if (port & 1) {
      registersState->VDG_Mode = registersState->VDG_Mode | mask;
    }

    SetGimeVdgMode(registersState->VDG_Mode);
  }

  if ((port == 0xDE) || (port == 0xDF)) {
    SetMapType(port & 1);
  }

  if ((port == 0xD7) || (port == 0xD9)) {
    SetCPUMultiplayerFlag(1);
  }

  if ((port == 0xD6) || (port == 0xD8)) {
    SetCPUMultiplayerFlag(0);
  }
}

void mc6883_reset()
{
  RegistersState* registersState = GetRegistersState();

  registersState->VDG_Mode = 0;
  registersState->Dis_Offset = 0;
  registersState->MPU_Rate = 0;

  registersState->Rom = GetInternalRomPointer();
}

unsigned char VDG_Offset(void)
{
  RegistersState* registersState = GetRegistersState();

  return(registersState->Dis_Offset);
}

unsigned char VDG_Modes(void)
{
  RegistersState* registersState = GetRegistersState();

  return(registersState->VDG_Mode);
}
