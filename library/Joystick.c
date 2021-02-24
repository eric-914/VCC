#include "Joystick.h"

static LPDIRECTINPUTDEVICE8 Joysticks[MAXSTICKS];
static unsigned char JoyStickIndex = 0;
static LPDIRECTINPUT8 di;
BOOL CALLBACK enumCallback(const DIDEVICEINSTANCE*, VOID*);
BOOL CALLBACK enumAxesCallback(const DIDEVICEOBJECTINSTANCE*, VOID*);
static unsigned char CurrentStick;

char StickName[MAXSTICKS][STRLEN];

JoystickState* InitializeInstance(JoystickState*);

static JoystickState* instance = InitializeInstance(new JoystickState());

extern "C" {
  __declspec(dllexport) JoystickState* __cdecl GetJoystickState() {
    return instance;
  }
}

JoystickState* InitializeInstance(JoystickState* p) {
  p->StickValue = 0;
  p->LeftStickX = 32;
  p->LeftStickY = 32;
  p->RightStickX = 32;
  p->RightStickY = 32;

  p->LeftButton1Status = 0;
  p->RightButton1Status = 0;
  p->LeftButton2Status = 0;
  p->RightButton2Status = 0;
  p->LeftStickNumber = 0;
  p->RightStickNumber = 0;

  return p;
}

extern "C" {
  __declspec(dllexport) char* __cdecl GetStickName(int index) {
    return StickName[index];
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl EnumerateJoysticks(void)
  {
    HRESULT hr;
    JoyStickIndex = 0;

    if (FAILED(hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&di, NULL))) {
      return(0);
    }

    if (FAILED(hr = di->EnumDevices(DI8DEVCLASS_GAMECTRL, enumCallback, NULL, DIEDFL_ATTACHEDONLY))) {
      return(0);
    }

    return(JoyStickIndex);
  }
}

BOOL CALLBACK enumCallback(const DIDEVICEINSTANCE* instance, VOID* context)
{
  HRESULT hr;

  hr = di->CreateDevice(instance->guidInstance, &Joysticks[JoyStickIndex], NULL);
  strncpy(StickName[JoyStickIndex], instance->tszProductName, STRLEN);
  JoyStickIndex++;

  return(JoyStickIndex < MAXSTICKS);
}

extern "C" {
  __declspec(dllexport) bool __cdecl InitJoyStick(unsigned char stickNumber)
  {
    //	DIDEVCAPS capabilities;
    HRESULT hr;

    CurrentStick = stickNumber;

    if (Joysticks[stickNumber] == NULL) {
      return(0);
    }

    if (FAILED(hr = Joysticks[stickNumber]->SetDataFormat(&c_dfDIJoystick2))) {
      return(0);
    }

    if (FAILED(hr = Joysticks[stickNumber]->EnumObjects(enumAxesCallback, NULL, DIDFT_AXIS))) {
      return(0);
    }

    return(1); //return true on success
  }
}

BOOL CALLBACK enumAxesCallback(const DIDEVICEOBJECTINSTANCE* instance, VOID* context)
{
  HWND hDlg = (HWND)context;
  DIPROPRANGE propRange;
  propRange.diph.dwSize = sizeof(DIPROPRANGE);
  propRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);
  propRange.diph.dwHow = DIPH_BYID;
  propRange.diph.dwObj = instance->dwType;
  propRange.lMin = 0;
  propRange.lMax = 0xFFFF;

  if (FAILED(Joysticks[CurrentStick]->SetProperty(DIPROP_RANGE, &propRange.diph))) {
    return(DIENUM_STOP);
  }

  return(DIENUM_CONTINUE);
}

extern "C" {
  __declspec(dllexport) HRESULT __cdecl JoyStickPoll(DIJOYSTATE2* js, unsigned char stickNumber)
  {
    HRESULT hr;

    if (Joysticks[stickNumber] == NULL) {
      return (S_OK);
    }

    hr = Joysticks[stickNumber]->Poll();

    if (FAILED(hr))
    {
      hr = Joysticks[stickNumber]->Acquire();

      while (hr == DIERR_INPUTLOST) {
        hr = Joysticks[stickNumber]->Acquire();
      }

      if (hr == DIERR_INVALIDPARAM) {
        return(E_FAIL);
      }

      if (hr == DIERR_OTHERAPPHASPRIO) {
        return(S_OK);
      }
    }

    if (FAILED(hr = Joysticks[stickNumber]->GetDeviceState(sizeof(DIJOYSTATE2), js))) {
      return(hr);
    }

    return(S_OK);
  }
}

/*
  Joystick related code
*/

extern "C"
{
  __declspec(dllexport) void __cdecl SetJoystick(unsigned short x, unsigned short y)
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
  __declspec(dllexport) void __cdecl SetStickNumbers(unsigned char leftStickNumber, unsigned char rightStickNumber)
  {
    JoystickState* joystickState = GetJoystickState();

    joystickState->LeftStickNumber = leftStickNumber;
    joystickState->RightStickNumber = rightStickNumber;
  }
}

extern "C"
{
  __declspec(dllexport) unsigned short __cdecl get_pot_value(unsigned char pot)
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

//0 = Left 1=right
extern "C"
{
  __declspec(dllexport) void __cdecl SetButtonStatus(unsigned char side, unsigned char state) //Side=0 Left Button Side=1 Right Button State 1=Down
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

extern "C"
{
  __declspec(dllexport) char __cdecl SetMouseStatus(char scanCode, unsigned char phase)
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
}
