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
#include <cstdint>

#include "mc6821def.h"
#include "mc6821.h"
#include "motor.h"

#include "PakInterfaceAccessors.h"
#include "vccKeyboardGetScan.h"
#include "SetGimeVdgMode2.h"
#include "mc6821state.h"

#include "library/cpudef.h"
#include "library/defines.h"

void CaptureBit(unsigned char);
void WritePrintMon(char*);
LRESULT CALLBACK PrintMon(HWND, UINT, WPARAM, LPARAM);

//static unsigned char CoutSample=0;
//extern STRConfig CurrentConfig;
// Shift Row Col
unsigned char pia0_read(unsigned char port)
{
  unsigned char dda, ddb;

  MC6821State* mc6821State = GetMC6821State();

  dda = (mc6821State->rega[1] & 4);
  ddb = (mc6821State->rega[3] & 4);

  switch (port)
  {
  case 1:
    return(mc6821State->rega[port]);

  case 3:
    return(mc6821State->rega[port]);

  case 0:
    if (dda)
    {
      mc6821State->rega[1] = (mc6821State->rega[1] & 63);

      return (vccKeyboardGetScan(mc6821State->rega[2])); //Read
    }
    else {
      return(mc6821State->rega_dd[port]);
    }

  case 2: //Write 
    if (ddb)
    {
      mc6821State->rega[3] = (mc6821State->rega[3] & 63);

      return(mc6821State->rega[port] & mc6821State->rega_dd[port]);
    }
    else {
      return(mc6821State->rega_dd[port]);
    }
  }

  return(0);
}

unsigned char pia1_read(unsigned char port)
{
  static unsigned int flag = 0, flag2 = 0;
  unsigned char dda, ddb;

  MC6821State* mc6821State = GetMC6821State();

  port -= 0x20;
  dda = (mc6821State->regb[1] & 4);
  ddb = (mc6821State->regb[3] & 4);

  switch (port)
  {
  case 1:
    //	return(0);

  case 3:
    return(mc6821State->regb[port]);

  case 2:
    if (ddb)
    {
      mc6821State->regb[3] = (mc6821State->regb[3] & 63);

      return(mc6821State->regb[port] & mc6821State->regb_dd[port]);
    }
    else {
      return(mc6821State->regb_dd[port]);
    }

  case 0:
    if (dda)
    {
      mc6821State->regb[1] = (mc6821State->regb[1] & 63); //Cass In
      flag = mc6821State->regb[port]; //& regb_dd[port];

      return(flag);
    }
    else {
      return(mc6821State->regb_dd[port]);
    }
  }

  return(0);
}

void pia0_write(unsigned char data, unsigned char port)
{
  unsigned char dda, ddb;

  MC6821State* mc6821State = GetMC6821State();

  dda = (mc6821State->rega[1] & 4);
  ddb = (mc6821State->rega[3] & 4);

  switch (port)
  {
  case 0:
    if (dda) {
      mc6821State->rega[port] = data;
    }
    else {
      mc6821State->rega_dd[port] = data;
    }

    return;

  case 2:
    if (ddb) {
      mc6821State->rega[port] = data;
    }
    else {
      mc6821State->rega_dd[port] = data;
    }

    return;

  case 1:
    mc6821State->rega[port] = (data & 0x3F);

    return;

  case 3:
    mc6821State->rega[port] = (data & 0x3F);

    return;
  }
}

void pia1_write(unsigned char data, unsigned char port)
{
  unsigned char dda, ddb;
  static unsigned short LastSS = 0;

  MC6821State* mc6821State = GetMC6821State();

  port -= 0x20;

  dda = (mc6821State->regb[1] & 4);
  ddb = (mc6821State->regb[3] & 4);

  switch (port)
  {
  case 0:
    if (dda) {
      mc6821State->regb[port] = data;

      CaptureBit((mc6821State->regb[0] & 2) >> 1);

      if (GetMuxState() == 0) {
        if ((mc6821State->regb[3] & 8) != 0) { //==0 for cassette writes
          mc6821State->Asample = (mc6821State->regb[0] & 0xFC) >> 1; //0 to 127
        }
        else {
          mc6821State->Csample = (mc6821State->regb[0] & 0xFC);
        }
      }
    }
    else {
      mc6821State->regb_dd[port] = data;
    }

    return;

  case 2: //FF22
    if (ddb)
    {
      mc6821State->regb[port] = (data & mc6821State->regb_dd[port]);

      SetGimeVdgMode2((mc6821State->regb[2] & 248) >> 3);

      mc6821State->Ssample = (mc6821State->regb[port] & 2) << 6;
    }
    else {
      mc6821State->regb_dd[port] = data;
    }

    return;

  case 1:
    mc6821State->regb[port] = (data & 0x3F);

    Motor((data & 8) >> 3);

    return;

  case 3:
    mc6821State->regb[port] = (data & 0x3F);

    return;
  }
}

unsigned char VDG_Mode(void)
{
  MC6821State* mc6821State = GetMC6821State();

  return((mc6821State->regb[2] & 248) >> 3);
}

void irq_hs(int phase)	//63.5 uS
{
  MC6821State* mc6821State = GetMC6821State();
  CPU* cpu = GetCPU();

  switch (phase)
  {
  case FALLING:	//HS went High to low
    if ((mc6821State->rega[1] & 2)) { //IRQ on low to High transition
      return;
    }

    mc6821State->rega[1] = (mc6821State->rega[1] | 128);

    if (mc6821State->rega[1] & 1) {
      cpu->CPUAssertInterrupt(IRQ, 1);
    }

    break;

  case RISING:	//HS went Low to High
    if (!(mc6821State->rega[1] & 2)) { //IRQ  High to low transition
      return;
    }

    mc6821State->rega[1] = (mc6821State->rega[1] | 128);

    if (mc6821State->rega[1] & 1) {
      cpu->CPUAssertInterrupt(IRQ, 1);
    }

    break;

  case ANY:
    mc6821State->rega[1] = (mc6821State->rega[1] | 128);

    if (mc6821State->rega[1] & 1) {
      cpu->CPUAssertInterrupt(IRQ, 1);
    }

    break;
  }
}

void irq_fs(int phase)	//60HZ Vertical sync pulse 16.667 mS
{
  MC6821State* mc6821State = GetMC6821State();

  if ((mc6821State->CartInserted == 1) && (mc6821State->CartAutoStart == 1)) {
    AssertCart();
  }

  CPU* cpu = GetCPU();

  switch (phase)
  {
  case 0:	//FS went High to low
    if ((mc6821State->rega[3] & 2) == 0) { //IRQ on High to low transition
      mc6821State->rega[3] = (mc6821State->rega[3] | 128);
    }

    if (mc6821State->rega[3] & 1) {
      cpu->CPUAssertInterrupt(IRQ, 1);
    }

    return;

  case 1:	//FS went Low to High

    if ((mc6821State->rega[3] & 2)) //IRQ  Low to High transition
    {
      mc6821State->rega[3] = (mc6821State->rega[3] | 128);

      if (mc6821State->rega[3] & 1) {
        cpu->CPUAssertInterrupt(IRQ, 1);
      }
    }

    return;
  }
}

void AssertCart(void)
{
  MC6821State* mc6821State = GetMC6821State();
  CPU* cpu = GetCPU();

  mc6821State->regb[3] = (mc6821State->regb[3] | 128);

  if (mc6821State->regb[3] & 1) {
    cpu->CPUAssertInterrupt(FIRQ, 0);
  }
  else {
    cpu->CPUDeAssertInterrupt(FIRQ); //Kludge but working
  }
}

void PiaReset()
{
  MC6821State* mc6821State = GetMC6821State();

  // Clear the PIA registers
  for (uint8_t index = 0; index < 4; index++)
  {
    mc6821State->rega[index] = 0;
    mc6821State->regb[index] = 0;
    mc6821State->rega_dd[index] = 0;
    mc6821State->regb_dd[index] = 0;
  }
}

unsigned char GetMuxState(void)
{
  MC6821State* mc6821State = GetMC6821State();

  return (((mc6821State->rega[1] & 8) >> 3) + ((mc6821State->rega[3] & 8) >> 2));
}

unsigned char DACState(void)
{
  MC6821State* mc6821State = GetMC6821State();

  return (mc6821State->regb[0] >> 2);
}

void SetCart(unsigned char cart)
{
  MC6821State* mc6821State = GetMC6821State();

  mc6821State->CartInserted = cart;
}

unsigned int GetDACSample(void)
{
  static unsigned int retVal = 0;
  static unsigned short sampleLeft = 0, sampleRight = 0, pakSample = 0;
  static unsigned short outLeft = 0, outRight = 0;
  static unsigned short lastLeft = 0, lastRight = 0;

  MC6821State* mc6821State = GetMC6821State();

  pakSample = PackAudioSample();
  sampleLeft = (pakSample >> 8) + mc6821State->Asample + mc6821State->Ssample;
  sampleRight = (pakSample & 0xFF) + mc6821State->Asample + mc6821State->Ssample; //9 Bits each
  sampleLeft = sampleLeft << 6;	//Conver to 16 bit values
  sampleRight = sampleRight << 6;	//For Max volume

  if (sampleLeft == lastLeft)	//Simulate a slow high pass filter
  {
    if (outLeft) {
      outLeft--;
    }
  }
  else
  {
    outLeft = sampleLeft;
    lastLeft = sampleLeft;
  }

  if (sampleRight == lastRight)
  {
    if (outRight) {
      outRight--;
    }
  }
  else
  {
    outRight = sampleRight;
    lastRight = sampleRight;
  }

  retVal = (outLeft << 16) + (outRight);

  return(retVal);
}

unsigned char SetCartAutoStart(unsigned char autostart)
{
  MC6821State* mc6821State = GetMC6821State();

  if (autostart != QUERY) {
    mc6821State->CartAutoStart = autostart;
  }

  return(mc6821State->CartAutoStart);
}

unsigned char GetCasSample(void)
{
  MC6821State* mc6821State = GetMC6821State();

  return(mc6821State->Csample);
}

void SetCassetteSample(unsigned char Sample)
{
  MC6821State* mc6821State = GetMC6821State();

  mc6821State->regb[0] = mc6821State->regb[0] & 0xFE;

  if (Sample > 0x7F) {
    mc6821State->regb[0] = mc6821State->regb[0] | 1;
  }
}

void CaptureBit(unsigned char sample)
{
  unsigned long bytesMoved = 0;
  static unsigned char bitMask = 1, startWait = 1;
  static char Byte = 0;

  MC6821State* mc6821State = GetMC6821State();

  if (mc6821State->hPrintFile == INVALID_HANDLE_VALUE) {
    return;
  }

  if (startWait & sample) { //Waiting for start bit
    return;
  }

  if (startWait)
  {
    startWait = 0;

    return;
  }

  if (sample) {
    Byte |= bitMask;
  }

  bitMask = bitMask << 1;

  if (!bitMask)
  {
    bitMask = 1;
    startWait = 1;

    WriteFile(mc6821State->hPrintFile, &Byte, 1, &bytesMoved, NULL);

    if (mc6821State->MonState) {
      WritePrintMon(&Byte);
    }

    if ((Byte == 0x0D) & mc6821State->AddLF)
    {
      Byte = 0x0A;

      WriteFile(mc6821State->hPrintFile, &Byte, 1, &bytesMoved, NULL);
    }

    Byte = 0;
  }
}

int OpenPrintFile(char* FileName)
{
  MC6821State* mc6821State = GetMC6821State();

  mc6821State->hPrintFile = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

  if (mc6821State->hPrintFile == INVALID_HANDLE_VALUE) {
    return(0);
  }

  return(1);
}

void ClosePrintFile(void)
{
  MC6821State* mc6821State = GetMC6821State();

  CloseHandle(mc6821State->hPrintFile);

  mc6821State->hPrintFile = INVALID_HANDLE_VALUE;

  FreeConsole();

  mc6821State->hOut = NULL;
}

void SetSerialParams(unsigned char textMode)
{
  MC6821State* mc6821State = GetMC6821State();

  mc6821State->AddLF = textMode;
}

void SetMonState(BOOL state)
{
  MC6821State* mc6821State = GetMC6821State();

  if (mc6821State->MonState & !state)
  {
    FreeConsole();

    mc6821State->hOut = NULL;
  }

  mc6821State->MonState = state;
}

void WritePrintMon(char* data)
{
  unsigned long dummy;

  MC6821State* mc6821State = GetMC6821State();

  if (mc6821State->hOut == NULL)
  {
    AllocConsole();

    mc6821State->hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleTitle("Printer Monitor");
  }

  WriteConsole(mc6821State->hOut, data, 1, &dummy, 0);

  if (data[0] == 0x0D)
  {
    data[0] = 0x0A;

    WriteConsole(mc6821State->hOut, data, 1, &dummy, 0);
  }
}
