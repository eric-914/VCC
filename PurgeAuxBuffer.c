#include "library/AudioState.h"

#include "GetFreeBlockCount.h"

void PurgeAuxBuffer(void)
{
  AudioState* audioState = GetAudioState();

  if ((!audioState->InitPassed) || (audioState->AudioPause)) {
    return;
  }

  return; //TODO: Why?

  audioState->AuxBufferPointer--;			//Normally points to next free block Point to last used block

  if (audioState->AuxBufferPointer >= 0)	//zero is a valid data block
  {
    while ((GetFreeBlockCount() <= 0)) {};

    audioState->hr = audioState->lpdsbuffer1->Lock(audioState->BuffOffset, audioState->BlockSize, &(audioState->SndPointer1), &(audioState->SndLength1), &(audioState->SndPointer2), &(audioState->SndLength2), 0);

    if (audioState->hr != DS_OK) {
      return;
    }

    memcpy(audioState->SndPointer1, audioState->AuxBuffer[audioState->AuxBufferPointer], audioState->SndLength1);

    if (audioState->SndPointer2 != NULL) {
      memcpy(audioState->SndPointer2, (audioState->AuxBuffer[audioState->AuxBufferPointer] + (audioState->SndLength1 >> 2)), audioState->SndLength2);
    }

    audioState->BuffOffset = (audioState->BuffOffset + audioState->BlockSize) % audioState->SndBuffLength;

    audioState->hr = audioState->lpdsbuffer1->Unlock(audioState->SndPointer1, audioState->SndLength1, audioState->SndPointer2, audioState->SndLength2);

    audioState->AuxBufferPointer--;
  }

  audioState->AuxBufferPointer = 0;
}
