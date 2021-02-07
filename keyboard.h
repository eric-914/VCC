#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

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
/*****************************************************************************/

#include "library\keyboarddef.h"

#ifdef __cplusplus
extern "C"
{
#endif

  void vccKeyboardBuildRuntimeTable(keyboardlayout_e keyBoardLayout);
  void vccKeyboardHandleKey(unsigned char, unsigned char, keyevent_e keyState);
  unsigned char	vccKeyboardGetScan(unsigned char);

  // globals referenced from config.c
  extern JoyStick	Left;
  extern JoyStick Right;

  void joystick(unsigned short, unsigned short);
  unsigned short	get_pot_value(unsigned char pot);
  void SetButtonStatus(unsigned char, unsigned char);
  void SetStickNumbers(unsigned char, unsigned char);

#ifdef __cplusplus
}
#endif

#endif // __KEYBOARD_H__

bool GetPaste();
void SetPaste(bool);
