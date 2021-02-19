#include "library/mc6821state.h"

#include "SetGimeVdgMode2.h"
#include "Motor.h"
#include "GetMuxState.h"
#include "CaptureBit.h"

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
