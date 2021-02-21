#include "library/MC6821.h"

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
