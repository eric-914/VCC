#include "library/registersstate.h"
#include "library/Graphics.h"
#include "library/VCC.h"

#include "SetDistoRamBank.h"
#include "library/MmuAccessors.h"
#include "SetInit0.h"
#include "SetInit1.h"
#include "SetGimeIRQSteering.h"
#include "SetGimeFIRQSteering.h"
#include "SetTimerMSB.h"
#include "SetTimerLSB.h"


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
