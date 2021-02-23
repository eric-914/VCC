#include "library/Audio.h"
#include "library/Callbacks.h"

int GetSoundCardList(SoundCardList* list)
{
  AudioState* audioState = GetAudioState();

  audioState->CardCount = 0;
  audioState->Cards = list;

  DirectSoundEnumerate(DirectSoundEnumerateCallback, NULL);

  return(audioState->CardCount);
}
