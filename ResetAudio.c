#include "AudioState.h"

#include "SetAudioRate.h"

void ResetAudio(void)
{
  AudioState* audioState = GetAudioState();

  SetAudioRate(audioState->iRateList[audioState->CurrentRate]);

  //	SetAudioRate(44100);
  if (audioState->InitPassed) {
    audioState->lpdsbuffer1->SetCurrentPosition(0);
  }

  audioState->BuffOffset = 0;
  audioState->AuxBufferPointer = 0;
}
