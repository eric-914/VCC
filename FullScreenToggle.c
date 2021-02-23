#include "library/VCC.h"
#include "library/Audio.h"
#include "library/Graphics.h"
#include "library/PAKInterface.h"

#include "CreateDirectDrawWindow.h"

void FullScreenToggle(WNDPROC WndProc)
{
  VccState* vccState = GetVccState();

  PauseAudio(true);

  if (!CreateDirectDrawWindow(&(vccState->EmuState), WndProc))
  {
    MessageBox(0, "Can't rebuild primary Window", "Error", 0);

    exit(0);
  }

  InvalidateBorder();
  RefreshDynamicMenu(&(vccState->EmuState));

  vccState->EmuState.ConfigDialog = NULL;

  PauseAudio(false);
}
