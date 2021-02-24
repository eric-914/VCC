#include <windows.h>

#include "VCC.h"
#include "Callbacks.h"
#include "Config.h"
#include "Graphics.h"
#include "Keyboard.h"
#include "Joystick.h"
#include "DirectDraw.h"

#include "../resources/resource.h"

extern "C" {
  __declspec(dllexport) void __cdecl HelpAbout(HWND hWnd) {
    VccState* vccState = GetVccState();

    DialogBox(vccState->SystemState.Resources, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)DialogBoxAboutCallback);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl CreateMainMenu(HWND hWnd) {
    VccState* vccState = GetVccState();

    if (!vccState->SystemState.FullScreen) {
      SetMenu(hWnd, LoadMenu(vccState->SystemState.Resources, MAKEINTRESOURCE(IDR_MENU)));
    }
    else {
      SetMenu(hWnd, NULL);
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl ShowConfiguration() {
    VccState* vccState = GetVccState();

#ifdef CONFIG_DIALOG_MODAL
    // open config dialog modally
    DialogBox(vccState->EmuState.Resources, (LPCTSTR)IDD_TCONFIG, hWnd, (DLGPROC)Config);
#else

    // open config dialog if not already open
    // opens modeless so you can control the cassette
    // while emulator is still running (assumed)
    if (vccState->SystemState.ConfigDialog == NULL)
    {
      vccState->SystemState.ConfigDialog = CreateDialog(vccState->SystemState.Resources, (LPCTSTR)IDD_TCONFIG, vccState->SystemState.WindowHandle, (DLGPROC)CreateMainConfigDialogCallback);

      // open modeless
      ShowWindow(vccState->SystemState.ConfigDialog, SW_SHOWNORMAL);
    }
#endif
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl EmuReset(unsigned char state) {
    VccState* vccState = GetVccState();

    if (vccState->SystemState.EmulationRunning) {
      vccState->SystemState.ResetPending = state;
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl EmuRun() {
    VccState* vccState = GetVccState();

    vccState->SystemState.EmulationRunning = TRUE;

    InvalidateBorder();
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl EmuExit() {
    VccState* vccState = GetVccState();

    vccState->BinaryRunning = 0;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl KeyUp(WPARAM wParam, LPARAM lParam) {
    // send emulator key up event to the emulator
    // TODO: Key up checks whether the emulation is running, this does not

    unsigned char OEMscan = (unsigned char)((lParam & 0x00FF0000) >> 16);

    vccKeyboardHandleKey((unsigned char)wParam, OEMscan, kEventKeyUp);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MouseMove(LPARAM lParam) {
    RECT clientSize;

    VccState* vccState = GetVccState();

    if (vccState->SystemState.EmulationRunning)
    {
      unsigned int x = LOWORD(lParam);
      unsigned int y = HIWORD(lParam);

      GetClientRect(vccState->SystemState.WindowHandle, &clientSize);

      x /= ((clientSize.right - clientSize.left) >> 6);
      y /= (((clientSize.bottom - clientSize.top) - 20) >> 6);

      SetJoystick(x, y);
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl ToggleOnOff() {
    VccState* vccState = GetVccState();

    vccState->SystemState.EmulationRunning = !vccState->SystemState.EmulationRunning;

    if (vccState->SystemState.EmulationRunning) {
      vccState->SystemState.ResetPending = 2;
    }
    else {
      SetStatusBarText("", &(vccState->SystemState));
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl ToggleFullScreen() {
    VccState* vccState = GetVccState();

    if (vccState->FlagEmuStop == TH_RUNNING)
    {
      vccState->FlagEmuStop = TH_REQWAIT;
      vccState->SystemState.FullScreen = !vccState->SystemState.FullScreen;
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl KeyDown(WPARAM wParam, LPARAM lParam) {
    unsigned char OEMscan = (unsigned char)((lParam & 0x00FF0000) >> 16); // just get the scan code

    VccState* vccState = GetVccState();

    // send other keystrokes to the emulator if it is active
    if (vccState->SystemState.EmulationRunning)
    {
      vccKeyboardHandleKey((unsigned char)wParam, OEMscan, kEventKeyDown);

      // Save key down in case focus is lost
      SaveLastTwoKeyDownEvents((unsigned char)wParam, OEMscan);
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl ToggleMonitorType() {
    SetMonitorType(!SetMonitorType(QUERY));
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl ToggleThrottle() {
    SetSpeedThrottle(!SetSpeedThrottle(QUERY));
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl ToggleInfoBand() {
    SetInfoBand(!SetInfoBand(QUERY));
    InvalidateBorder();
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SlowDown() {
    VccState* vccState = GetVccState();

    DecreaseOverclockSpeed(&(vccState->SystemState));
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SpeedUp() {
    VccState* vccState = GetVccState();

    IncreaseOverclockSpeed(&(vccState->SystemState));
  }
}
