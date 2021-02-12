/*****************************************************************************/
/*
Copyright 2015 by Joseph Forgione
This file is part of VCC (Virtual Color Computer).

    VCC (Virtual Color Computer) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    VCC (Virtual Color Computer) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with VCC (Virtual Color Computer).  If not, see <http://www.gnu.org/licenses/>.
*/

#include "joystickstate.h"

const static unsigned short StickValue = 0;
const static unsigned short LeftStickX = 32;
const static unsigned short LeftStickY = 32;
const static unsigned short RightStickX = 32;
const static unsigned short RightStickY = 32;

const static unsigned char LeftButton1Status = 0;
const static unsigned char RightButton1Status = 0;
const static unsigned char LeftButton2Status = 0;
const static unsigned char RightButton2Status = 0;
const static unsigned char LeftStickNumber = 0;
const static unsigned char RightStickNumber = 0;

JoystickState* InitializeInstance(JoystickState* j);

static JoystickState* instance = InitializeInstance(new JoystickState());

extern "C" {
  __declspec(dllexport) JoystickState* __cdecl GetJoystickState() {
    return instance;
  }
}

JoystickState* InitializeInstance(JoystickState* j) {
  j->StickValue = StickValue;
  j->LeftStickX = LeftStickX;
  j->LeftStickY = LeftStickY;
  j->RightStickX = RightStickX;
  j->RightStickY = RightStickY;

  j->LeftButton1Status = LeftButton1Status;
  j->RightButton1Status = RightButton1Status;
  j->LeftButton2Status = LeftButton2Status;
  j->RightButton2Status = RightButton2Status;
  j->LeftStickNumber = LeftStickNumber;
  j->RightStickNumber = RightStickNumber;

  return j;
}