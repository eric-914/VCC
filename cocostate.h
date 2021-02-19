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

#include "library/systemstate.h"

typedef struct
{
  double SoundInterrupt;
  double PicosToSoundSample;
  double CycleDrift;
  double CyclesThisLine;
  double PicosToInterrupt;
  double OldMaster;
  double MasterTickCounter;
  double UnxlatedTickCounter;
  double PicosThisLine;
  double CyclesPerSecord;
  double LinesPerSecond;
  double PicosPerLine;
  double CyclesPerLine;

  unsigned char SoundOutputMode;
  unsigned char HorzInterruptEnabled;
  unsigned char VertInterruptEnabled;
  unsigned char TopBorder;
  unsigned char BottomBorder;
  unsigned char LinesperScreen;
  unsigned char TimerInterruptEnabled;
  unsigned char BlinkPhase;

  unsigned short TimerClockRate;
  unsigned short SoundRate;
  unsigned short AudioIndex;

  unsigned int StateSwitch;

  int MasterTimer;
  int TimerCycleCount;
  int ClipCycle;
  int WaitCycle;
  int IntEnable;
  int SndEnable;
  int OverClock;

  char Throttle;

  unsigned int AudioBuffer[16384];
  unsigned char CassBuffer[8192];

  void (*AudioEvent)(void);
  void (*DrawTopBorder[4]) (SystemState*);
  void (*DrawBottomBorder[4]) (SystemState*);
  void (*UpdateScreen[4]) (SystemState*);
} CoCoState;

extern "C" __declspec(dllexport) CoCoState * __cdecl GetCoCoState();
