#pragma once

#include "joystickdef.h"

typedef struct
{
  unsigned short StickValue;
  unsigned short LeftStickX;
  unsigned short LeftStickY;
  unsigned short RightStickX;
  unsigned short RightStickY;

  unsigned char LeftButton1Status;
  unsigned char RightButton1Status;
  unsigned char LeftButton2Status;
  unsigned char RightButton2Status;
  unsigned char LeftStickNumber;
  unsigned char RightStickNumber;

  JoyStick Left;
  JoyStick Right;
} JoystickState;

extern "C" __declspec(dllexport) JoystickState* __cdecl GetJoystickState();
