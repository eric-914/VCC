#include "library/Audio.h"

#include "DSEnumCallback.h"

int GetSoundCardList(SoundCardList* list)
{
  AudioState* audioState = GetAudioState();

  audioState->CardCount = 0;
  audioState->Cards = list;

  DirectSoundEnumerate(DSEnumCallback, NULL);

  return(audioState->CardCount);
}
