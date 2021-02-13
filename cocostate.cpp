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

#include "coco3state.h"
#include "sampledefs.h"

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
const unsigned char TopBoarder = 0;
const unsigned char BottomBoarder = 0;
const unsigned char TimerInterruptEnabled = 0;
const unsigned char BlinkPhase = 1;

CoCoState* InitializeInstance(CoCoState* j);

static CoCoState* instance = InitializeInstance(new CoCoState());

extern "C" {
  __declspec(dllexport) CoCoState* __cdecl GetCoCoState() {
    return instance;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetClockSpeed(unsigned short cycles)
  {
    CoCoState* coco = GetCoCoState();

    coco->OverClock = cycles;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetHorzInterruptState(unsigned char state)
  {
    CoCoState* coco = GetCoCoState();

    coco->HorzInterruptEnabled = !!state;
  }
}

extern "C" {
  __declspec(dllexport) unsigned short __cdecl SetAudioRate(unsigned short rate)
  {
    instance->SndEnable = 1;
    instance->SoundInterrupt = 0;
    instance->CycleDrift = 0;
    instance->AudioIndex = 0;

    if (rate != 0) {	//Force Mute or 44100Hz
      rate = 44100;
    }

    if (rate == 0) {
      instance->SndEnable = 0;
    }
    else
    {
      instance->SoundInterrupt = PICOSECOND / rate;
      instance->PicosToSoundSample = instance->SoundInterrupt;
    }

    instance->SoundRate = rate;

    return(0);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetMasterTickCounter(void)
  {
    double Rate[2] = { PICOSECOND / (TARGETFRAMERATE * LINESPERFIELD), PICOSECOND / COLORBURST };

    CoCoState* coco = GetCoCoState();

    if (coco->UnxlatedTickCounter == 0) {
      coco->MasterTickCounter = 0;
    }
    else {
      coco->MasterTickCounter = (coco->UnxlatedTickCounter + 2) * Rate[coco->TimerClockRate];
    }

    if (coco->MasterTickCounter != coco->OldMaster)
    {
      coco->OldMaster = coco->MasterTickCounter;
      coco->PicosToInterrupt = coco->MasterTickCounter;
    }

    coco->IntEnable = coco->MasterTickCounter == 0 ? 0 : 1;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetTimerInterruptState(unsigned char state)
  {
    CoCoState* coco = GetCoCoState();

    coco->TimerInterruptEnabled = state;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetInterruptTimer(unsigned short timer)
  {
    instance->UnxlatedTickCounter = (timer & 0xFFF);

    SetMasterTickCounter();
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetVertInterruptState(unsigned char state)
  {
    instance->VertInterruptEnabled = !!state;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetTimerClockRate(unsigned char clockRate)
  {
    //1= 279.265nS (1/ColorBurst)
    //0= 63.695uS  (1/60*262)  1 scanline time

    instance->TimerClockRate = !!clockRate;

    SetMasterTickCounter();
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl AudioOut(void)
  {
    CoCoState* coco = GetCoCoState();

    coco->AudioBuffer[coco->AudioIndex++] = GetDACSample(); /* mc6821.c */
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl CassOut(void)
  {
    CoCoState* coco = GetCoCoState();

    coco->CassBuffer[coco->AudioIndex++] = GetCasSample(); /* mc6821.c */
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl CassIn(void)
  {
    CoCoState* coco = GetCoCoState();

    coco->AudioBuffer[coco->AudioIndex] = GetDACSample(); /* mc6821.c */

    SetCassetteSample(coco->CassBuffer[coco->AudioIndex++]);  /* mc6821.c */
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl CocoReset(void)
  {
    instance->HorzInterruptEnabled = 0;
    instance->VertInterruptEnabled = 0;
    instance->TimerInterruptEnabled = 0;
    instance->MasterTimer = 0;
    instance->TimerClockRate = 0;
    instance->MasterTickCounter = 0;
    instance->UnxlatedTickCounter = 0;
    instance->OldMaster = 0;

    instance->SoundInterrupt = 0;
    instance->PicosToSoundSample = 0;
    instance->CycleDrift = 0;
    instance->CyclesThisLine = 0;
    instance->PicosThisLine = 0;
    instance->IntEnable = 0;
    instance->AudioIndex = 0;
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
  c->TopBoarder = TopBoarder;
  c->BottomBoarder = BottomBoarder;
  c->TimerInterruptEnabled = TimerInterruptEnabled;
  c->BlinkPhase = BlinkPhase;

  c->CyclesPerSecord = (COLORBURST / 4) * (TARGETFRAMERATE / FRAMESPERSECORD);
  c->LinesPerSecond = (double)TARGETFRAMERATE * (double)LINESPERFIELD;
  c->PicosPerLine = PICOSECOND / c->LinesPerSecond;
  c->CyclesPerLine = c->CyclesPerSecord / c->LinesPerSecond;

  c->AudioEvent = AudioOut;

  c->DrawTopBoarder[0] = DrawTopBoarder8;
  c->DrawTopBoarder[1] = DrawTopBoarder16;
  c->DrawTopBoarder[2] = DrawTopBoarder24;
  c->DrawTopBoarder[3] = DrawTopBoarder32;

  c->DrawBottomBoarder[0] = DrawBottomBoarder8;
  c->DrawBottomBoarder[1] = DrawBottomBoarder16;
  c->DrawBottomBoarder[2] = DrawBottomBoarder24;
  c->DrawBottomBoarder[3] = DrawBottomBoarder32;

  c->UpdateScreen[0] = UpdateScreen8;
  c->UpdateScreen[1] = UpdateScreen16;
  c->UpdateScreen[2] = UpdateScreen24;
  c->UpdateScreen[3] = UpdateScreen32;

  return c;
}