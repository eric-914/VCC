#include "library/Audio.h"

#include "DirectSoundEnumerateCallback.h"

int GetSoundCardList(SoundCardList* list)
{
  AudioState* audioState = GetAudioState();

  audioState->CardCount = 0;
  audioState->Cards = list;

  DirectSoundEnumerate(DirectSoundEnumerateCallback, NULL);

  return(audioState->CardCount);
}
