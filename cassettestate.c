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

#include "cassettestate.h"

#include "library/cassettedef.h"

const char FileType = 0;

const int LastTrans = 0;

const unsigned char Byte = 0;
const unsigned char LastSample = 0;
const unsigned char Mask = 0;
const unsigned char MotorState = 0;
const unsigned char Quiet = 30;
const unsigned char TapeMode = STOP;
const unsigned char WriteProtect = 0;

const unsigned int TempIndex = 0;

const unsigned long BytesMoved = 0;
const unsigned long TapeOffset = 0;
const unsigned long TotalSize = 0;

const unsigned char One[21] = { 0x80,0xA8,0xC8,0xE8,0xE8,0xF8,0xF8,0xE8,0xC8,0xA8,0x78,0x50,0x50,0x30,0x10,0x00,0x00,0x10,0x30,0x30,0x50 };
const unsigned char Zero[40] = { 0x80,0x90,0xA8,0xB8,0xC8,0xD8,0xE8,0xE8,0xF0,0xF8,0xF8,0xF8,0xF0,0xE8,0xD8,0xC8,0xB8,0xA8,0x90,0x78,0x78,0x68,0x50,0x40,0x30,0x20,0x10,0x08,0x00,0x00,0x00,0x08,0x10,0x10,0x20,0x30,0x40,0x50,0x68,0x68 };

CassetteState* InitializeInstance(CassetteState* cassetteState);

static CassetteState* instance = InitializeInstance(new CassetteState());

extern "C" {
  __declspec(dllexport) CassetteState* __cdecl GetCassetteState() {
    return instance;
  }
}

CassetteState* InitializeInstance(CassetteState* c) {
  c->Byte = Byte;
  c->BytesMoved = BytesMoved;
  c->FileType = FileType;
  c->LastSample = LastSample;
  c->LastTrans = LastTrans;
  c->Mask = Mask;
  c->MotorState = MotorState;
  c->Quiet = Quiet;
  c->TapeMode = TapeMode;
  c->TapeOffset = TapeOffset;
  c->TempIndex = TempIndex;
  c->TotalSize = TotalSize;
  c->WriteProtect = WriteProtect;

  c->CasBuffer = NULL;
  c->TapeHandle = NULL;

  strcpy(c->TapeFileName, "");

  for (int i = 0; i<sizeof(One); i++) {
    c->One[i] = One[i];
  }

  for (int i = 0; i<sizeof(Zero); i++) {
    c->Zero[i] = Zero[i];
  }

  return c;
}