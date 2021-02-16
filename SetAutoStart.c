#include "vccstate.h"

unsigned char SetAutoStart(unsigned char autostart)
{
  VccState* vccState = GetVccState();

  if (autostart != QUERY) {
    vccState->AutoStart = autostart;
  }

  return(vccState->AutoStart);
}
