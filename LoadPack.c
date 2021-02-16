#include <process.h>

#include "vccstate.h"

extern unsigned __stdcall CartLoad(void* dummy);

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
