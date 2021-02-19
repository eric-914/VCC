#include "library/mc6821state.h"

#include "vccKeyboardGetScan.h"

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
