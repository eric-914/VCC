#include <stdint.h>

#include "MC6821.h"

#include "macros.h"
#include "cpudef.h"
#include "defines.h"

#include "PAKInterface.h"
#include "Keyboard.h"
#include "Cassette.h"
#include "Graphics.h"

const unsigned char rega[4] = { 0,0,0,0 };
const unsigned char regb[4] = { 0,0,0,0 };
const unsigned char rega_dd[4] = { 0,0,0,0 };
const unsigned char regb_dd[4] = { 0,0,0,0 };

MC6821State* InitializeInstance(MC6821State*);

static MC6821State* instance = InitializeInstance(new MC6821State());

extern "C" {
  __declspec(dllexport) MC6821State* __cdecl GetMC6821State() {
    return instance;
  }
}

MC6821State* InitializeInstance(MC6821State* p) {
  p->LeftChannel = 0;
  p->RightChannel = 0;
  p->Asample = 0;
  p->Ssample = 0;
  p->Csample = 0;
  p->CartInserted = 0;
  p->CartAutoStart = 1;
  p->AddLF = 0;

  p->hPrintFile = INVALID_HANDLE_VALUE;
  p->hOut = NULL;
  p->MonState = FALSE;

  ARRAYCOPY(rega);
  ARRAYCOPY(regb);
  ARRAYCOPY(rega_dd);
  ARRAYCOPY(regb_dd);

  return p;
}

extern "C" {
  __declspec(dllexport) unsigned int __cdecl MC6821_GetDACSample(void)
  {
    static unsigned int retVal = 0;
    static unsigned short sampleLeft = 0, sampleRight = 0, pakSample = 0;
    static unsigned short outLeft = 0, outRight = 0;
    static unsigned short lastLeft = 0, lastRight = 0;

    pakSample = PakAudioSample();
    sampleLeft = (pakSample >> 8) + instance->Asample + instance->Ssample;
    sampleRight = (pakSample & 0xFF) + instance->Asample + instance->Ssample; //9 Bits each
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
}

extern "C" {
  __declspec(dllexport) void __cdecl MC6821_AssertCart(void)
  {
    CPU* cpu = GetCPU();

    instance->regb[3] = (instance->regb[3] | 128);

    if (instance->regb[3] & 1) {
      cpu->CPUAssertInterrupt(FIRQ, 0);
    }
    else {
      cpu->CPUDeAssertInterrupt(FIRQ); //Kludge but working
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MC6821_ClosePrintFile(void)
  {
    CloseHandle(instance->hPrintFile);

    instance->hPrintFile = INVALID_HANDLE_VALUE;

    FreeConsole();

    instance->hOut = NULL;
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl MC6821_DACState(void)
  {
    return (instance->regb[0] >> 2);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl MC6821_GetCasSample(void)
  {
    return(instance->Csample);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl MC6821_GetMuxState(void)
  {
    return (((instance->rega[1] & 8) >> 3) + ((instance->rega[3] & 8) >> 2));
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MC6821_irq_fs(int phase)	//60HZ Vertical sync pulse 16.667 mS
  {
    if ((instance->CartInserted == 1) && (instance->CartAutoStart == 1)) {
      MC6821_AssertCart();
    }

    CPU* cpu = GetCPU();

    switch (phase)
    {
    case 0:	//FS went High to low
      if ((instance->rega[3] & 2) == 0) { //IRQ on High to low transition
        instance->rega[3] = (instance->rega[3] | 128);
      }

      if (instance->rega[3] & 1) {
        cpu->CPUAssertInterrupt(IRQ, 1);
      }

      return;

    case 1:	//FS went Low to High

      if ((instance->rega[3] & 2)) //IRQ  Low to High transition
      {
        instance->rega[3] = (instance->rega[3] | 128);

        if (instance->rega[3] & 1) {
          cpu->CPUAssertInterrupt(IRQ, 1);
        }
      }

      return;
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MC6821_irq_hs(int phase)	//63.5 uS
  {
    CPU* cpu = GetCPU();

    switch (phase)
    {
    case FALLING:	//HS went High to low
      if ((instance->rega[1] & 2)) { //IRQ on low to High transition
        return;
      }

      instance->rega[1] = (instance->rega[1] | 128);

      if (instance->rega[1] & 1) {
        cpu->CPUAssertInterrupt(IRQ, 1);
      }

      break;

    case RISING:	//HS went Low to High
      if (!(instance->rega[1] & 2)) { //IRQ  High to low transition
        return;
      }

      instance->rega[1] = (instance->rega[1] | 128);

      if (instance->rega[1] & 1) {
        cpu->CPUAssertInterrupt(IRQ, 1);
      }

      break;

    case ANY:
      instance->rega[1] = (instance->rega[1] | 128);

      if (instance->rega[1] & 1) {
        cpu->CPUAssertInterrupt(IRQ, 1);
      }

      break;
    }
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl MC6821_OpenPrintFile(char* filename)
  {
    instance->hPrintFile = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if (instance->hPrintFile == INVALID_HANDLE_VALUE) {
      return(0);
    }

    return(1);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MC6821_PiaReset()
  {
    // Clear the PIA registers
    for (uint8_t index = 0; index < 4; index++)
    {
      instance->rega[index] = 0;
      instance->regb[index] = 0;
      instance->rega_dd[index] = 0;
      instance->regb_dd[index] = 0;
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetCart(unsigned char cart)
  {
    instance->CartInserted = cart;
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl MC6821_SetCartAutoStart(unsigned char autostart)
  {
    if (autostart != QUERY) {
      instance->CartAutoStart = autostart;
    }

    return(instance->CartAutoStart);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MC6821_SetCassetteSample(unsigned char sample)
  {
    instance->regb[0] = instance->regb[0] & 0xFE;

    if (sample > 0x7F) {
      instance->regb[0] = instance->regb[0] | 1;
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MC6821_SetMonState(BOOL state)
  {
    if (instance->MonState & !state)
    {
      FreeConsole();

      instance->hOut = NULL;
    }

    instance->MonState = state;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MC6821_SetSerialParams(unsigned char textMode)
  {
    instance->AddLF = textMode;
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl MC6821_VDG_Mode(void)
  {
    return((instance->regb[2] & 248) >> 3);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MC6821_WritePrintMon(char* data)
  {
    unsigned long dummy;

    if (instance->hOut == NULL)
    {
      AllocConsole();

      instance->hOut = GetStdHandle(STD_OUTPUT_HANDLE);

      SetConsoleTitle("Printer Monitor");
    }

    WriteConsole(instance->hOut, data, 1, &dummy, 0);

    if (data[0] == 0x0D)
    {
      data[0] = 0x0A;

      WriteConsole(instance->hOut, data, 1, &dummy, 0);
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MC6821_CaptureBit(unsigned char sample)
  {
    unsigned long bytesMoved = 0;
    static unsigned char bitMask = 1, startWait = 1;
    static char Byte = 0;

    if (instance->hPrintFile == INVALID_HANDLE_VALUE) {
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

      WriteFile(instance->hPrintFile, &Byte, 1, &bytesMoved, NULL);

      if (instance->MonState) {
        MC6821_WritePrintMon(&Byte);
      }

      if ((Byte == 0x0D) & instance->AddLF)
      {
        Byte = 0x0A;

        WriteFile(instance->hPrintFile, &Byte, 1, &bytesMoved, NULL);
      }

      Byte = 0;
    }
  }
}

// Shift Row Col
extern "C" {
  __declspec(dllexport) unsigned char __cdecl MC6821_pia0_read(unsigned char port)
  {
    unsigned char dda, ddb;

    dda = (instance->rega[1] & 4);
    ddb = (instance->rega[3] & 4);

    switch (port)
    {
    case 1:
      return(instance->rega[port]);

    case 3:
      return(instance->rega[port]);

    case 0:
      if (dda)
      {
        instance->rega[1] = (instance->rega[1] & 63);

        return (vccKeyboardGetScan(instance->rega[2])); //Read
      }
      else {
        return(instance->rega_dd[port]);
      }

    case 2: //Write 
      if (ddb)
      {
        instance->rega[3] = (instance->rega[3] & 63);

        return(instance->rega[port] & instance->rega_dd[port]);
      }
      else {
        return(instance->rega_dd[port]);
      }
    }

    return(0);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MC6821_pia0_write(unsigned char data, unsigned char port)
  {
    unsigned char dda, ddb;

    dda = (instance->rega[1] & 4);
    ddb = (instance->rega[3] & 4);

    switch (port)
    {
    case 0:
      if (dda) {
        instance->rega[port] = data;
      }
      else {
        instance->rega_dd[port] = data;
      }

      return;

    case 2:
      if (ddb) {
        instance->rega[port] = data;
      }
      else {
        instance->rega_dd[port] = data;
      }

      return;

    case 1:
      instance->rega[port] = (data & 0x3F);

      return;

    case 3:
      instance->rega[port] = (data & 0x3F);

      return;
    }
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl MC6821_pia1_read(unsigned char port)
  {
    static unsigned int flag = 0, flag2 = 0;
    unsigned char dda, ddb;

    port -= 0x20;
    dda = (instance->regb[1] & 4);
    ddb = (instance->regb[3] & 4);

    switch (port)
    {
    case 1:
      //	return(0);

    case 3:
      return(instance->regb[port]);

    case 2:
      if (ddb)
      {
        instance->regb[3] = (instance->regb[3] & 63);

        return(instance->regb[port] & instance->regb_dd[port]);
      }
      else {
        return(instance->regb_dd[port]);
      }

    case 0:
      if (dda)
      {
        instance->regb[1] = (instance->regb[1] & 63); //Cass In
        flag = instance->regb[port]; //& regb_dd[port];

        return(flag);
      }
      else {
        return(instance->regb_dd[port]);
      }
    }

    return(0);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MC6821_pia1_write(unsigned char data, unsigned char port)
  {
    unsigned char dda, ddb;
    static unsigned short LastSS = 0;

    port -= 0x20;

    dda = (instance->regb[1] & 4);
    ddb = (instance->regb[3] & 4);

    switch (port)
    {
    case 0:
      if (dda) {
        instance->regb[port] = data;

        MC6821_CaptureBit((instance->regb[0] & 2) >> 1);

        if (MC6821_GetMuxState() == 0) {
          if ((instance->regb[3] & 8) != 0) { //==0 for cassette writes
            instance->Asample = (instance->regb[0] & 0xFC) >> 1; //0 to 127
          }
          else {
            instance->Csample = (instance->regb[0] & 0xFC);
          }
        }
      }
      else {
        instance->regb_dd[port] = data;
      }

      return;

    case 2: //FF22
      if (ddb)
      {
        instance->regb[port] = (data & instance->regb_dd[port]);

        SetGimeVdgMode2((instance->regb[2] & 248) >> 3);

        instance->Ssample = (instance->regb[port] & 2) << 6;
      }
      else {
        instance->regb_dd[port] = data;
      }

      return;

    case 1:
      instance->regb[port] = (data & 0x3F);

      Motor((data & 8) >> 3);

      return;

    case 3:
      instance->regb[port] = (data & 0x3F);

      return;
    }
  }
}