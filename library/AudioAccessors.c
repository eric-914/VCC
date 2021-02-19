#include "AudioState.h"

extern "C" {
  __declspec(dllexport) unsigned short __cdecl GetSoundStatus(void)
  {
    return(GetAudioState()->CurrentRate);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl PauseAudio(unsigned char pause)
  {
    AudioState* audioState = GetAudioState();

    audioState->AudioPause = pause;

    if (audioState->InitPassed)
    {
      if (audioState->AudioPause == 1) {
        audioState->hr = audioState->lpdsbuffer1->Stop();
      }
      else {
        audioState->hr = audioState->lpdsbuffer1->Play(0, 0, DSBPLAY_LOOPING);
      }
    }

    return(audioState->AudioPause);
  }
}

extern "C" {
  __declspec(dllexport) const char* __cdecl GetRateList(unsigned char index) {
    return GetAudioState()->RateList[index];
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl SoundDeInit(void)
  {
    AudioState* audioState = GetAudioState();

    if (audioState->InitPassed)
    {
      audioState->InitPassed = 0;

      audioState->lpdsbuffer1->Stop();
      audioState->lpds->Release();
    }

    return(0);
  }
}
