#include <windows.h>

#include "VCC.h"
#include "Joystick.h"
#include "MessageHandlers.h"

#include "../resources/resource.h"

#include "ProcessCommandMessage.h"
#include "ProcessKeyDownMessage.h"

//----------------------------------------------------------------------------------------
//	lParam bits
//	  0-15	The repeat count for the current message. The value is the number of times
//			the keystroke is autorepeated as a result of the user holding down the key.
//			If the keystroke is held long enough, multiple messages are sent. However,
//			the repeat count is not cumulative.
//	 16-23	The scan code. The value depends on the OEM.
//	    24	Indicates whether the key is an extended key, such as the right-hand ALT and
//			CTRL keys that appear on an enhanced 101- or 102-key keyboard. The value is
//			one if it is an extended key; otherwise, it is zero.
//	 25-28	Reserved; do not use.
//	    29	The context code. The value is always zero for a WM_KEYDOWN message.
//	    30	The previous key state. The value is one if the key is down before the
//	   		message is sent, or it is zero if the key is up.
//	    31	The transition state. The value is always zero for a WM_KEYDOWN message.
//----------------------------------------------------------------------------------------

extern "C" {
  __declspec(dllexport) void __cdecl ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    VccState* vccState = GetVccState();

    switch (message)
    {
    case WM_CLOSE:        EmuExit();                                break;
    case WM_COMMAND:      ProcessCommandMessage(hWnd, wParam);      break;
    case WM_CREATE:       CreateMainMenu(hWnd);                     break;
    case WM_KEYDOWN:      ProcessKeyDownMessage(wParam, lParam);    break;
    case WM_KEYUP:        KeyUp(wParam, lParam);                    break;
    case WM_KILLFOCUS:    SendSavedKeyEvents();                     break;
    case WM_LBUTTONDOWN:  SetButtonStatus(0, 1);                    break;
    case WM_LBUTTONUP:    SetButtonStatus(0, 0);                    break;
    case WM_MOUSEMOVE:    MouseMove(lParam);                        break;
    case WM_RBUTTONDOWN:  SetButtonStatus(1, 1);                    break;
    case WM_RBUTTONUP:    SetButtonStatus(1, 0);                    break;
    case WM_SYSCOMMAND:   ProcessSysCommandMessage(hWnd, wParam);   break;
    case WM_SYSKEYDOWN:   ProcessSysKeyDownMessage(wParam, lParam); break;
    case WM_SYSKEYUP:     KeyUp(wParam, lParam);                    break;
    }
  }
}