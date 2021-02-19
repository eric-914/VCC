#pragma once

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

#include <windows.h>

typedef struct
{
  HANDLE TapeHandle;

  char CassPath[MAX_PATH];
  char FileType;
  char TapeFileName[MAX_PATH];

  int LastTrans;

  unsigned char Byte;
  unsigned char LastSample;
  unsigned char Mask;
  unsigned char MotorState;
  unsigned char One[21];
  unsigned char Quiet;
  unsigned char TapeMode;
  unsigned char TempBuffer[8192];
  unsigned char WriteProtect;
  unsigned char Zero[40];

  unsigned char* CasBuffer;

  unsigned int TempIndex;

  unsigned long BytesMoved;
  unsigned long TapeOffset;
  unsigned long TotalSize;

} CassetteState;

extern "C" __declspec(dllexport) CassetteState * __cdecl GetCassetteState();
