#include "AudioState.h"

#include "library/defines.h"

int GetFreeBlockCount(void) //return 0 on full buffer
{
  unsigned long writeCursor = 0, playCursor = 0;
  long retVal = 0, maxSize = 0;

  AudioState* audioState = GetAudioState();

  if ((!audioState->InitPassed) || (audioState->AudioPause)) {
    return(AUDIOBUFFERS);
  }

  retVal = audioState->lpdsbuffer1->GetCurrentPosition(&playCursor, &writeCursor);

  if (audioState->BuffOffset <= playCursor) {
    maxSize = playCursor - audioState->BuffOffset;
  }
  else {
    maxSize = audioState->SndBuffLength - audioState->BuffOffset + playCursor;
  }

  return(maxSize / audioState->BlockSize);
}
