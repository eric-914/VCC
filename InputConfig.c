#include <windows.h>

#include "configstate.h"
#include "resource.h"

#include "library/keyboarddef.h"

LRESULT CALLBACK InputConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  ConfigState* configState = GetConfigState();

  switch (message)
  {
  case WM_INITDIALOG:
    // copy keyboard layout names to the pull-down menu
    for (int x = 0; x < kKBLayoutCount; x++)
    {
      SendDlgItemMessage(hDlg, IDC_KBCONFIG, CB_ADDSTRING, (WPARAM)0, (LPARAM)k_keyboardLayoutNames[x]);
    }

    // select the current layout
    SendDlgItemMessage(hDlg, IDC_KBCONFIG, CB_SETCURSEL, (WPARAM)(configState->CurrentConfig.KeyMap), (LPARAM)0);
    break;

  case WM_COMMAND:
    configState->TempConfig.KeyMap = (unsigned char)SendDlgItemMessage(hDlg, IDC_KBCONFIG, CB_GETCURSEL, 0, 0);
    break;
  }

  return(0);
}
