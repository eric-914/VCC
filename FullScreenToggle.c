#include "vccstate.h"

#include "CreateDDWindow.h"
#include "RefreshDynamicMenu.h"
#include "AudioAccessors.h"

extern void InvalidateBoarder();

void FullScreenToggle(void)
{
  VccState* vccState = GetVccState();

  PauseAudio(true);

  if (!CreateDDWindow(&(vccState->EmuState)))
  {
    MessageBox(0, "Can't rebuild primary Window", "Error", 0);

    exit(0);
  }

  InvalidateBoarder();
  RefreshDynamicMenu(&(vccState->EmuState));

  vccState->EmuState.ConfigDialog = NULL;

  PauseAudio(false);
}
