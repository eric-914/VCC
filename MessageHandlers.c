#include <windows.h>

#include "library/VCC.h"
#include "library/Callbacks.h"
#include "library/Config.h"
#include "library/Graphics.h"
#include "library/Keyboard.h"
#include "library/Joystick.h"
#include "library/DirectDraw.h"

#include "resources/resource.h"

void HelpAbout(HWND hWnd) {
  VccState* vccState = GetVccState();

  DialogBox(vccState->EmuState.Resources, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)DialogBoxAboutCallback);
}

void CreateMenu(HWND hWnd) {
  VccState* vccState = GetVccState();

  if (!vccState->EmuState.FullScreen) {
    SetMenu(hWnd, LoadMenu(vccState->EmuState.Resources, MAKEINTRESOURCE(IDR_MENU)));
  }
  else {
    SetMenu(hWnd, NULL);
  }
}

void ShowConfiguration() {
  VccState* vccState = GetVccState();

#ifdef CONFIG_DIALOG_MODAL
  // open config dialog modally
  DialogBox(vccState->EmuState.Resources, (LPCTSTR)IDD_TCONFIG, hWnd, (DLGPROC)Config);
#else

  // open config dialog if not already open
  // opens modeless so you can control the cassette
  // while emulator is still running (assumed)
  if (vccState->EmuState.ConfigDialog == NULL)
  {
    vccState->EmuState.ConfigDialog = CreateDialog(vccState->EmuState.Resources, (LPCTSTR)IDD_TCONFIG, vccState->EmuState.WindowHandle, (DLGPROC)CreateMainConfigDialogCallback);

    // open modeless
    ShowWindow(vccState->EmuState.ConfigDialog, SW_SHOWNORMAL);
  }
#endif
}

void EmuReset(unsigned char state) {
  VccState* vccState = GetVccState();

  if (vccState->EmuState.EmulationRunning) {
    vccState->EmuState.ResetPending = state;
  }
}

void EmuRun() {
  VccState* vccState = GetVccState();

  vccState->EmuState.EmulationRunning = TRUE;

  InvalidateBorder();
}

void EmuExit() {
  VccState* vccState = GetVccState();

  vccState->BinaryRunning = 0;
}

void KeyUp(WPARAM wParam, LPARAM lParam) {
  // send emulator key up event to the emulator
  // TODO: Key up checks whether the emulation is running, this does not

  unsigned char OEMscan = (unsigned char)((lParam & 0x00FF0000) >> 16);

  vccKeyboardHandleKey((unsigned char)wParam, OEMscan, kEventKeyUp);
}

void MouseMove(LPARAM lParam) {
  RECT clientSize;

  VccState* vccState = GetVccState();

  if (vccState->EmuState.EmulationRunning)
  {
    unsigned int x = LOWORD(lParam);
    unsigned int y = HIWORD(lParam);

    GetClientRect(vccState->EmuState.WindowHandle, &clientSize);

    x /= ((clientSize.right - clientSize.left) >> 6);
    y /= (((clientSize.bottom - clientSize.top) - 20) >> 6);

    SetJoystick(x, y);
  }
}

void ToggleOnOff() {
  VccState* vccState = GetVccState();

  vccState->EmuState.EmulationRunning = !vccState->EmuState.EmulationRunning;

  if (vccState->EmuState.EmulationRunning) {
    vccState->EmuState.ResetPending = 2;
  }
  else {
    SetStatusBarText("", &(vccState->EmuState));
  }
}

void ToggleFullScreen() {
  VccState* vccState = GetVccState();

  if (vccState->FlagEmuStop == TH_RUNNING)
  {
    vccState->FlagEmuStop = TH_REQWAIT;
    vccState->EmuState.FullScreen = !vccState->EmuState.FullScreen;
  }
}

void KeyDown(WPARAM wParam, LPARAM lParam) {
  unsigned char OEMscan = (unsigned char)((lParam & 0x00FF0000) >> 16); // just get the scan code

  VccState* vccState = GetVccState();

  // send other keystrokes to the emulator if it is active
  if (vccState->EmuState.EmulationRunning)
  {
    vccKeyboardHandleKey((unsigned char)wParam, OEMscan, kEventKeyDown);

    // Save key down in case focus is lost
    SaveLastTwoKeyDownEvents((unsigned char)wParam, OEMscan);
  }
}

void ToggleMonitorType() {
  SetMonitorType(!SetMonitorType(QUERY));
}

void ToggleThrottle() {
  SetSpeedThrottle(!SetSpeedThrottle(QUERY));
}

void ToggleInfoBand() {
  SetInfoBand(!SetInfoBand(QUERY));
  InvalidateBorder();
}

void SlowDown() {
  VccState* vccState = GetVccState();

  DecreaseOverclockSpeed(&(vccState->EmuState));
}

void SpeedUp() {
  VccState* vccState = GetVccState();

  IncreaseOverclockSpeed(&(vccState->EmuState));
}
