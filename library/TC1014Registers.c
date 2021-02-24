#include "TC1014Registers.h"
#include "VCC.h"
#include "TC1014MMU.h"
#include "Graphics.h"
#include "Keyboard.h"
#include "CoCo.h"

#include "cpudef.h"
#include "defines.h"

TC1014RegistersState* InitializeInstance(TC1014RegistersState*);

static TC1014RegistersState* instance = InitializeInstance(new TC1014RegistersState());

extern "C" {
  __declspec(dllexport) TC1014RegistersState* __cdecl GetTC1014RegistersState() {
    return instance;
  }
}

TC1014RegistersState* InitializeInstance(TC1014RegistersState* p) {
  p->EnhancedFIRQFlag = 0;
  p->EnhancedIRQFlag = 0;
  p->VDG_Mode = 0;
  p->Dis_Offset = 0;
  p->MPU_Rate = 0;
  p->LastIrq = 0;
  p->LastFirq = 0;
  p->VerticalOffsetRegister = 0;
  p->InterruptTimer = 0;

  return p;
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl GimeRead(unsigned char port)
  {
    static unsigned char temp;

    TC1014RegistersState* registersState = GetTC1014RegistersState();

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
}

extern "C" {
  __declspec(dllexport) void __cdecl GimeAssertKeyboardInterrupt(void)
  {
    TC1014RegistersState* registersState = GetTC1014RegistersState();

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
}

extern "C" {
  __declspec(dllexport) void __cdecl GimeAssertVertInterrupt(void)
  {
    TC1014RegistersState* registersState = GetTC1014RegistersState();

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
}

extern "C" {
  __declspec(dllexport) void __cdecl GimeAssertTimerInterrupt(void)
  {
    TC1014RegistersState* registersState = GetTC1014RegistersState();

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
}

extern "C" {
  __declspec(dllexport) void __cdecl GimeAssertHorzInterrupt(void)
  {
    TC1014RegistersState* registersState = GetTC1014RegistersState();

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
}

extern "C" {
  __declspec(dllexport) void __cdecl MC6883Reset()
  {
    TC1014RegistersState* registersState = GetTC1014RegistersState();

    registersState->VDG_Mode = 0;
    registersState->Dis_Offset = 0;
    registersState->MPU_Rate = 0;

    registersState->Rom = GetInternalRomPointer();
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SAMRead(unsigned char port) //SAM don't talk much :)
  {
    TC1014RegistersState* registersState = GetTC1014RegistersState();

    if ((port >= 0xF0) && (port <= 0xFF)) { //IRQ vectors from rom
      return(registersState->Rom[0x3F00 + port]);
    }

    return(0);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SAMWrite(unsigned char data, unsigned char port)
  {
    unsigned char mask = 0;
    unsigned char reg = 0;

    TC1014RegistersState* registersState = GetTC1014RegistersState();

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
}

extern "C" {
  __declspec(dllexport) void __cdecl SetGimeFIRQSteering(unsigned char data) //93
  {
    TC1014RegistersState* registersState = GetTC1014RegistersState();

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
}

extern "C" {
  __declspec(dllexport) void __cdecl SetGimeIRQSteering(unsigned char data) //92
  {
    TC1014RegistersState* registersState = GetTC1014RegistersState();

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
}

extern "C" {
  __declspec(dllexport) void __cdecl SetInit0(unsigned char data)
  {
    TC1014RegistersState* registersState = GetTC1014RegistersState();

    SetCompatMode(!!(data & 128));
    SetMmuEnabled(!!(data & 64)); //MMUEN
    SetRomMap(data & 3);			//MC0-MC1
    SetVectors(data & 8);			//MC3

    registersState->EnhancedFIRQFlag = (data & 16) >> 4;
    registersState->EnhancedIRQFlag = (data & 32) >> 5;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetInit1(unsigned char data)
  {
    SetMmuTask(data & 1);			//TR
    SetTimerClockRate(data & 32);	//TINS
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetTimerLSB(unsigned char data) //95
  {
    unsigned short temp;

    TC1014RegistersState* registersState = GetTC1014RegistersState();

    temp = ((registersState->GimeRegisters[0x94] << 8) + registersState->GimeRegisters[0x95]) & 4095;

    SetInterruptTimer(temp);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetTimerMSB(unsigned char data) //94
  {
    unsigned short temp;

    TC1014RegistersState* registersState = GetTC1014RegistersState();

    temp = ((registersState->GimeRegisters[0x94] << 8) + registersState->GimeRegisters[0x95]) & 4095;

    SetInterruptTimer(temp);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl GimeWrite(unsigned char port, unsigned char data)
  {
    TC1014RegistersState* registersState = GetTC1014RegistersState();

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
      SetGimeIRQSteering(data);
      break;

    case 0x93:
      SetGimeFIRQSteering(data);
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
      SetGimePalette(port - 0xB0, data & 63);
      break;
    }
  }
}