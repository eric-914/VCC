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

#include "cocostate.h"

#include "AudioOut.h" //--TROUBLE!

#include "library/defines.h"
#include "library/tcc1014graphics-8.h"
#include "library/tcc1014graphics-16.h"
#include "library/tcc1014graphics-24.h"
#include "library/tcc1014graphics-32.h"

const double SoundInterrupt = 0;
const double PicosToSoundSample = SoundInterrupt;
const double CycleDrift = 0;
const double CyclesThisLine = 0;
const unsigned int StateSwitch = 0;
const unsigned short SoundRate = 0;
const unsigned char SoundOutputMode = 0;	//Default to Speaker 1=Cassette
const unsigned short AudioIndex = 0;
const double PicosToInterrupt = 0;
const int MasterTimer = 0;
const double OldMaster = 0;
const unsigned short TimerClockRate = 0;
const int TimerCycleCount = 0;
const double MasterTickCounter = 0;
const double UnxlatedTickCounter = 0;
const double PicosThisLine = 0;
const char Throttle = 0;
const int ClipCycle = 1;
const int WaitCycle = 2000;
const int IntEnable = 0;
const int SndEnable = 1;
const int OverClock = 1;
const unsigned char HorzInterruptEnabled = 0;
const unsigned char VertInterruptEnabled = 0;
const unsigned char TopBorder = 0;
const unsigned char BottomBorder = 0;
const unsigned char TimerInterruptEnabled = 0;
const unsigned char BlinkPhase = 1;

CoCoState* InitializeInstance(CoCoState* coco);

static CoCoState* instance = InitializeInstance(new CoCoState());

extern "C" {
  __declspec(dllexport) CoCoState* __cdecl GetCoCoState() {
    return instance;
  }
}

CoCoState* InitializeInstance(CoCoState* c) {
  c->SoundInterrupt = SoundInterrupt;
  c->PicosToSoundSample = PicosToSoundSample;
  c->CycleDrift = CycleDrift;
  c->CyclesThisLine = CyclesThisLine;
  c->StateSwitch = StateSwitch;
  c->SoundRate = SoundRate;
  c->SoundOutputMode = SoundOutputMode;
  c->AudioIndex = AudioIndex;
  c->PicosToInterrupt = PicosToInterrupt;
  c->MasterTimer = MasterTimer;
  c->OldMaster = OldMaster;
  c->TimerClockRate = TimerClockRate;
  c->TimerCycleCount = TimerCycleCount;
  c->MasterTickCounter = MasterTickCounter;
  c->UnxlatedTickCounter = UnxlatedTickCounter;
  c->PicosThisLine = PicosThisLine;
  c->Throttle = Throttle;
  c->ClipCycle = ClipCycle;
  c->WaitCycle = WaitCycle;
  c->IntEnable = IntEnable;
  c->SndEnable = SndEnable;
  c->OverClock = OverClock;
  c->HorzInterruptEnabled = HorzInterruptEnabled;
  c->VertInterruptEnabled = VertInterruptEnabled;
  c->TopBorder = TopBorder;
  c->BottomBorder = BottomBorder;
  c->TimerInterruptEnabled = TimerInterruptEnabled;
  c->BlinkPhase = BlinkPhase;

  c->CyclesPerSecord = (COLORBURST / 4) * (TARGETFRAMERATE / FRAMESPERSECORD);
  c->LinesPerSecond = (double)TARGETFRAMERATE * (double)LINESPERFIELD;
  c->PicosPerLine = PICOSECOND / c->LinesPerSecond;
  c->CyclesPerLine = c->CyclesPerSecord / c->LinesPerSecond;

  c->AudioEvent = AudioOut;

  c->DrawTopBorder[0] = DrawTopBorder8;
  c->DrawTopBorder[1] = DrawTopBorder16;
  c->DrawTopBorder[2] = DrawTopBorder24;
  c->DrawTopBorder[3] = DrawTopBorder32;

  c->DrawBottomBorder[0] = DrawBottomBorder8;
  c->DrawBottomBorder[1] = DrawBottomBorder16;
  c->DrawBottomBorder[2] = DrawBottomBorder24;
  c->DrawBottomBorder[3] = DrawBottomBorder32;

  c->UpdateScreen[0] = UpdateScreen8;
  c->UpdateScreen[1] = UpdateScreen16;
  c->UpdateScreen[2] = UpdateScreen24;
  c->UpdateScreen[3] = UpdateScreen32;

  return c;
}