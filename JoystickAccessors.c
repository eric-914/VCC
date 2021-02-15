#include "library/keyboarddef.h"
#include "library/joystickstate.h"
#include "library/joystickinput.h"

/*
  Joystick related code
*/

extern "C"
{
  void SetJoystick(unsigned short x, unsigned short y)
  {
    JoystickState* joystickState = GetJoystickState();

    if (x > 63) {
      x = 63;
    }

    if (y > 63) {
      y = 63;
    }

    if (joystickState->Left.UseMouse == 1)
    {
      joystickState->LeftStickX = x;
      joystickState->LeftStickY = y;
    }

    if (joystickState->Right.UseMouse == 1)
    {
      joystickState->RightStickX = x;
      joystickState->RightStickY = y;
    }

    return;
  }
}

extern "C"
{
  void SetStickNumbers(unsigned char leftStickNumber, unsigned char rightStickNumber)
  {
    JoystickState* joystickState = GetJoystickState();

    joystickState->LeftStickNumber = leftStickNumber;
    joystickState->RightStickNumber = rightStickNumber;
  }
}

extern "C"
{
  unsigned short get_pot_value(unsigned char pot)
  {
    DIJOYSTATE2 Stick1;

    JoystickState* joystickState = GetJoystickState();

    if (joystickState->Left.UseMouse == 3)
    {
      JoyStickPoll(&Stick1, joystickState->LeftStickNumber);
      joystickState->LeftStickX = (unsigned short)Stick1.lX >> 10;
      joystickState->LeftStickY = (unsigned short)Stick1.lY >> 10;
      joystickState->LeftButton1Status = Stick1.rgbButtons[0] >> 7;
      joystickState->LeftButton2Status = Stick1.rgbButtons[1] >> 7;
    }

    if (joystickState->Right.UseMouse == 3)
    {
      JoyStickPoll(&Stick1, joystickState->RightStickNumber);
      joystickState->RightStickX = (unsigned short)Stick1.lX >> 10;
      joystickState->RightStickY = (unsigned short)Stick1.lY >> 10;
      joystickState->RightButton1Status = Stick1.rgbButtons[0] >> 7;
      joystickState->RightButton2Status = Stick1.rgbButtons[1] >> 7;
    }

    switch (pot)
    {
    case 0:
      return(joystickState->RightStickX);
      break;

    case 1:
      return(joystickState->RightStickY);
      break;

    case 2:
      return(joystickState->LeftStickX);
      break;

    case 3:
      return(joystickState->LeftStickY);
      break;
    }

    return (0);
  }
}

extern "C"
{
  void SetButtonStatus(unsigned char side, unsigned char state) //Side=0 Left Button Side=1 Right Button State 1=Down
  {
    unsigned char buttonStatus = (side << 1) | state;

    JoystickState* joystickState = GetJoystickState();

    if (joystickState->Left.UseMouse == 1)
      switch (buttonStatus)
      {
      case 0:
        joystickState->LeftButton1Status = 0;
        break;

      case 1:
        joystickState->LeftButton1Status = 1;
        break;

      case 2:
        joystickState->LeftButton2Status = 0;
        break;

      case 3:
        joystickState->LeftButton2Status = 1;
        break;
      }

    if (joystickState->Right.UseMouse == 1)
      switch (buttonStatus)
      {
      case 0:
        joystickState->RightButton1Status = 0;
        break;

      case 1:
        joystickState->RightButton1Status = 1;
        break;

      case 2:
        joystickState->RightButton2Status = 0;
        break;

      case 3:
        joystickState->RightButton2Status = 1;
        break;
      }
  }
}

char SetMouseStatus(char scanCode, unsigned char phase)
{
  char retValue = scanCode;

  JoystickState* joystickState = GetJoystickState();

  switch (phase)
  {
  case 0:
    if (joystickState->Left.UseMouse == 0)
    {
      if (scanCode == joystickState->Left.Left)
      {
        joystickState->LeftStickX = 32;
        retValue = 0;
      }

      if (scanCode == joystickState->Left.Right)
      {
        joystickState->LeftStickX = 32;
        retValue = 0;
      }

      if (scanCode == joystickState->Left.Up)
      {
        joystickState->LeftStickY = 32;
        retValue = 0;
      }

      if (scanCode == joystickState->Left.Down)
      {
        joystickState->LeftStickY = 32;
        retValue = 0;
      }

      if (scanCode == joystickState->Left.Fire1)
      {
        joystickState->LeftButton1Status = 0;
        retValue = 0;
      }

      if (scanCode == joystickState->Left.Fire2)
      {
        joystickState->LeftButton2Status = 0;
        retValue = 0;
      }
    }

    if (joystickState->Right.UseMouse == 0)
    {
      if (scanCode == joystickState->Right.Left)
      {
        joystickState->RightStickX = 32;
        retValue = 0;
      }

      if (scanCode == joystickState->Right.Right)
      {
        joystickState->RightStickX = 32;
        retValue = 0;
      }

      if (scanCode == joystickState->Right.Up)
      {
        joystickState->RightStickY = 32;
        retValue = 0;
      }

      if (scanCode == joystickState->Right.Down)
      {
        joystickState->RightStickY = 32;
        retValue = 0;
      }

      if (scanCode == joystickState->Right.Fire1)
      {
        joystickState->RightButton1Status = 0;
        retValue = 0;
      }

      if (scanCode == joystickState->Right.Fire2)
      {
        joystickState->RightButton2Status = 0;
        retValue = 0;
      }
    }
    break;

  case 1:
    if (joystickState->Left.UseMouse == 0)
    {
      if (scanCode == joystickState->Left.Left)
      {
        joystickState->LeftStickX = 0;
        retValue = 0;
      }

      if (scanCode == joystickState->Left.Right)
      {
        joystickState->LeftStickX = 63;
        retValue = 0;
      }

      if (scanCode == joystickState->Left.Up)
      {
        joystickState->LeftStickY = 0;
        retValue = 0;
      }

      if (scanCode == joystickState->Left.Down)
      {
        joystickState->LeftStickY = 63;
        retValue = 0;
      }

      if (scanCode == joystickState->Left.Fire1)
      {
        joystickState->LeftButton1Status = 1;
        retValue = 0;
      }

      if (scanCode == joystickState->Left.Fire2)
      {
        joystickState->LeftButton2Status = 1;
        retValue = 0;
      }
    }

    if (joystickState->Right.UseMouse == 0)
    {
      if (scanCode == joystickState->Right.Left)
      {
        retValue = 0;
        joystickState->RightStickX = 0;
      }

      if (scanCode == joystickState->Right.Right)
      {
        joystickState->RightStickX = 63;
        retValue = 0;
      }

      if (scanCode == joystickState->Right.Up)
      {
        joystickState->RightStickY = 0;
        retValue = 0;
      }

      if (scanCode == joystickState->Right.Down)
      {
        joystickState->RightStickY = 63;
        retValue = 0;
      }

      if (scanCode == joystickState->Right.Fire1)
      {
        joystickState->RightButton1Status = 1;
        retValue = 0;
      }

      if (scanCode == joystickState->Right.Fire2)
      {
        joystickState->RightButton2Status = 1;
        retValue = 0;
      }
    }
    break;
  }

  return(retValue);
}
