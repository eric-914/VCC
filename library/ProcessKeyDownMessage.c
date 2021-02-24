#include <windows.h>

#include "VCC.h"
#include "Config.h"
#include "MessageHandlers.h"

extern "C" {
  __declspec(dllexport) void __cdecl ProcessKeyDownMessage(WPARAM wParam, LPARAM lParam) {
    // get key scan code for emulator control keys
    unsigned char OEMscan = (unsigned char)((lParam & 0x00FF0000) >> 16); // just get the scan code

    VccState* vccState = GetVccState();

    switch (OEMscan)
    {
    case DIK_F3:    SlowDown();           break;
    case DIK_F4:    SpeedUp();            break;
    case DIK_F5:    EmuReset(1);          break;
    case DIK_F6:    ToggleMonitorType();  break;
    case DIK_F8:    ToggleThrottle();     break;
    case DIK_F9:    ToggleOnOff();        break;
    case DIK_F10:   ToggleInfoBand();     break;
    case DIK_F11:   ToggleFullScreen();   break;

    default:        KeyDown(wParam, lParam);
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl ProcessSysKeyDownMessage(WPARAM wParam, LPARAM lParam) {
    // Ignore repeated system keys
    if (!(lParam >> 30)) {
      ProcessKeyDownMessage(wParam, lParam);
    }
  }
}