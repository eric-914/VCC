#pragma once

#include "di.version.h"
#include <dinput.h>

#define MAXSTICKS 10
#define STRLEN 64

typedef struct {
  unsigned char UseMouse;
  unsigned char Up;
  unsigned char Down;
  unsigned char Left;
  unsigned char Right;
  unsigned char Fire1;
  unsigned char Fire2;
  unsigned char DiDevice;
  unsigned char HiRes;
} JoyStick;

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

extern "C" __declspec(dllexport) JoystickState * __cdecl GetJoystickState();

extern "C" __declspec(dllexport) HRESULT __cdecl JoyStickPoll(DIJOYSTATE2*, unsigned char);

extern "C" __declspec(dllexport) bool __cdecl InitJoyStick(unsigned char);

extern "C" __declspec(dllexport) char __cdecl SetMouseStatus(char, unsigned char);

extern "C" __declspec(dllexport) char* __cdecl GetStickName(int);

extern "C" __declspec(dllexport) int __cdecl EnumerateJoysticks();

extern "C" __declspec(dllexport) unsigned short __cdecl get_pot_value(unsigned char);

extern "C" __declspec(dllexport) void __cdecl SetButtonStatus(unsigned char, unsigned char);
extern "C" __declspec(dllexport) void __cdecl SetJoystick(unsigned short, unsigned short);
extern "C" __declspec(dllexport) void __cdecl SetStickNumbers(unsigned char, unsigned char);
