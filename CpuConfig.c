#include <windows.h>
#include <string>
#include <ShlObj.h>

#include "configstate.h"
#include "resource.h"

LRESULT CALLBACK CpuConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  ConfigState* configState = GetConfigState();

  switch (message)
  {
  case WM_INITDIALOG:
    SendDlgItemMessage(hDlg, IDC_CLOCKSPEED, TBM_SETRANGE, TRUE, MAKELONG(2, configState->CurrentConfig.MaxOverclock));	//Maximum overclock

    sprintf(configState->OutBuffer, "%2.3f Mhz", (float)(configState->TempConfig.CPUMultiplyer) * .894);

    SendDlgItemMessage(hDlg, IDC_CLOCKDISPLAY, WM_SETTEXT, strlen(configState->OutBuffer), (LPARAM)(LPCSTR)(configState->OutBuffer));
    SendDlgItemMessage(hDlg, IDC_CLOCKSPEED, TBM_SETPOS, TRUE, configState->TempConfig.CPUMultiplyer);

    for (unsigned char temp = 0; temp <= 3; temp++) {
      SendDlgItemMessage(hDlg, configState->Ramchoice[temp], BM_SETCHECK, (temp == configState->TempConfig.RamSize), 0);
    }

    for (unsigned char temp = 0; temp <= 1; temp++) {
      SendDlgItemMessage(hDlg, configState->Cpuchoice[temp], BM_SETCHECK, (temp == configState->TempConfig.CpuType), 0);
    }

    SendDlgItemMessage(hDlg, IDC_CPUICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(configState->CpuIcons[configState->TempConfig.CpuType]));

    break;

  case WM_HSCROLL:
    configState->TempConfig.CPUMultiplyer = (unsigned char)SendDlgItemMessage(hDlg, IDC_CLOCKSPEED, TBM_GETPOS, (WPARAM)0, (WPARAM)0);

    sprintf(configState->OutBuffer, "%2.3f Mhz", (float)(configState->TempConfig.CPUMultiplyer) * .894);

    SendDlgItemMessage(hDlg, IDC_CLOCKDISPLAY, WM_SETTEXT, strlen(configState->OutBuffer), (LPARAM)(LPCSTR)(configState->OutBuffer));

    break;

  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDC_128K:
    case IDC_512K:
    case IDC_2M:
    case IDC_8M:
      for (unsigned char temp = 0; temp <= 3; temp++) {
        if (LOWORD(wParam) == configState->Ramchoice[temp])
        {
          for (unsigned char temp2 = 0; temp2 <= 3; temp2++) {
            SendDlgItemMessage(hDlg, configState->Ramchoice[temp2], BM_SETCHECK, 0, 0);
          }

          SendDlgItemMessage(hDlg, configState->Ramchoice[temp], BM_SETCHECK, 1, 0);

          configState->TempConfig.RamSize = temp;
        }
      }

      break;

    case IDC_6809:
    case IDC_6309:
      for (unsigned char temp = 0; temp <= 1; temp++) {
        if (LOWORD(wParam) == configState->Cpuchoice[temp])
        {
          for (unsigned char temp2 = 0; temp2 <= 1; temp2++) {
            SendDlgItemMessage(hDlg, configState->Cpuchoice[temp2], BM_SETCHECK, 0, 0);
          }

          SendDlgItemMessage(hDlg, configState->Cpuchoice[temp], BM_SETCHECK, 1, 0);

          configState->TempConfig.CpuType = temp;

          SendDlgItemMessage(hDlg, IDC_CPUICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(configState->CpuIcons[configState->TempConfig.CpuType]));
        }
      }

      break;
    }

    break;
  }

  return(0);
}
