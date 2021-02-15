#include <windows.h>

#include "configstate.h"
#include "resource.h"

#include "SelectFile.h"

#include "library/systemstate.h"

extern void ClosePrintFile(void);
extern void SetMonState(BOOL);
extern void SetSerialParams(unsigned char);

extern SystemState EmuState;

LRESULT CALLBACK BitBangerConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  ConfigState* configState = GetConfigState();

  switch (message)
  {
  case WM_INITDIALOG:
    if (!strlen(configState->SerialCaptureFile)) {
      strcpy(configState->SerialCaptureFile, "No Capture File");
    }

    SendDlgItemMessage(hDlg, IDC_SERIALFILE, WM_SETTEXT, strlen(configState->SerialCaptureFile), (LPARAM)(LPCSTR)(configState->SerialCaptureFile));
    SendDlgItemMessage(hDlg, IDC_LF, BM_SETCHECK, configState->TextMode, 0);
    SendDlgItemMessage(hDlg, IDC_PRINTMON, BM_SETCHECK, configState->PrtMon, 0);

    break;

  case WM_COMMAND:

    switch (LOWORD(wParam))
    {
    case IDC_OPEN:
      SelectFile(&EmuState, configState->SerialCaptureFile);

      SendDlgItemMessage(hDlg, IDC_SERIALFILE, WM_SETTEXT, strlen(configState->SerialCaptureFile), (LPARAM)(LPCSTR)(configState->SerialCaptureFile));

      break;

    case IDC_CLOSE:
      ClosePrintFile();
      strcpy(configState->SerialCaptureFile, "No Capture File");

      SendDlgItemMessage(hDlg, IDC_SERIALFILE, WM_SETTEXT, strlen(configState->SerialCaptureFile), (LPARAM)(LPCSTR)(configState->SerialCaptureFile));

      configState->PrtMon = FALSE;

      SetMonState(configState->PrtMon);

      SendDlgItemMessage(hDlg, IDC_PRINTMON, BM_SETCHECK, configState->PrtMon, 0);

      break;

    case IDC_LF:
      configState->TextMode = (char)SendDlgItemMessage(hDlg, IDC_LF, BM_GETCHECK, 0, 0);

      SetSerialParams(configState->TextMode);

      break;

    case IDC_PRINTMON:
      configState->PrtMon = (char)SendDlgItemMessage(hDlg, IDC_PRINTMON, BM_GETCHECK, 0, 0);

      SetMonState(configState->PrtMon);
    }

    break;
  }

  return(0);
}