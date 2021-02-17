#include "registersstate.h"

#include "MmuAccessors.h"
#include "SetGimeVdgOffset.h"
#include "SetGimeVdgMode.h"
#include "SetCPUMultiplayerFlag.h"

void SAMWrite(unsigned char data, unsigned char port)
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
