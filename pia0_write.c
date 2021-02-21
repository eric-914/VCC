#include "library/MC6821.h"

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
