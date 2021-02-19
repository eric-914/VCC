#include "library/mc6821state.h"

#include "library/defines.h"

unsigned char SetCartAutoStart(unsigned char autostart)
{
  MC6821State* mc6821State = GetMC6821State();

  if (autostart != QUERY) {
    mc6821State->CartAutoStart = autostart;
  }

  return(mc6821State->CartAutoStart);
}
