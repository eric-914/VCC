#include <process.h>

#include "library/vccstate.h"
#include "CartLoad.h"

void LoadPack(void)
{
  unsigned threadID;

  VccState* vccState = GetVccState();

  if (vccState->DialogOpen) {
    return;
  }

  vccState->DialogOpen = true;

  _beginthreadex(NULL, 0, &CartLoad, CreateEvent(NULL, FALSE, FALSE, NULL), 0, &threadID);
}
