#include "library/vccstate.h"

#include "CreateDDWindow.h"
#include "RefreshDynamicMenu.h"
#include "AudioAccessors.h"
#include "InvalidateBorder.h "

void FullScreenToggle(WNDPROC WndProc)
{
  VccState* vccState = GetVccState();

  PauseAudio(true);

  if (!CreateDDWindow(&(vccState->EmuState), WndProc))
  {
    MessageBox(0, "Can't rebuild primary Window", "Error", 0);

    exit(0);
  }

  InvalidateBorder();
  RefreshDynamicMenu(&(vccState->EmuState));

  vccState->EmuState.ConfigDialog = NULL;

  PauseAudio(false);
}
