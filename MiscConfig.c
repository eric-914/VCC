#include <windows.h>

#include "library/Config.h"
#include "resources/resource.h"

LRESULT CALLBACK MiscConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  ConfigState* configState = GetConfigState();

  switch (message)
  {
  case WM_INITDIALOG:
    SendDlgItemMessage(hDlg, IDC_AUTOSTART, BM_SETCHECK, configState->TempConfig.AutoStart, 0);
    SendDlgItemMessage(hDlg, IDC_AUTOCART, BM_SETCHECK, configState->TempConfig.CartAutoStart, 0);

    break;

  case WM_COMMAND:
    configState->TempConfig.AutoStart = (unsigned char)SendDlgItemMessage(hDlg, IDC_AUTOSTART, BM_GETCHECK, 0, 0);
    configState->TempConfig.CartAutoStart = (unsigned char)SendDlgItemMessage(hDlg, IDC_AUTOCART, BM_GETCHECK, 0, 0);

    break;
  }

  return(0);
}
