#include <windows.h>

#include "library/VCC.h"
#include "library/Config.h"
#include "library/Keyboard.h"
#include "library/Graphics.h"
#include "library/DirectDraw.h"

#include "resources/resource.h"

#include "MessageHandlers.h"

void ProcessKeyDownMessage(WPARAM wParam, LPARAM lParam) {
  // get key scan code for emulator control keys
  unsigned char OEMscan = (unsigned char)((lParam & 0x00FF0000) >> 16); // just get the scan code

  VccState* vccState = GetVccState();

  switch (OEMscan)
  {
  case DIK_F3:
    DecreaseOverclockSpeed(&(vccState->EmuState));
    break;

  case DIK_F4:
    IncreaseOverclockSpeed(&(vccState->EmuState));
    break;

  case DIK_F5:
    EmuReset(1);
    break;

  case DIK_F6:
    SetMonitorType(!SetMonitorType(QUERY));
    break;

  case DIK_F8:
    SetSpeedThrottle(!SetSpeedThrottle(QUERY));
    break;

  case DIK_F9:
    vccState->EmuState.EmulationRunning = !vccState->EmuState.EmulationRunning;

    if (vccState->EmuState.EmulationRunning) {
      vccState->EmuState.ResetPending = 2;
    }
    else {
      SetStatusBarText("", &(vccState->EmuState));
    }

    break;

  case DIK_F10:
    SetInfoBand(!SetInfoBand(QUERY));
    InvalidateBorder();

    break;

  case DIK_F11:
    if (vccState->FlagEmuStop == TH_RUNNING)
    {
      vccState->FlagEmuStop = TH_REQWAIT;
      vccState->EmuState.FullScreen = !vccState->EmuState.FullScreen;
    }

    break;

  default:
    // send other keystrokes to the emulator if it is active
    if (vccState->EmuState.EmulationRunning)
    {
      vccKeyboardHandleKey((unsigned char)wParam, OEMscan, kEventKeyDown);

      // Save key down in case focus is lost
      SaveLastTwoKeyDownEvents((unsigned char)wParam, OEMscan);
    }
    break;
  }
}

void ProcessSysKeyDownMessage(WPARAM wParam, LPARAM lParam) {
  // Ignore repeated system keys
  if (!(lParam >> 30)) {
    ProcessKeyDownMessage(wParam, lParam);
  }
}
