#include "library/AudioState.h"

#include "DSEnumCallback.h"

int GetSoundCardList(SndCardList* list)
{
  AudioState* audioState = GetAudioState();

  audioState->CardCount = 0;
  audioState->Cards = list;

  DirectSoundEnumerate(DSEnumCallback, NULL);

  return(audioState->CardCount);
}
