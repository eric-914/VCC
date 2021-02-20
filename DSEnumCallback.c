#include <windows.h>

#include "library/Audio.h"

BOOL CALLBACK DSEnumCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
{
  AudioState* audioState = GetAudioState();

  strncpy(audioState->Cards[audioState->CardCount].CardName, lpcstrDescription, 63);
  audioState->Cards[audioState->CardCount++].Guid = lpGuid;

  return (audioState->CardCount < MAXCARDS);
}
