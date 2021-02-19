#include "library/cocostate.h"

#include "library/defines.h"

extern "C" {
  __declspec(dllexport) unsigned short __cdecl SetAudioRate(unsigned short rate)
  {
    CoCoState* coco = GetCoCoState();

    coco->SndEnable = 1;
    coco->SoundInterrupt = 0;
    coco->CycleDrift = 0;
    coco->AudioIndex = 0;

    if (rate != 0) {	//Force Mute or 44100Hz
      rate = 44100;
    }

    if (rate == 0) {
      coco->SndEnable = 0;
    }
    else
    {
      coco->SoundInterrupt = PICOSECOND / rate;
      coco->PicosToSoundSample = coco->SoundInterrupt;
    }

    coco->SoundRate = rate;

    return(0);
  }
}

