#include <windows.h>
#include <string>
#include <ShlObj.h>

#include "configstate.h"
#include "resources/resource.h"

LRESULT CALLBACK DisplayConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static bool isRGB;

  ConfigState* configState = GetConfigState();

  switch (message)
  {
  case WM_INITDIALOG:

    SendDlgItemMessage(hDlg, IDC_FRAMESKIP, TBM_SETRANGE, TRUE, MAKELONG(1, 6));
    SendDlgItemMessage(hDlg, IDC_SCANLINES, BM_SETCHECK, configState->TempConfig.ScanLines, 0);
    SendDlgItemMessage(hDlg, IDC_THROTTLE, BM_SETCHECK, configState->TempConfig.SpeedThrottle, 0);
    SendDlgItemMessage(hDlg, IDC_FRAMESKIP, TBM_SETPOS, TRUE, configState->TempConfig.FrameSkip);
    SendDlgItemMessage(hDlg, IDC_RESIZE, BM_SETCHECK, configState->TempConfig.Resize, 0);
    SendDlgItemMessage(hDlg, IDC_ASPECT, BM_SETCHECK, configState->TempConfig.Aspect, 0);
    SendDlgItemMessage(hDlg, IDC_REMEMBER_SIZE, BM_SETCHECK, configState->TempConfig.RememberSize, 0);

    sprintf(configState->OutBuffer, "%i", configState->TempConfig.FrameSkip);

    SendDlgItemMessage(hDlg, IDC_FRAMEDISPLAY, WM_SETTEXT, strlen(configState->OutBuffer), (LPARAM)(LPCSTR)(configState->OutBuffer));

    for (unsigned char temp = 0; temp <= 1; temp++) {
      SendDlgItemMessage(hDlg, configState->Monchoice[temp], BM_SETCHECK, (temp == configState->TempConfig.MonitorType), 0);
    }

    if (configState->TempConfig.MonitorType == 1) { //If RGB monitor is chosen, gray out palette choice
      isRGB = TRUE;

      SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETSTATE, 1, 0);
      SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETSTATE, 1, 0);
      SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETDONTCLICK, 1, 0);
      SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETDONTCLICK, 1, 0);
    }

    SendDlgItemMessage(hDlg, IDC_MONTYPE, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(configState->MonIcons[configState->TempConfig.MonitorType]));

    for (unsigned char temp = 0; temp <= 1; temp++) {
      SendDlgItemMessage(hDlg, configState->PaletteChoice[temp], BM_SETCHECK, (temp == configState->TempConfig.PaletteType), 0);
    }

    break;

  case WM_HSCROLL:
    configState->TempConfig.FrameSkip = (unsigned char)SendDlgItemMessage(hDlg, IDC_FRAMESKIP, TBM_GETPOS, (WPARAM)0, (WPARAM)0);

    sprintf(configState->OutBuffer, "%i", configState->TempConfig.FrameSkip);

    SendDlgItemMessage(hDlg, IDC_FRAMEDISPLAY, WM_SETTEXT, strlen(configState->OutBuffer), (LPARAM)(LPCSTR)(configState->OutBuffer));

    break;

  case WM_COMMAND:
    configState->TempConfig.Resize = 1;
    configState->TempConfig.Aspect = (unsigned char)SendDlgItemMessage(hDlg, IDC_ASPECT, BM_GETCHECK, 0, 0);
    configState->TempConfig.ScanLines = (unsigned char)SendDlgItemMessage(hDlg, IDC_SCANLINES, BM_GETCHECK, 0, 0);
    configState->TempConfig.SpeedThrottle = (unsigned char)SendDlgItemMessage(hDlg, IDC_THROTTLE, BM_GETCHECK, 0, 0);
    configState->TempConfig.RememberSize = (unsigned char)SendDlgItemMessage(hDlg, IDC_REMEMBER_SIZE, BM_GETCHECK, 0, 0);

    //POINT p = { 640,480 };
    switch (LOWORD(wParam))
    {
    case IDC_REMEMBER_SIZE:
      configState->TempConfig.Resize = 1;

      SendDlgItemMessage(hDlg, IDC_RESIZE, BM_GETCHECK, 1, 0);

      break;

    case IDC_COMPOSITE:
      isRGB = FALSE;
      for (unsigned char temp = 0; temp <= 1; temp++) { //This finds the current Monitor choice, then sets both buttons in the nested loop.
        if (LOWORD(wParam) == configState->Monchoice[temp])
        {
          for (unsigned char temp2 = 0; temp2 <= 1; temp2++) {
            SendDlgItemMessage(hDlg, configState->Monchoice[temp2], BM_SETCHECK, 0, 0);
          }

          SendDlgItemMessage(hDlg, configState->Monchoice[temp], BM_SETCHECK, 1, 0);

          configState->TempConfig.MonitorType = temp;
        }
      }

      SendDlgItemMessage(hDlg, IDC_MONTYPE, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(configState->MonIcons[configState->TempConfig.MonitorType]));
      SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETSTATE, 0, 0);
      SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETSTATE, 0, 0);
      break;

    case IDC_RGB:
      isRGB = TRUE;

      for (unsigned char temp = 0; temp <= 1; temp++) { //This finds the current Monitor choice, then sets both buttons in the nested loop.
        if (LOWORD(wParam) == configState->Monchoice[temp])
        {
          for (unsigned char temp2 = 0; temp2 <= 1; temp2++) {
            SendDlgItemMessage(hDlg, configState->Monchoice[temp2], BM_SETCHECK, 0, 0);
          }

          SendDlgItemMessage(hDlg, configState->Monchoice[temp], BM_SETCHECK, 1, 0);

          configState->TempConfig.MonitorType = temp;
        }
      }

      SendDlgItemMessage(hDlg, IDC_MONTYPE, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(configState->MonIcons[configState->TempConfig.MonitorType]));
      //If RGB is chosen, disable palette buttons.
      SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETSTATE, 1, 0);
      SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETSTATE, 1, 0);
      SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETDONTCLICK, 1, 0);
      SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETDONTCLICK, 1, 0);
      break;

    case IDC_ORG_PALETTE:
      if (!isRGB) {
        //Original Composite palette
        SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETCHECK, 1, 0);
        SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETCHECK, 0, 0);
        configState->TempConfig.PaletteType = 0;
      }
      else {
        SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETSTATE, 1, 0);
      }
      break;

    case IDC_UPD_PALETTE:
      if (!isRGB) {
        //New Composite palette
        SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETCHECK, 1, 0);
        SendDlgItemMessage(hDlg, IDC_ORG_PALETTE, BM_SETCHECK, 0, 0);
        configState->TempConfig.PaletteType = 1;
      }
      else {
        SendDlgItemMessage(hDlg, IDC_UPD_PALETTE, BM_SETSTATE, 1, 0);
      }
      break;
    }
    break;
  }

  return(0);
}
