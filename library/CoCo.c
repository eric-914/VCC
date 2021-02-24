#include "CoCo.h"

#include "defines.h"

#include "Graphics.h"
#include "tcc1014graphics-8.h"
#include "tcc1014graphics-16.h"
#include "tcc1014graphics-24.h"
#include "tcc1014graphics-32.h"

#include "MC6821.h"
#include "Cassette.h"
#include "Keyboard.h"
#include "Clipboard.h"
#include "VCC.h"
#include "Throttle.h"
#include "DirectDraw.h"

#include "cpudef.h"

#include "PAKInterface.h"
#include "TC1014Registers.h"

CoCoState* InitializeInstance(CoCoState*);

static CoCoState* instance = InitializeInstance(new CoCoState());

extern "C" {
  __declspec(dllexport) CoCoState* __cdecl GetCoCoState() {
    return instance;
  }
}

CoCoState* InitializeInstance(CoCoState* p) {
  p->AudioIndex = 0;
  p->BlinkPhase = 1;
  p->BottomBorder = 0;
  p->ClipCycle = 1;
  p->CycleDrift = 0;
  p->CyclesThisLine = 0;
  p->HorzInterruptEnabled = 0;
  p->IntEnable = 0;
  p->MasterTickCounter = 0;
  p->MasterTimer = 0;
  p->OldMaster = 0;
  p->OverClock = 1;
  p->PicosThisLine = 0;
  p->PicosToInterrupt = 0;
  p->PicosToSoundSample = 0;
  p->SndEnable = 1;
  p->SoundInterrupt = 0;
  p->SoundOutputMode = 0; //Default to Speaker 1=Cassette
  p->SoundRate = 0;
  p->StateSwitch = 0;
  p->Throttle = 0;
  p->TimerClockRate = 0;
  p->TimerCycleCount = 0;
  p->TimerInterruptEnabled = 0;
  p->TopBorder = 0;
  p->UnxlatedTickCounter = 0;
  p->VertInterruptEnabled = 0;
  p->WaitCycle = 2000;

  p->CyclesPerSecord = (COLORBURST / 4) * (TARGETFRAMERATE / FRAMESPERSECORD);
  p->LinesPerSecond = (double)TARGETFRAMERATE * (double)LINESPERFIELD;
  p->PicosPerLine = PICOSECOND / p->LinesPerSecond;
  p->CyclesPerLine = p->CyclesPerSecord / p->LinesPerSecond;

  p->AudioEvent = AudioOut;

  p->DrawTopBorder[0] = DrawTopBorder8;
  p->DrawTopBorder[1] = DrawTopBorder16;
  p->DrawTopBorder[2] = DrawTopBorder24;
  p->DrawTopBorder[3] = DrawTopBorder32;

  p->DrawBottomBorder[0] = DrawBottomBorder8;
  p->DrawBottomBorder[1] = DrawBottomBorder16;
  p->DrawBottomBorder[2] = DrawBottomBorder24;
  p->DrawBottomBorder[3] = DrawBottomBorder32;

  p->UpdateScreen[0] = UpdateScreen8;
  p->UpdateScreen[1] = UpdateScreen16;
  p->UpdateScreen[2] = UpdateScreen24;
  p->UpdateScreen[3] = UpdateScreen32;

  return p;
}

extern "C" {
  __declspec(dllexport) void __cdecl AudioOut(void)
  {
    instance->AudioBuffer[instance->AudioIndex++] = MC6821_GetDACSample();
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetVertInterruptState(unsigned char state)
  {
    instance->VertInterruptEnabled = !!state;
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

extern "C" {
  __declspec(dllexport) void __cdecl SetClockSpeed(unsigned short cycles)
  {
    instance->OverClock = cycles;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetHorzInterruptState(unsigned char state)
  {
    instance->HorzInterruptEnabled = !!state;
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

    if (instance->UnxlatedTickCounter == 0) {
      instance->MasterTickCounter = 0;
    }
    else {
      instance->MasterTickCounter = (instance->UnxlatedTickCounter + 2) * Rate[instance->TimerClockRate];
    }

    if (instance->MasterTickCounter != instance->OldMaster)
    {
      instance->OldMaster = instance->MasterTickCounter;
      instance->PicosToInterrupt = instance->MasterTickCounter;
    }

    instance->IntEnable = instance->MasterTickCounter == 0 ? 0 : 1;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetTimerInterruptState(unsigned char state)
  {
    instance->TimerInterruptEnabled = state;
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
  __declspec(dllexport) void __cdecl SetTimerClockRate(unsigned char clockRate)
  {
    //1= 279.265nS (1/ColorBurst)
    //0= 63.695uS  (1/60*262)  1 scanline time

    instance->TimerClockRate = !!clockRate;

    SetMasterTickCounter();
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetLinesperScreen(unsigned char lines)
  {
    GraphicsState* graphicsState = GetGraphicsState();

    lines = (lines & 3);

    instance->LinesperScreen = graphicsState->Lpf[lines];
    instance->TopBorder = graphicsState->VcenterTable[lines];
    instance->BottomBorder = 243 - (instance->TopBorder + instance->LinesperScreen); //4 lines of top border are unrendered 244-4=240 rendered scanlines
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl CassIn(void)
  {
    instance->AudioBuffer[instance->AudioIndex] = MC6821_GetDACSample();

    MC6821_SetCassetteSample(instance->CassBuffer[instance->AudioIndex++]);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl CassOut(void)
  {
    instance->CassBuffer[instance->AudioIndex++] = MC6821_GetCasSample();
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetSndOutMode(unsigned char mode)  //0 = Speaker 1= Cassette Out 2=Cassette In
  {
    static unsigned char lastMode = 0;
    static unsigned short primarySoundRate = instance->SoundRate;

    switch (mode)
    {
    case 0:
      if (lastMode == 1) {	//Send the last bits to be encoded
        FlushCassetteBuffer(instance->CassBuffer, instance->AudioIndex); /* Cassette.cpp */
      }

      instance->AudioEvent = AudioOut;

      SetAudioRate(primarySoundRate);

      break;

    case 1:
      instance->AudioEvent = CassOut;

      primarySoundRate = instance->SoundRate;

      SetAudioRate(TAPEAUDIORATE);

      break;

    case 2:
      instance->AudioEvent = CassIn;

      primarySoundRate = instance->SoundRate;

      SetAudioRate(TAPEAUDIORATE);

      break;

    default:	//QUERY
      return(instance->SoundOutputMode);
      break;
    }

    if (mode != lastMode)
    {
      instance->AudioIndex = 0;	//Reset Buffer on true mode switch
      lastMode = mode;
    }

    instance->SoundOutputMode = mode;

    return(instance->SoundOutputMode);
  }
}

extern "C" {
  __declspec(dllexport) /* _inline */ int __cdecl CPUCycle(void)
  {
    CPU* cpu = GetCPU();

    if (instance->HorzInterruptEnabled) {
      GimeAssertHorzInterrupt();
    }

    MC6821_irq_hs(ANY);
    PakTimer();

    instance->PicosThisLine += instance->PicosPerLine;

    while (instance->PicosThisLine > 1)
    {
      instance->StateSwitch = 0;

      if ((instance->PicosToInterrupt <= instance->PicosThisLine) && instance->IntEnable) {	//Does this iteration need to Timer Interrupt
        instance->StateSwitch = 1;
      }

      if ((instance->PicosToSoundSample <= instance->PicosThisLine) && instance->SndEnable) { //Does it need to collect an Audio sample
        instance->StateSwitch += 2;
      }

      switch (instance->StateSwitch)
      {
      case 0:		//No interrupts this line
        instance->CyclesThisLine = instance->CycleDrift + (instance->PicosThisLine * instance->CyclesPerLine * instance->OverClock / instance->PicosPerLine);

        if (instance->CyclesThisLine >= 1) {	//Avoid un-needed CPU engine calls
          instance->CycleDrift = cpu->CPUExec((int)floor(instance->CyclesThisLine)) + (instance->CyclesThisLine - floor(instance->CyclesThisLine));
        }
        else {
          instance->CycleDrift = instance->CyclesThisLine;
        }

        instance->PicosToInterrupt -= instance->PicosThisLine;
        instance->PicosToSoundSample -= instance->PicosThisLine;
        instance->PicosThisLine = 0;

        break;

      case 1:		//Only Interrupting
        instance->PicosThisLine -= instance->PicosToInterrupt;
        instance->CyclesThisLine = instance->CycleDrift + (instance->PicosToInterrupt * instance->CyclesPerLine * instance->OverClock / instance->PicosPerLine);

        if (instance->CyclesThisLine >= 1) {
          instance->CycleDrift = cpu->CPUExec((int)floor(instance->CyclesThisLine)) + (instance->CyclesThisLine - floor(instance->CyclesThisLine));
        }
        else {
          instance->CycleDrift = instance->CyclesThisLine;
        }

        GimeAssertTimerInterrupt();

        instance->PicosToSoundSample -= instance->PicosToInterrupt;
        instance->PicosToInterrupt = instance->MasterTickCounter;

        break;

      case 2:		//Only Sampling
        instance->PicosThisLine -= instance->PicosToSoundSample;
        instance->CyclesThisLine = instance->CycleDrift + (instance->PicosToSoundSample * instance->CyclesPerLine * instance->OverClock / instance->PicosPerLine);

        if (instance->CyclesThisLine >= 1) {
          instance->CycleDrift = cpu->CPUExec((int)floor(instance->CyclesThisLine)) + (instance->CyclesThisLine - floor(instance->CyclesThisLine));
        }
        else {
          instance->CycleDrift = instance->CyclesThisLine;
        }

        instance->AudioEvent();

        instance->PicosToInterrupt -= instance->PicosToSoundSample;
        instance->PicosToSoundSample = instance->SoundInterrupt;

        break;

      case 3:		//Interrupting and Sampling
        if (instance->PicosToSoundSample < instance->PicosToInterrupt)
        {
          instance->PicosThisLine -= instance->PicosToSoundSample;
          instance->CyclesThisLine = instance->CycleDrift + (instance->PicosToSoundSample * instance->CyclesPerLine * instance->OverClock / instance->PicosPerLine);

          if (instance->CyclesThisLine >= 1) {
            instance->CycleDrift = cpu->CPUExec((int)floor(instance->CyclesThisLine)) + (instance->CyclesThisLine - floor(instance->CyclesThisLine));
          }
          else {
            instance->CycleDrift = instance->CyclesThisLine;
          }

          instance->AudioEvent();

          instance->PicosToInterrupt -= instance->PicosToSoundSample;
          instance->PicosToSoundSample = instance->SoundInterrupt;
          instance->PicosThisLine -= instance->PicosToInterrupt;

          instance->CyclesThisLine = instance->CycleDrift + (instance->PicosToInterrupt * instance->CyclesPerLine * instance->OverClock / instance->PicosPerLine);

          if (instance->CyclesThisLine >= 1) {
            instance->CycleDrift = cpu->CPUExec((int)floor(instance->CyclesThisLine)) + (instance->CyclesThisLine - floor(instance->CyclesThisLine));
          }
          else {
            instance->CycleDrift = instance->CyclesThisLine;
          }

          GimeAssertTimerInterrupt();

          instance->PicosToSoundSample -= instance->PicosToInterrupt;
          instance->PicosToInterrupt = instance->MasterTickCounter;

          break;
        }

        if (instance->PicosToSoundSample > instance->PicosToInterrupt)
        {
          instance->PicosThisLine -= instance->PicosToInterrupt;
          instance->CyclesThisLine = instance->CycleDrift + (instance->PicosToInterrupt * instance->CyclesPerLine * instance->OverClock / instance->PicosPerLine);

          if (instance->CyclesThisLine >= 1) {
            instance->CycleDrift = cpu->CPUExec((int)floor(instance->CyclesThisLine)) + (instance->CyclesThisLine - floor(instance->CyclesThisLine));
          }
          else {
            instance->CycleDrift = instance->CyclesThisLine;
          }

          GimeAssertTimerInterrupt();

          instance->PicosToSoundSample -= instance->PicosToInterrupt;
          instance->PicosToInterrupt = instance->MasterTickCounter;
          instance->PicosThisLine -= instance->PicosToSoundSample;
          instance->CyclesThisLine = instance->CycleDrift + (instance->PicosToSoundSample * instance->CyclesPerLine * instance->OverClock / instance->PicosPerLine);

          if (instance->CyclesThisLine >= 1) {
            instance->CycleDrift = cpu->CPUExec((int)floor(instance->CyclesThisLine)) + (instance->CyclesThisLine - floor(instance->CyclesThisLine));
          }
          else {
            instance->CycleDrift = instance->CyclesThisLine;
          }

          instance->AudioEvent();

          instance->PicosToInterrupt -= instance->PicosToSoundSample;
          instance->PicosToSoundSample = instance->SoundInterrupt;

          break;
        }

        //They are the same (rare)
        instance->PicosThisLine -= instance->PicosToInterrupt;
        instance->CyclesThisLine = instance->CycleDrift + (instance->PicosToSoundSample * instance->CyclesPerLine * instance->OverClock / instance->PicosPerLine);

        if (instance->CyclesThisLine > 1) {
          instance->CycleDrift = cpu->CPUExec((int)floor(instance->CyclesThisLine)) + (instance->CyclesThisLine - floor(instance->CyclesThisLine));
        }
        else {
          instance->CycleDrift = instance->CyclesThisLine;
        }

        GimeAssertTimerInterrupt();

        instance->AudioEvent();

        instance->PicosToInterrupt = instance->MasterTickCounter;
        instance->PicosToSoundSample = instance->SoundInterrupt;
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
      if (instance->Throttle == 0) {
        instance->Throttle = SetSpeedThrottle(QUERY);

        if (instance->Throttle == 0) {
          instance->Throttle = 2; // 2 = No throttle.
        }
      }

      SetSpeedThrottle(0);

      if (instance->ClipCycle == 1) {
        key = PeekClipboard();

        if (key == SHIFT) {
          vccKeyboardHandleKey(SHIFT, SHIFT, kEventKeyDown);  //Press shift and...
          PopClipboard();
          key = PeekClipboard();
        }

        vccKeyboardHandleKey(key, key, kEventKeyDown);

        instance->WaitCycle = key == 0x1c ? 6000 : 2000;
      }
      else if (instance->ClipCycle == 500) {
        key = PeekClipboard();

        vccKeyboardHandleKey(SHIFT, SHIFT, kEventKeyUp);
        vccKeyboardHandleKey(0x42, key, kEventKeyUp);
        PopClipboard();

        if (ClipboardEmpty()) { //Finished?
          SetPaste(false);

          //Done pasting. Reset throttle to original state
          if (instance->Throttle == 2) {
            SetSpeedThrottle(0);
          }
          else {
            SetSpeedThrottle(1);
          }

          //...and reset the keymap to the original state
          vccKeyboardBuildRuntimeTable((keyboardlayout_e)GetCurrentKeyMap());

          instance->Throttle = 0;
        }
      }

      instance->ClipCycle++;

      if (instance->ClipCycle > instance->WaitCycle) {
        instance->ClipCycle = 1;
      }
    }

    return(0);
  }
}

extern "C" {
  __declspec(dllexport) float __cdecl RenderFrame(SystemState* systemState)
  {
    static unsigned short FrameCounter = 0;

    //********************************Start of frame Render*****************************************************
    GetGraphicsState()->BlinkState = instance->BlinkPhase;

    MC6821_irq_fs(0);				//FS low to High transition start of display Blink needs this

    for (systemState->LineCounter = 0; systemState->LineCounter < 13; systemState->LineCounter++) {		//Vertical Blanking 13 H lines
      CPUCycle();
    }

    for (systemState->LineCounter = 0; systemState->LineCounter < 4; systemState->LineCounter++) {		//4 non-Rendered top Border lines
      CPUCycle();
    }

    if (!(FrameCounter % systemState->FrameSkip)) {
      if (LockScreen(systemState)) {
        return(0);
      }
    }

    for (systemState->LineCounter = 0; systemState->LineCounter < (instance->TopBorder - 4); systemState->LineCounter++)
    {
      if (!(FrameCounter % systemState->FrameSkip)) {
        instance->DrawTopBorder[systemState->BitDepth](systemState);
      }

      CPUCycle();
    }

    for (systemState->LineCounter = 0; systemState->LineCounter < instance->LinesperScreen; systemState->LineCounter++)		//Active Display area		
    {
      CPUCycle();

      if (!(FrameCounter % systemState->FrameSkip)) {
        instance->UpdateScreen[systemState->BitDepth](systemState);
      }
    }

    MC6821_irq_fs(1);  //End of active display FS goes High to Low

    if (instance->VertInterruptEnabled) {
      GimeAssertVertInterrupt();
    }

    for (systemState->LineCounter = 0; systemState->LineCounter < (instance->BottomBorder); systemState->LineCounter++)	// Bottom border
    {
      CPUCycle();

      if (!(FrameCounter % systemState->FrameSkip)) {
        instance->DrawBottomBorder[systemState->BitDepth](systemState);
      }
    }

    if (!(FrameCounter % systemState->FrameSkip))
    {
      UnlockScreen(systemState);
      SetBorderChange(0);
    }

    for (systemState->LineCounter = 0; systemState->LineCounter < 6; systemState->LineCounter++) {		//Vertical Retrace 6 H lines
      CPUCycle();
    }

    switch (instance->SoundOutputMode)
    {
    case 0:
      FlushAudioBuffer(instance->AudioBuffer, instance->AudioIndex << 2);
      break;

    case 1:
      FlushCassetteBuffer(instance->CassBuffer, instance->AudioIndex);
      break;

    case 2:
      LoadCassetteBuffer(instance->CassBuffer);
      break;
    }

    instance->AudioIndex = 0;

    return(CalculateFPS());
  }
}