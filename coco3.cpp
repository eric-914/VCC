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

#include <string>
#include <iostream>
#include <math.h>

#include "windows.h"
#include "cpudef.h"
#include "tcc1014graphics.h"
#include "tcc1014registers.h"
#include "mc6821.h"
#include "pakinterface.h"
#include "audio.h"
#include "coco3.h"
#include "clipboard.h"
#include "throttle.h"
#include "Vcc.h"
#include "cassette.h"
#include "DirectDrawInterface.h"
#include "keyboard.h"
#include "config.h"
#include "tcc1014mmu.h"

#include "library/configdef.h"
#include "library/defines.h"
#include "library/systemstate.h"

static double SoundInterrupt = 0;
static double PicosToSoundSample = SoundInterrupt;
static double CyclesPerSecord = (COLORBURST / 4) * (TARGETFRAMERATE / FRAMESPERSECORD);
static double LinesPerSecond = (double)TARGETFRAMERATE * (double)LINESPERFIELD;
static double PicosPerLine = PICOSECOND / LinesPerSecond;
static double CyclesPerLine = CyclesPerSecord / LinesPerSecond;
static double CycleDrift = 0;
static double CyclesThisLine = 0;
static unsigned int StateSwitch = 0;
unsigned short SoundRate = 0;

static unsigned char HorzInterruptEnabled = 0, VertInterruptEnabled = 0;
static unsigned char TopBoarder = 0, BottomBoarder = 0;
static unsigned char LinesperScreen;
static unsigned char TimerInterruptEnabled = 0;
static int MasterTimer = 0;
static unsigned short TimerClockRate = 0;
static int TimerCycleCount = 0;
static double MasterTickCounter = 0, UnxlatedTickCounter = 0, OldMaster = 0;
static double PicosThisLine = 0;
static unsigned char BlinkPhase = 1;
static unsigned int AudioBuffer[16384];
static unsigned char CassBuffer[8192];
static unsigned short AudioIndex = 0;
double PicosToInterrupt = 0;
static int IntEnable = 0;
static int SndEnable = 1;
static int OverClock = 1;
static unsigned char SoundOutputMode = 0;	//Default to Speaker 1=Cassette

static int clipcycle = 1, cyclewait = 2000;
char tmpthrottle = 0;
void AudioOut(void);
void CassOut(void);
void CassIn(void);
void (*AudioEvent)(void) = AudioOut;
void SetMasterTickCounter(void);
void (*DrawTopBoarder[4]) (SystemState*) = { DrawTopBoarder8, DrawTopBoarder16, DrawTopBoarder24, DrawTopBoarder32 };
void (*DrawBottomBoarder[4]) (SystemState*) = { DrawBottomBoarder8, DrawBottomBoarder16, DrawBottomBoarder24, DrawBottomBoarder32 };
void (*UpdateScreen[4]) (SystemState*) = { UpdateScreen8, UpdateScreen16, UpdateScreen24, UpdateScreen32 };

using namespace std;

_inline int CPUCycle(void);

float RenderFrame(SystemState* systemState)
{
  static unsigned short FrameCounter = 0;

  //********************************Start of frame Render*****************************************************
  SetBlinkState(BlinkPhase);
  irq_fs(0);				//FS low to High transition start of display Boink needs this

  for (systemState->LineCounter = 0; systemState->LineCounter < 13; systemState->LineCounter++) {		//Vertical Blanking 13 H lines
    CPUCycle();
  }

  for (systemState->LineCounter = 0; systemState->LineCounter < 4; systemState->LineCounter++) {		//4 non-Rendered top Boarder lines
    CPUCycle();
  }

  if (!(FrameCounter % systemState->FrameSkip)) {
    if (LockScreen(systemState)) {
      return(0);
    }
  }

  for (systemState->LineCounter = 0; systemState->LineCounter < (TopBoarder - 4); systemState->LineCounter++)
  {
    if (!(FrameCounter % systemState->FrameSkip)) {
      DrawTopBoarder[systemState->BitDepth](systemState);
    }

    CPUCycle();
  }

  for (systemState->LineCounter = 0; systemState->LineCounter < LinesperScreen; systemState->LineCounter++)		//Active Display area		
  {
    CPUCycle();

    if (!(FrameCounter % systemState->FrameSkip)) {
      UpdateScreen[systemState->BitDepth](systemState);
    }
  }

  irq_fs(1);  //End of active display FS goes High to Low

  if (VertInterruptEnabled) {
    GimeAssertVertInterrupt();
  }

  for (systemState->LineCounter = 0; systemState->LineCounter < (BottomBoarder); systemState->LineCounter++)	// Bottom boarder
  {
    CPUCycle();

    if (!(FrameCounter % systemState->FrameSkip)) {
      DrawBottomBoarder[systemState->BitDepth](systemState);
    }
  }

  if (!(FrameCounter % systemState->FrameSkip))
  {
    UnlockScreen(systemState);
    SetBoarderChange(0);
  }

  for (systemState->LineCounter = 0; systemState->LineCounter < 6; systemState->LineCounter++) {		//Vertical Retrace 6 H lines
    CPUCycle();
  }

  switch (SoundOutputMode)
  {
  case 0:
    FlushAudioBuffer(AudioBuffer, AudioIndex << 2);
    break;

  case 1:
    FlushCassetteBuffer(CassBuffer, AudioIndex);
    break;

  case 2:
    LoadCassetteBuffer(CassBuffer);
    break;
  }

  AudioIndex = 0;

  return(CalculateFPS());
}

void SetClockSpeed(unsigned short cycles)
{
  OverClock = cycles;
}

void SetHorzInterruptState(unsigned char state)
{
  HorzInterruptEnabled = !!state;
}

void SetVertInterruptState(unsigned char state)
{
  VertInterruptEnabled = !!state;
}

void SetLinesperScreen(unsigned char lines)
{
  lines = (lines & 3);
  LinesperScreen = GetLpf(lines);
  TopBoarder = GetVcenterTable(lines);
  BottomBoarder = 243 - (TopBoarder + LinesperScreen); //4 lines of top boarder are unrendered 244-4=240 rendered scanlines
}

_inline int CPUCycle(void)
{
  if (HorzInterruptEnabled) {
    GimeAssertHorzInterrupt();
  }

  irq_hs(ANY);
  PakTimer();
  PicosThisLine += PicosPerLine;

  while (PicosThisLine > 1)
  {
    StateSwitch = 0;

    if ((PicosToInterrupt <= PicosThisLine) && IntEnable) {	//Does this iteration need to Timer Interrupt
      StateSwitch = 1;
    }

    if ((PicosToSoundSample <= PicosThisLine) && SndEnable) { //Does it need to collect an Audio sample
      StateSwitch += 2;
    }

    switch (StateSwitch)
    {
    case 0:		//No interrupts this line
      CyclesThisLine = CycleDrift + (PicosThisLine * CyclesPerLine * OverClock / PicosPerLine);

      if (CyclesThisLine >= 1) {	//Avoid un-needed CPU engine calls
        CycleDrift = CPUExec((int)floor(CyclesThisLine)) + (CyclesThisLine - floor(CyclesThisLine));
      }
      else {
        CycleDrift = CyclesThisLine;
      }

      PicosToInterrupt -= PicosThisLine;
      PicosToSoundSample -= PicosThisLine;
      PicosThisLine = 0;
      break;

    case 1:		//Only Interrupting
      PicosThisLine -= PicosToInterrupt;
      CyclesThisLine = CycleDrift + (PicosToInterrupt * CyclesPerLine * OverClock / PicosPerLine);

      if (CyclesThisLine >= 1) {
        CycleDrift = CPUExec((int)floor(CyclesThisLine)) + (CyclesThisLine - floor(CyclesThisLine));
      }
      else {
        CycleDrift = CyclesThisLine;
      }

      GimeAssertTimerInterrupt();
      PicosToSoundSample -= PicosToInterrupt;
      PicosToInterrupt = MasterTickCounter;
      break;

    case 2:		//Only Sampling
      PicosThisLine -= PicosToSoundSample;
      CyclesThisLine = CycleDrift + (PicosToSoundSample * CyclesPerLine * OverClock / PicosPerLine);

      if (CyclesThisLine >= 1) {
        CycleDrift = CPUExec((int)floor(CyclesThisLine)) + (CyclesThisLine - floor(CyclesThisLine));
      }
      else {
        CycleDrift = CyclesThisLine;
      }

      AudioEvent();
      PicosToInterrupt -= PicosToSoundSample;
      PicosToSoundSample = SoundInterrupt;
      break;

    case 3:		//Interrupting and Sampling
      if (PicosToSoundSample < PicosToInterrupt)
      {
        PicosThisLine -= PicosToSoundSample;
        CyclesThisLine = CycleDrift + (PicosToSoundSample * CyclesPerLine * OverClock / PicosPerLine);

        if (CyclesThisLine >= 1) {
          CycleDrift = CPUExec((int)floor(CyclesThisLine)) + (CyclesThisLine - floor(CyclesThisLine));
        }
        else {
          CycleDrift = CyclesThisLine;
        }

        AudioEvent();
        PicosToInterrupt -= PicosToSoundSample;
        PicosToSoundSample = SoundInterrupt;
        PicosThisLine -= PicosToInterrupt;

        CyclesThisLine = CycleDrift + (PicosToInterrupt * CyclesPerLine * OverClock / PicosPerLine);

        if (CyclesThisLine >= 1) {
          CycleDrift = CPUExec((int)floor(CyclesThisLine)) + (CyclesThisLine - floor(CyclesThisLine));
        }
        else {
          CycleDrift = CyclesThisLine;
        }

        GimeAssertTimerInterrupt();
        PicosToSoundSample -= PicosToInterrupt;
        PicosToInterrupt = MasterTickCounter;
        break;
      }

      if (PicosToSoundSample > PicosToInterrupt)
      {
        PicosThisLine -= PicosToInterrupt;
        CyclesThisLine = CycleDrift + (PicosToInterrupt * CyclesPerLine * OverClock / PicosPerLine);

        if (CyclesThisLine >= 1) {
          CycleDrift = CPUExec((int)floor(CyclesThisLine)) + (CyclesThisLine - floor(CyclesThisLine));
        }
        else {
          CycleDrift = CyclesThisLine;
        }

        GimeAssertTimerInterrupt();
        PicosToSoundSample -= PicosToInterrupt;
        PicosToInterrupt = MasterTickCounter;
        PicosThisLine -= PicosToSoundSample;
        CyclesThisLine = CycleDrift + (PicosToSoundSample * CyclesPerLine * OverClock / PicosPerLine);

        if (CyclesThisLine >= 1) {
          CycleDrift = CPUExec((int)floor(CyclesThisLine)) + (CyclesThisLine - floor(CyclesThisLine));
        }
        else {
          CycleDrift = CyclesThisLine;
        }

        AudioEvent();
        PicosToInterrupt -= PicosToSoundSample;
        PicosToSoundSample = SoundInterrupt;
        break;
      }

      //They are the same (rare)
      PicosThisLine -= PicosToInterrupt;
      CyclesThisLine = CycleDrift + (PicosToSoundSample * CyclesPerLine * OverClock / PicosPerLine);

      if (CyclesThisLine > 1) {
        CycleDrift = CPUExec((int)floor(CyclesThisLine)) + (CyclesThisLine - floor(CyclesThisLine));
      }
      else {
        CycleDrift = CyclesThisLine;
      }

      GimeAssertTimerInterrupt();
      AudioEvent();
      PicosToInterrupt = MasterTickCounter;
      PicosToSoundSample = SoundInterrupt;
    }
  }

  if (!ClipboardEmpty()) {
    char tmp[] = { 0x00 };
    char kbstate = 2;
    int z = 0;
    char key;
    const char SHIFT = 0x36;

    //Remember the original throttle setting.
    //Set it to off. We need speed for this!
    if (tmpthrottle == 0) {
      tmpthrottle = SetSpeedThrottle(QUERY);

      if (tmpthrottle == 0) { 
        tmpthrottle = 2; // 2 = No throttle.
      } 
    }

    SetSpeedThrottle(0);

    if (clipcycle == 1) {
      key = PeekClipboard();

      if (key == SHIFT) {
        vccKeyboardHandleKey(SHIFT, SHIFT, kEventKeyDown);  //Press shift and...
        PopClipboard();
        key = PeekClipboard();
      }

      vccKeyboardHandleKey(key, key, kEventKeyDown);

      if (key == 0x1c) {
        cyclewait = 6000;
      }
      else {
        cyclewait = 2000;
      }
    }
    else if (clipcycle == 500) {
      key = PeekClipboard();

      vccKeyboardHandleKey(SHIFT, SHIFT, kEventKeyUp);
      vccKeyboardHandleKey(0x42, key, kEventKeyUp);
      PopClipboard();

      if (ClipboardEmpty()) { //Finished?
        SetPaste(false);

        //Done pasting. Reset throttle to original state
        if (tmpthrottle == 2) { 
          SetSpeedThrottle(0); 
        }
        else { 
          SetSpeedThrottle(1); 
        }

        //...and reset the keymap to the original state
        vccKeyboardBuildRuntimeTable((keyboardlayout_e)GetCurrentKeyMap());
        tmpthrottle = 0;
      }
    }

    clipcycle++;

    if (clipcycle > cyclewait) { 
      clipcycle = 1; 
    }
  }

  return(0);
}

void SetTimerInterruptState(unsigned char state)
{
  TimerInterruptEnabled = state;
}

void SetInterruptTimer(unsigned short timer)
{
  UnxlatedTickCounter = (timer & 0xFFF);
  SetMasterTickCounter();
}

void SetTimerClockRate(unsigned char clockRate)	//1= 279.265nS (1/ColorBurst)
{											                    //0= 63.695uS  (1/60*262)  1 scanline time
  TimerClockRate = !!clockRate;
  SetMasterTickCounter();
}

void SetMasterTickCounter(void)
{
  double Rate[2] = { PICOSECOND / (TARGETFRAMERATE * LINESPERFIELD), PICOSECOND / COLORBURST };

  if (UnxlatedTickCounter == 0) {
    MasterTickCounter = 0;
  }
  else {
    MasterTickCounter = (UnxlatedTickCounter + 2) * Rate[TimerClockRate];
  }

  if (MasterTickCounter != OldMaster)
  {
    OldMaster = MasterTickCounter;
    PicosToInterrupt = MasterTickCounter;
  }

  if (MasterTickCounter != 0) {
    IntEnable = 1;
  }
  else {
    IntEnable = 0;
  }
}

void MiscReset(void)
{
  HorzInterruptEnabled = 0;
  VertInterruptEnabled = 0;
  TimerInterruptEnabled = 0;
  MasterTimer = 0;
  TimerClockRate = 0;
  MasterTickCounter = 0;
  UnxlatedTickCounter = 0;
  OldMaster = 0;

  SoundInterrupt = 0;
  PicosToSoundSample = SoundInterrupt;
  CycleDrift = 0;
  CyclesThisLine = 0;
  PicosThisLine = 0;
  IntEnable = 0;
  AudioIndex = 0;
  ResetAudio();
}

unsigned short SetAudioRate(unsigned short rate)
{
  SndEnable = 1;
  SoundInterrupt = 0;
  CycleDrift = 0;
  AudioIndex = 0;

  if (rate != 0) {	//Force Mute or 44100Hz
    rate = 44100;
  }

  if (rate == 0) {
    SndEnable = 0;
  }
  else
  {
    SoundInterrupt = PICOSECOND / rate;
    PicosToSoundSample = SoundInterrupt;
  }

  SoundRate = rate;

  return(0);
}

void AudioOut(void)
{
  AudioBuffer[AudioIndex++] = GetDACSample();
}

void CassOut(void)
{
  CassBuffer[AudioIndex++] = GetCasSample();
}

void CassIn(void)
{
  AudioBuffer[AudioIndex] = GetDACSample();
  SetCassetteSample(CassBuffer[AudioIndex++]);
}

unsigned char SetSndOutMode(unsigned char mode)  //0 = Speaker 1= Cassette Out 2=Cassette In
{
  static unsigned char LastMode = 0;
  static unsigned short PrimarySoundRate = SoundRate;

  switch (mode)
  {
  case 0:
    if (LastMode == 1) {	//Send the last bits to be encoded
      FlushCassetteBuffer(CassBuffer, AudioIndex);
    }

    AudioEvent = AudioOut;
    SetAudioRate(PrimarySoundRate);

    break;

  case 1:
    AudioEvent = CassOut;
    PrimarySoundRate = SoundRate;
    SetAudioRate(TAPEAUDIORATE);
    break;

  case 2:
    AudioEvent = CassIn;
    PrimarySoundRate = SoundRate;;
    SetAudioRate(TAPEAUDIORATE);
    break;

  default:	//QUERY
    return(SoundOutputMode);
    break;
  }

  if (mode != LastMode)
  {
    AudioIndex = 0;	//Reset Buffer on true mode switch
    LastMode = mode;
  }

  SoundOutputMode = mode;

  return(SoundOutputMode);
}
