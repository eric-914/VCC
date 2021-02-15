#include "AudioState.h"

unsigned short GetSoundStatus(void)
{
  return(GetAudioState()->CurrentRate);
}

unsigned char PauseAudio(unsigned char pause)
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

const char* GetRateList(unsigned char index) {
  return GetAudioState()->RateList[index];
}

int SoundDeInit(void)
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
