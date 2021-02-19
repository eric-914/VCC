#include <windows.h>

#include "library/vccstate.h"
#include "resource.h"

// Mesage handler for the About box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  VccState* vccState = GetVccState();

  switch (message)
  {
  case WM_INITDIALOG:
    SendDlgItemMessage(hDlg, IDC_TITLE, WM_SETTEXT, strlen(vccState->AppName), (LPARAM)(LPCSTR)(vccState->AppName));

    return TRUE;

  case WM_COMMAND:
    if (LOWORD(wParam) == IDOK)
    {
      EndDialog(hDlg, LOWORD(wParam));

      return TRUE;
    }

    break;
  }

  return FALSE;
}
