#include "configstate.h"

#include "library/joystickstate.h"
#include "library/joystickinput.h"

#include "library/JoystickAccessors.h"

void RefreshJoystickStatus(void)
{
  bool temp = false;

  ConfigState* configState = GetConfigState();
  JoystickState* joystickState = GetJoystickState();

  configState->NumberofJoysticks = EnumerateJoysticks();

  for (unsigned char index = 0; index < configState->NumberofJoysticks; index++) {
    temp = InitJoyStick(index);
  }

  if (joystickState->Right.DiDevice > (configState->NumberofJoysticks - 1)) {
    joystickState->Right.DiDevice = 0;
  }

  if (joystickState->Left.DiDevice > (configState->NumberofJoysticks - 1)) {
    joystickState->Left.DiDevice = 0;
  }

  SetStickNumbers(joystickState->Left.DiDevice, joystickState->Right.DiDevice);

  if (configState->NumberofJoysticks == 0)	//Use Mouse input if no Joysticks present
  {
    if (joystickState->Left.UseMouse == 3) {
      joystickState->Left.UseMouse = 1;
    }

    if (joystickState->Right.UseMouse == 3) {
      joystickState->Right.UseMouse = 1;
    }
  }
}

