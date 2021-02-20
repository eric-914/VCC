#include "library/AudioState.h"

#include "library/Config.h"
#include "GetFreeBlockCount.h"

void FlushAudioBuffer(unsigned int* aBuffer, unsigned short length)
{
  unsigned short leftAverage = 0, rightAverage = 0, index = 0;
  unsigned char flag = 0;
  unsigned char* Abuffer2 = (unsigned char*)aBuffer;

  AudioState* audioState = GetAudioState();

  leftAverage = aBuffer[0] >> 16;
  rightAverage = aBuffer[0] & 0xFFFF;

  UpdateSoundBar(leftAverage, rightAverage);

  if ((!audioState->InitPassed) || (audioState->AudioPause)) {
    return;
  }

  if (GetFreeBlockCount() <= 0)	//this should only kick in when frame skipping or unthrottled
  {
    memcpy(audioState->AuxBuffer[audioState->AuxBufferPointer], Abuffer2, length);	//Saving buffer to aux stack

    audioState->AuxBufferPointer++;		//and chase your own tail
    audioState->AuxBufferPointer %= 5;	//At this point we are so far behind we may as well drop the buffer

    return;
  }

  audioState->hr = audioState->lpdsbuffer1->Lock(audioState->BuffOffset, length, &(audioState->SndPointer1), &(audioState->SndLength1), &(audioState->SndPointer2), &(audioState->SndLength2), 0);

  if (audioState->hr != DS_OK) {
    return;
  }

  memcpy(audioState->SndPointer1, Abuffer2, audioState->SndLength1);	// copy first section of circular buffer

  if (audioState->SndPointer2 != NULL) { // copy last section of circular buffer if wrapped
    memcpy(audioState->SndPointer2, Abuffer2 + audioState->SndLength1, audioState->SndLength2);
  }

  audioState->hr = audioState->lpdsbuffer1->Unlock(audioState->SndPointer1, audioState->SndLength1, audioState->SndPointer2, audioState->SndLength2);// unlock the buffer

  audioState->BuffOffset = (audioState->BuffOffset + length) % audioState->SndBuffLength;	//Where to write next
}
