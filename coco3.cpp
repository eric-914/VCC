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
#include "tcc1014graphics.h"
#include "tcc1014registers.h"
#include "mc6821.h"
#include "pakinterface.h"
#include "audio.h"
#include "coco3state.h"
#include "coco3.h"
#include "clipboard.h"
#include "throttle.h"
#include "Vcc.h"
#include "cassette.h"
#include "DirectDrawInterface.h"
#include "keyboard.h"
#include "config.h"
#include "tcc1014mmu.h"
#include "sampledefs.h"

#include "library/configdef.h"
#include "library/cpudef.h"
#include "library/defines.h"
#include "library/graphicsstate.h"
#include "library/systemstate.h"
#include "library/tcc1014graphics-8.h"
#include "library/tcc1014graphics-16.h"
#include "library/tcc1014graphics-24.h"
#include "library/tcc1014graphics-32.h"

void SetMasterTickCounter(void);

using namespace std;

_inline int CPUCycle(void);

float RenderFrame(SystemState* systemState)
{
  static unsigned short FrameCounter = 0;

  CoCoState* coco = GetCoCoState();

  //********************************Start of frame Render*****************************************************
  GetGraphicsState()->BlinkState = coco->BlinkPhase;

  irq_fs(0);				//FS low to High transition start of display Blink needs this

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

  for (systemState->LineCounter = 0; systemState->LineCounter < (coco->TopBoarder - 4); systemState->LineCounter++)
  {
    if (!(FrameCounter % systemState->FrameSkip)) {
      coco->DrawTopBoarder[systemState->BitDepth](systemState);
    }

    CPUCycle();
  }

  for (systemState->LineCounter = 0; systemState->LineCounter < coco->LinesperScreen; systemState->LineCounter++)		//Active Display area		
  {
    CPUCycle();

    if (!(FrameCounter % systemState->FrameSkip)) {
      coco->UpdateScreen[systemState->BitDepth](systemState);
    }
  }

  irq_fs(1);  //End of active display FS goes High to Low

  if (coco->VertInterruptEnabled) {
    GimeAssertVertInterrupt();
  }

  for (systemState->LineCounter = 0; systemState->LineCounter < (coco->BottomBoarder); systemState->LineCounter++)	// Bottom boarder
  {
    CPUCycle();

    if (!(FrameCounter % systemState->FrameSkip)) {
      coco->DrawBottomBoarder[systemState->BitDepth](systemState);
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

  switch (coco->SoundOutputMode)
  {
  case 0:
    FlushAudioBuffer(coco->AudioBuffer, coco->AudioIndex << 2);
    break;

  case 1:
    FlushCassetteBuffer(coco->CassBuffer, coco->AudioIndex);
    break;

  case 2:
    LoadCassetteBuffer(coco->CassBuffer);
    break;
  }

  coco->AudioIndex = 0;

  return(CalculateFPS());
}

void SetLinesperScreen(unsigned char lines)
{
  CoCoState* coco = GetCoCoState();
  GraphicsState* graphicsState = GetGraphicsState();

  lines = (lines & 3);

  coco->LinesperScreen = graphicsState->Lpf[lines];
  coco->TopBoarder = graphicsState->VcenterTable[lines];  /* graphicsstate.c */
  coco->BottomBoarder = 243 - (coco->TopBoarder + coco->LinesperScreen); //4 lines of top boarder are unrendered 244-4=240 rendered scanlines
}

_inline int CPUCycle(void)
{
  CPU* cpu = GetCPU();
  CoCoState* coco = GetCoCoState();

  if (coco->HorzInterruptEnabled) {
    GimeAssertHorzInterrupt();
  }

  irq_hs(ANY);
  PakTimer();

  coco->PicosThisLine += coco->PicosPerLine;

  while (coco->PicosThisLine > 1)
  {
    coco->StateSwitch = 0;

    if ((coco->PicosToInterrupt <= coco->PicosThisLine) && coco->IntEnable) {	//Does this iteration need to Timer Interrupt
      coco->StateSwitch = 1;
    }

    if ((coco->PicosToSoundSample <= coco->PicosThisLine) && coco->SndEnable) { //Does it need to collect an Audio sample
      coco->StateSwitch += 2;
    }

    switch (coco->StateSwitch)
    {
    case 0:		//No interrupts this line
      coco->CyclesThisLine = coco->CycleDrift + (coco->PicosThisLine * coco->CyclesPerLine * coco->OverClock / coco->PicosPerLine);

      if (coco->CyclesThisLine >= 1) {	//Avoid un-needed CPU engine calls
        coco->CycleDrift = cpu->CPUExec((int)floor(coco->CyclesThisLine)) + (coco->CyclesThisLine - floor(coco->CyclesThisLine));
      }
      else {
        coco->CycleDrift = coco->CyclesThisLine;
      }

      coco->PicosToInterrupt -= coco->PicosThisLine;
      coco->PicosToSoundSample -= coco->PicosThisLine;
      coco->PicosThisLine = 0;

      break;

    case 1:		//Only Interrupting
      coco->PicosThisLine -= coco->PicosToInterrupt;
      coco->CyclesThisLine = coco->CycleDrift + (coco->PicosToInterrupt * coco->CyclesPerLine * coco->OverClock / coco->PicosPerLine);

      if (coco->CyclesThisLine >= 1) {
        coco->CycleDrift = cpu->CPUExec((int)floor(coco->CyclesThisLine)) + (coco->CyclesThisLine - floor(coco->CyclesThisLine));
      }
      else {
        coco->CycleDrift = coco->CyclesThisLine;
      }

      GimeAssertTimerInterrupt();

      coco->PicosToSoundSample -= coco->PicosToInterrupt;
      coco->PicosToInterrupt = coco->MasterTickCounter;

      break;

    case 2:		//Only Sampling
      coco->PicosThisLine -= coco->PicosToSoundSample;
      coco->CyclesThisLine = coco->CycleDrift + (coco->PicosToSoundSample * coco->CyclesPerLine * coco->OverClock / coco->PicosPerLine);

      if (coco->CyclesThisLine >= 1) {
        coco->CycleDrift = cpu->CPUExec((int)floor(coco->CyclesThisLine)) + (coco->CyclesThisLine - floor(coco->CyclesThisLine));
      }
      else {
        coco->CycleDrift = coco->CyclesThisLine;
      }

      coco->AudioEvent();

      coco->PicosToInterrupt -= coco->PicosToSoundSample;
      coco->PicosToSoundSample = coco->SoundInterrupt;

      break;

    case 3:		//Interrupting and Sampling
      if (coco->PicosToSoundSample < coco->PicosToInterrupt)
      {
        coco->PicosThisLine -= coco->PicosToSoundSample;
        coco->CyclesThisLine = coco->CycleDrift + (coco->PicosToSoundSample * coco->CyclesPerLine * coco->OverClock / coco->PicosPerLine);

        if (coco->CyclesThisLine >= 1) {
          coco->CycleDrift = cpu->CPUExec((int)floor(coco->CyclesThisLine)) + (coco->CyclesThisLine - floor(coco->CyclesThisLine));
        }
        else {
          coco->CycleDrift = coco->CyclesThisLine;
        }

        coco->AudioEvent();

        coco->PicosToInterrupt -= coco->PicosToSoundSample;
        coco->PicosToSoundSample = coco->SoundInterrupt;
        coco->PicosThisLine -= coco->PicosToInterrupt;

        coco->CyclesThisLine = coco->CycleDrift + (coco->PicosToInterrupt * coco->CyclesPerLine * coco->OverClock / coco->PicosPerLine);

        if (coco->CyclesThisLine >= 1) {
          coco->CycleDrift = cpu->CPUExec((int)floor(coco->CyclesThisLine)) + (coco->CyclesThisLine - floor(coco->CyclesThisLine));
        }
        else {
          coco->CycleDrift = coco->CyclesThisLine;
        }

        GimeAssertTimerInterrupt();

        coco->PicosToSoundSample -= coco->PicosToInterrupt;
        coco->PicosToInterrupt = coco->MasterTickCounter;

        break;
      }

      if (coco->PicosToSoundSample > coco->PicosToInterrupt)
      {
        coco->PicosThisLine -= coco->PicosToInterrupt;
        coco->CyclesThisLine = coco->CycleDrift + (coco->PicosToInterrupt * coco->CyclesPerLine * coco->OverClock / coco->PicosPerLine);

        if (coco->CyclesThisLine >= 1) {
          coco->CycleDrift = cpu->CPUExec((int)floor(coco->CyclesThisLine)) + (coco->CyclesThisLine - floor(coco->CyclesThisLine));
        }
        else {
          coco->CycleDrift = coco->CyclesThisLine;
        }

        GimeAssertTimerInterrupt();

        coco->PicosToSoundSample -= coco->PicosToInterrupt;
        coco->PicosToInterrupt = coco->MasterTickCounter;
        coco->PicosThisLine -= coco->PicosToSoundSample;
        coco->CyclesThisLine = coco->CycleDrift + (coco->PicosToSoundSample * coco->CyclesPerLine * coco->OverClock / coco->PicosPerLine);

        if (coco->CyclesThisLine >= 1) {
          coco->CycleDrift = cpu->CPUExec((int)floor(coco->CyclesThisLine)) + (coco->CyclesThisLine - floor(coco->CyclesThisLine));
        }
        else {
          coco->CycleDrift = coco->CyclesThisLine;
        }

        coco->AudioEvent();

        coco->PicosToInterrupt -= coco->PicosToSoundSample;
        coco->PicosToSoundSample = coco->SoundInterrupt;

        break;
      }

      //They are the same (rare)
      coco->PicosThisLine -= coco->PicosToInterrupt;
      coco->CyclesThisLine = coco->CycleDrift + (coco->PicosToSoundSample * coco->CyclesPerLine * coco->OverClock / coco->PicosPerLine);

      if (coco->CyclesThisLine > 1) {
        coco->CycleDrift = cpu->CPUExec((int)floor(coco->CyclesThisLine)) + (coco->CyclesThisLine - floor(coco->CyclesThisLine));
      }
      else {
        coco->CycleDrift = coco->CyclesThisLine;
      }

      GimeAssertTimerInterrupt();

      coco->AudioEvent();

      coco->PicosToInterrupt = coco->MasterTickCounter;
      coco->PicosToSoundSample = coco->SoundInterrupt;
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
    if (coco->Throttle == 0) {
      coco->Throttle = SetSpeedThrottle(QUERY);

      if (coco->Throttle == 0) {
        coco->Throttle = 2; // 2 = No throttle.
      }
    }

    SetSpeedThrottle(0);

    if (coco->ClipCycle == 1) {
      key = PeekClipboard();

      if (key == SHIFT) {
        vccKeyboardHandleKey(SHIFT, SHIFT, kEventKeyDown);  //Press shift and...
        PopClipboard();
        key = PeekClipboard();
      }

      vccKeyboardHandleKey(key, key, kEventKeyDown);

      coco->WaitCycle = key == 0x1c ? 6000 : 2000;
    }
    else if (coco->ClipCycle == 500) {
      key = PeekClipboard();

      vccKeyboardHandleKey(SHIFT, SHIFT, kEventKeyUp);
      vccKeyboardHandleKey(0x42, key, kEventKeyUp);
      PopClipboard();

      if (ClipboardEmpty()) { //Finished?
        SetPaste(false);

        //Done pasting. Reset throttle to original state
        if (coco->Throttle == 2) {
          SetSpeedThrottle(0);
        }
        else {
          SetSpeedThrottle(1);
        }

        //...and reset the keymap to the original state
        vccKeyboardBuildRuntimeTable((keyboardlayout_e)GetCurrentKeyMap());

        coco->Throttle = 0;
      }
    }

    coco->ClipCycle++;

    if (coco->ClipCycle > coco->WaitCycle) {
      coco->ClipCycle = 1;
    }
  }

  return(0);
}

unsigned char SetSndOutMode(unsigned char mode)  //0 = Speaker 1= Cassette Out 2=Cassette In
{
  CoCoState* coco = GetCoCoState();

  static unsigned char lastMode = 0;
  static unsigned short primarySoundRate = coco->SoundRate;

  switch (mode)
  {
  case 0:
    if (lastMode == 1) {	//Send the last bits to be encoded
      FlushCassetteBuffer(coco->CassBuffer, coco->AudioIndex); /* Cassette.cpp */
    }

    coco->AudioEvent = AudioOut;

    SetAudioRate(primarySoundRate);

    break;

  case 1:
    coco->AudioEvent = CassOut;

    primarySoundRate = coco->SoundRate;

    SetAudioRate(TAPEAUDIORATE);

    break;

  case 2:
    coco->AudioEvent = CassIn;

    primarySoundRate = coco->SoundRate;

    SetAudioRate(TAPEAUDIORATE);

    break;

  default:	//QUERY
    return(coco->SoundOutputMode);
    break;
  }

  if (mode != lastMode)
  {
    coco->AudioIndex = 0;	//Reset Buffer on true mode switch
    lastMode = mode;
  }

  coco->SoundOutputMode = mode;

  return(coco->SoundOutputMode);
}
