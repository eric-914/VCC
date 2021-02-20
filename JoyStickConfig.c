#include <windows.h>

#include "library/Config.h"
#include "resources/resource.h"
#include "library/vccstate.h"

#include "GetKeyName.h"

#include "library/joystickstate.h"
#include "library/joystickinput.h"
#include "library/systemstate.h"

LRESULT CALLBACK JoyStickConfig(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static int LeftJoyStick[6] = { IDC_LEFT_LEFT, IDC_LEFT_RIGHT, IDC_LEFT_UP, IDC_LEFT_DOWN, IDC_LEFT_FIRE1, IDC_LEFT_FIRE2 };
  static int RightJoyStick[6] = { IDC_RIGHT_LEFT, IDC_RIGHT_RIGHT, IDC_RIGHT_UP, IDC_RIGHT_DOWN, IDC_RIGHT_FIRE1, IDC_RIGHT_FIRE2 };
  static int LeftRadios[4] = { IDC_LEFT_KEYBOARD, IDC_LEFT_USEMOUSE, IDC_LEFTAUDIO, IDC_LEFTJOYSTICK };
  static int RightRadios[4] = { IDC_RIGHT_KEYBOARD, IDC_RIGHT_USEMOUSE, IDC_RIGHTAUDIO, IDC_RIGHTJOYSTICK };

  ConfigState* configState = GetConfigState();
  JoystickState* joystickState = GetJoystickState();
  VccState* vccState = GetVccState();

  switch (message)
  {
  case WM_INITDIALOG:
    configState->JoystickIcons[0] = LoadIcon(vccState->EmuState.Resources, (LPCTSTR)IDI_KEYBOARD);
    configState->JoystickIcons[1] = LoadIcon(vccState->EmuState.Resources, (LPCTSTR)IDI_MOUSE);
    configState->JoystickIcons[2] = LoadIcon(vccState->EmuState.Resources, (LPCTSTR)IDI_AUDIO);
    configState->JoystickIcons[3] = LoadIcon(vccState->EmuState.Resources, (LPCTSTR)IDI_JOYSTICK);

    for (unsigned char temp = 0; temp < 68; temp++)
    {
      for (unsigned char temp2 = 0; temp2 < 6; temp2++)
      {
        SendDlgItemMessage(hDlg, LeftJoyStick[temp2], CB_ADDSTRING, (WPARAM)0, (LPARAM)GetKeyName(temp));
        SendDlgItemMessage(hDlg, RightJoyStick[temp2], CB_ADDSTRING, (WPARAM)0, (LPARAM)GetKeyName(temp));
      }
    }

    for (unsigned char temp = 0; temp < 6; temp++)
    {
      EnableWindow(GetDlgItem(hDlg, LeftJoyStick[temp]), (joystickState->Left.UseMouse == 0));
    }

    for (unsigned char temp = 0; temp < 6; temp++)
    {
      EnableWindow(GetDlgItem(hDlg, RightJoyStick[temp]), (joystickState->Right.UseMouse == 0));
    }

    for (unsigned char temp = 0; temp <= 2; temp++)
    {
      SendDlgItemMessage(hDlg, configState->LeftJoystickEmulation[temp], BM_SETCHECK, (temp == joystickState->Left.HiRes), 0);
      SendDlgItemMessage(hDlg, configState->RightJoystickEmulation[temp], BM_SETCHECK, (temp == joystickState->Right.HiRes), 0);
    }

    EnableWindow(GetDlgItem(hDlg, IDC_LEFTAUDIODEVICE), (joystickState->Left.UseMouse == 2));
    EnableWindow(GetDlgItem(hDlg, IDC_RIGHTAUDIODEVICE), (joystickState->Right.UseMouse == 2));
    EnableWindow(GetDlgItem(hDlg, IDC_LEFTJOYSTICKDEVICE), (joystickState->Left.UseMouse == 3));
    EnableWindow(GetDlgItem(hDlg, IDC_RIGHTJOYSTICKDEVICE), (joystickState->Right.UseMouse == 3));

    EnableWindow(GetDlgItem(hDlg, IDC_LEFTJOYSTICK), (configState->NumberofJoysticks > 0));		//Grey the Joystick Radios if
    EnableWindow(GetDlgItem(hDlg, IDC_RIGHTJOYSTICK), (configState->NumberofJoysticks > 0));	  //No Joysticks are present

    //populate joystick combo boxs
    for (unsigned char index = 0; index < configState->NumberofJoysticks; index++)
    {
      SendDlgItemMessage(hDlg, IDC_RIGHTJOYSTICKDEVICE, CB_ADDSTRING, (WPARAM)0, (LPARAM)GetStickName(index));
      SendDlgItemMessage(hDlg, IDC_LEFTJOYSTICKDEVICE, CB_ADDSTRING, (WPARAM)0, (LPARAM)GetStickName(index));
    }

    SendDlgItemMessage(hDlg, IDC_RIGHTJOYSTICKDEVICE, CB_SETCURSEL, (WPARAM)joystickState->Right.DiDevice, (LPARAM)0);
    SendDlgItemMessage(hDlg, IDC_LEFTJOYSTICKDEVICE, CB_SETCURSEL, (WPARAM)joystickState->Left.DiDevice, (LPARAM)0);

    SendDlgItemMessage(hDlg, LeftJoyStick[0], CB_SETCURSEL, (WPARAM)TranslateScan2Display(joystickState->Left.Left), (LPARAM)0);
    SendDlgItemMessage(hDlg, LeftJoyStick[1], CB_SETCURSEL, (WPARAM)TranslateScan2Display(joystickState->Left.Right), (LPARAM)0);
    SendDlgItemMessage(hDlg, LeftJoyStick[2], CB_SETCURSEL, (WPARAM)TranslateScan2Display(joystickState->Left.Up), (LPARAM)0);
    SendDlgItemMessage(hDlg, LeftJoyStick[3], CB_SETCURSEL, (WPARAM)TranslateScan2Display(joystickState->Left.Down), (LPARAM)0);
    SendDlgItemMessage(hDlg, LeftJoyStick[4], CB_SETCURSEL, (WPARAM)TranslateScan2Display(joystickState->Left.Fire1), (LPARAM)0);
    SendDlgItemMessage(hDlg, LeftJoyStick[5], CB_SETCURSEL, (WPARAM)TranslateScan2Display(joystickState->Left.Fire2), (LPARAM)0);
    SendDlgItemMessage(hDlg, RightJoyStick[0], CB_SETCURSEL, (WPARAM)TranslateScan2Display(joystickState->Right.Left), (LPARAM)0);
    SendDlgItemMessage(hDlg, RightJoyStick[1], CB_SETCURSEL, (WPARAM)TranslateScan2Display(joystickState->Right.Right), (LPARAM)0);
    SendDlgItemMessage(hDlg, RightJoyStick[2], CB_SETCURSEL, (WPARAM)TranslateScan2Display(joystickState->Right.Up), (LPARAM)0);
    SendDlgItemMessage(hDlg, RightJoyStick[3], CB_SETCURSEL, (WPARAM)TranslateScan2Display(joystickState->Right.Down), (LPARAM)0);
    SendDlgItemMessage(hDlg, RightJoyStick[4], CB_SETCURSEL, (WPARAM)TranslateScan2Display(joystickState->Right.Fire1), (LPARAM)0);
    SendDlgItemMessage(hDlg, RightJoyStick[5], CB_SETCURSEL, (WPARAM)TranslateScan2Display(joystickState->Right.Fire2), (LPARAM)0);

    for (unsigned char temp = 0; temp <= 3; temp++)
    {
      SendDlgItemMessage(hDlg, LeftRadios[temp], BM_SETCHECK, temp == joystickState->Left.UseMouse, 0);
    }

    for (unsigned char temp = 0; temp <= 3; temp++)
    {
      SendDlgItemMessage(hDlg, RightRadios[temp], BM_SETCHECK, temp == joystickState->Right.UseMouse, 0);
    }

    configState->Left = joystickState->Left;
    configState->Right = joystickState->Right;

    SendDlgItemMessage(hDlg, IDC_LEFTICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(configState->JoystickIcons[joystickState->Left.UseMouse]));
    SendDlgItemMessage(hDlg, IDC_RIGHTICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(configState->JoystickIcons[joystickState->Right.UseMouse]));
    break;

  case WM_COMMAND:
    for (unsigned char temp = 0; temp <= 3; temp++)
    {
      if (LOWORD(wParam) == LeftRadios[temp])
      {
        for (unsigned char temp2 = 0; temp2 <= 3; temp2++) {
          SendDlgItemMessage(hDlg, LeftRadios[temp2], BM_SETCHECK, 0, 0);
        }

        SendDlgItemMessage(hDlg, LeftRadios[temp], BM_SETCHECK, 1, 0);

        configState->Left.UseMouse = temp;
      }
    }

    for (unsigned char temp = 0; temp <= 3; temp++) {
      if (LOWORD(wParam) == RightRadios[temp])
      {
        for (unsigned char temp2 = 0; temp2 <= 3; temp2++) {
          SendDlgItemMessage(hDlg, RightRadios[temp2], BM_SETCHECK, 0, 0);
        }

        SendDlgItemMessage(hDlg, RightRadios[temp], BM_SETCHECK, 1, 0);

        configState->Right.UseMouse = temp;
      }
    }

    for (unsigned char temp = 0; temp <= 2; temp++) {
      if (LOWORD(wParam) == configState->LeftJoystickEmulation[temp])
      {
        for (unsigned char temp2 = 0; temp2 <= 2; temp2++) {
          SendDlgItemMessage(hDlg, configState->LeftJoystickEmulation[temp2], BM_SETCHECK, 0, 0);
        }

        SendDlgItemMessage(hDlg, configState->LeftJoystickEmulation[temp], BM_SETCHECK, 1, 0);

        configState->Left.HiRes = temp;
      }
    }

    for (unsigned char temp = 0; temp <= 2; temp++)
    {
      if (LOWORD(wParam) == configState->RightJoystickEmulation[temp])
      {
        for (unsigned char temp2 = 0; temp2 <= 2; temp2++) {
          SendDlgItemMessage(hDlg, configState->RightJoystickEmulation[temp2], BM_SETCHECK, 0, 0);
        }

        SendDlgItemMessage(hDlg, configState->RightJoystickEmulation[temp], BM_SETCHECK, 1, 0);

        configState->Right.HiRes = temp;
      }
    }

    for (unsigned char temp = 0; temp < 6; temp++)
    {
      EnableWindow(GetDlgItem(hDlg, LeftJoyStick[temp]), (configState->Left.UseMouse == 0));
    }

    for (unsigned char temp = 0; temp < 6; temp++)
    {
      EnableWindow(GetDlgItem(hDlg, RightJoyStick[temp]), (configState->Right.UseMouse == 0));
    }

    EnableWindow(GetDlgItem(hDlg, IDC_LEFTAUDIODEVICE), (configState->Left.UseMouse == 2));
    EnableWindow(GetDlgItem(hDlg, IDC_RIGHTAUDIODEVICE), (configState->Right.UseMouse == 2));
    EnableWindow(GetDlgItem(hDlg, IDC_LEFTJOYSTICKDEVICE), (configState->Left.UseMouse == 3));
    EnableWindow(GetDlgItem(hDlg, IDC_RIGHTJOYSTICKDEVICE), (configState->Right.UseMouse == 3));

    configState->Left.Left = TranslateDisplay2Scan(SendDlgItemMessage(hDlg, LeftJoyStick[0], CB_GETCURSEL, 0, 0));
    configState->Left.Right = TranslateDisplay2Scan(SendDlgItemMessage(hDlg, LeftJoyStick[1], CB_GETCURSEL, 0, 0));
    configState->Left.Up = TranslateDisplay2Scan(SendDlgItemMessage(hDlg, LeftJoyStick[2], CB_GETCURSEL, 0, 0));
    configState->Left.Down = TranslateDisplay2Scan(SendDlgItemMessage(hDlg, LeftJoyStick[3], CB_GETCURSEL, 0, 0));
    configState->Left.Fire1 = TranslateDisplay2Scan(SendDlgItemMessage(hDlg, LeftJoyStick[4], CB_GETCURSEL, 0, 0));
    configState->Left.Fire2 = TranslateDisplay2Scan(SendDlgItemMessage(hDlg, LeftJoyStick[5], CB_GETCURSEL, 0, 0));

    configState->Right.Left = TranslateDisplay2Scan(SendDlgItemMessage(hDlg, RightJoyStick[0], CB_GETCURSEL, 0, 0));
    configState->Right.Right = TranslateDisplay2Scan(SendDlgItemMessage(hDlg, RightJoyStick[1], CB_GETCURSEL, 0, 0));
    configState->Right.Up = TranslateDisplay2Scan(SendDlgItemMessage(hDlg, RightJoyStick[2], CB_GETCURSEL, 0, 0));
    configState->Right.Down = TranslateDisplay2Scan(SendDlgItemMessage(hDlg, RightJoyStick[3], CB_GETCURSEL, 0, 0));
    configState->Right.Fire1 = TranslateDisplay2Scan(SendDlgItemMessage(hDlg, RightJoyStick[4], CB_GETCURSEL, 0, 0));
    configState->Right.Fire2 = TranslateDisplay2Scan(SendDlgItemMessage(hDlg, RightJoyStick[5], CB_GETCURSEL, 0, 0));

    configState->Right.DiDevice = (unsigned char)SendDlgItemMessage(hDlg, IDC_RIGHTJOYSTICKDEVICE, CB_GETCURSEL, 0, 0);
    configState->Left.DiDevice = (unsigned char)SendDlgItemMessage(hDlg, IDC_LEFTJOYSTICKDEVICE, CB_GETCURSEL, 0, 0);	//Fix Me;

    SendDlgItemMessage(hDlg, IDC_LEFTICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(configState->JoystickIcons[configState->Left.UseMouse]));
    SendDlgItemMessage(hDlg, IDC_RIGHTICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(configState->JoystickIcons[configState->Right.UseMouse]));

    break;
  }

  return(0);
}
