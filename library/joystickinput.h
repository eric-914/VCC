#ifndef __JOYSTICKINPUT_H__
#define __JOYSTICKINPUT_H__
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

#include "di.version.h"
#include <dinput.h>

#define MAXSTICKS 10
#define STRLEN 64

extern "C" __declspec(dllexport) char* __cdecl GetStickName(int index);

extern "C" __declspec(dllexport) HRESULT __cdecl JoyStickPoll(DIJOYSTATE2*, unsigned char);
extern "C" __declspec(dllexport) int __cdecl EnumerateJoysticks(void);
extern "C" __declspec(dllexport) bool __cdecl InitJoyStick(unsigned char);

#endif
