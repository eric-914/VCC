#ifndef __CONFIG_H__
#define __CONFIG_H__
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

#include <iostream>

#include "library/commandline.h"
#include "library/systemstate.h"

using namespace std;

LRESULT CALLBACK Config(HWND, UINT, WPARAM, LPARAM);

void LoadConfig(SystemState*, CmdLineArguments cmdArg);

char* AppDirectory();
char* BasicRomName(void);

void GetIniFilePath(char*);
void SetIniFilePath(char*);

int GetKeyboardLayout();
int GetPaletteType();
int GetRememberSize();

POINT GetIniWindowSize();
void SetWindowSize(POINT);

unsigned char WriteIniFile(void);
unsigned char ReadIniFile(void);

void UpdateConfig(void);
void UpdateSoundBar(unsigned short, unsigned short);
void UpdateTapeCounter(unsigned int, unsigned char);

void DecreaseOverclockSpeed();
void IncreaseOverclockSpeed();

void GetProfileText(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString);
void SetProfileText(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpString);
unsigned short GetProfileShort(LPCSTR lpAppName, LPCSTR lpKeyName, int nDefault);
unsigned char GetProfileByte(LPCSTR lpAppName, LPCSTR lpKeyName, int nDefault);

#endif
