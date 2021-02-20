#include "CoCo.h"

#include "defines.h"

#include "graphicsstate.h"
#include "tcc1014graphics-8.h"
#include "tcc1014graphics-16.h"
#include "tcc1014graphics-24.h"
#include "tcc1014graphics-32.h"

#include "GetDACSample.h"

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
    instance->AudioBuffer[instance->AudioIndex++] = GetDACSample();
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
