#include <windows.h>
#include <string>

#include "resources/resource.h"
#include "library/Config.h"
#include "library/Cassette.h"

LRESULT CALLBACK CreateTapeConfigDialogCallback(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  ConfigState* configState = GetConfigState();

  configState->CounterText.cbSize = sizeof(CHARFORMAT);
  configState->CounterText.dwMask = CFM_BOLD | CFM_COLOR;
  configState->CounterText.dwEffects = CFE_BOLD;
  configState->CounterText.crTextColor = RGB(255, 255, 255);

  configState->ModeText.cbSize = sizeof(CHARFORMAT);
  configState->ModeText.dwMask = CFM_BOLD | CFM_COLOR;
  configState->ModeText.dwEffects = CFE_BOLD;
  configState->ModeText.crTextColor = RGB(255, 0, 0);

  switch (message)
  {
  case WM_INITDIALOG:
    configState->TapeCounter = GetTapeCounter();

    sprintf(configState->OutBuffer, "%i", configState->TapeCounter);

    SendDlgItemMessage(hDlg, IDC_TCOUNT, WM_SETTEXT, strlen(configState->OutBuffer), (LPARAM)(LPCSTR)(configState->OutBuffer));
    SendDlgItemMessage(hDlg, IDC_MODE, WM_SETTEXT, strlen(configState->Tmodes[configState->Tmode]), (LPARAM)(LPCSTR)(configState->Tmodes[configState->Tmode]));

    GetTapeName(configState->TapeFileName);

    SendDlgItemMessage(hDlg, IDC_TAPEFILE, WM_SETTEXT, strlen(configState->TapeFileName), (LPARAM)(LPCSTR)(configState->TapeFileName));
    SendDlgItemMessage(hDlg, IDC_TCOUNT, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0, 0, 0));
    SendDlgItemMessage(hDlg, IDC_TCOUNT, EM_SETCHARFORMAT, SCF_ALL, (LPARAM) & (configState->CounterText));
    SendDlgItemMessage(hDlg, IDC_MODE, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0, 0, 0));
    SendDlgItemMessage(hDlg, IDC_MODE, EM_SETCHARFORMAT, SCF_ALL, (LPARAM) & (configState->CounterText));

    configState->hDlgTape = hDlg;

    break;

  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDC_PLAY:
      configState->Tmode = PLAY;

      SetTapeMode(configState->Tmode);

      break;

    case IDC_REC:
      configState->Tmode = REC;

      SetTapeMode(configState->Tmode);

      break;

    case IDC_STOP:
      configState->Tmode = STOP;

      SetTapeMode(configState->Tmode);

      break;

    case IDC_EJECT:
      configState->Tmode = EJECT;

      SetTapeMode(configState->Tmode);

      break;

    case IDC_RESET:
      configState->TapeCounter = 0;

      SetTapeCounter(configState->TapeCounter);

      break;

    case IDC_TBROWSE:
      LoadTape();

      configState->TapeCounter = 0;

      SetTapeCounter(configState->TapeCounter);

      break;
    }

    break;
  }

  return(0);
}
