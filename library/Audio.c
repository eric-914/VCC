#include "Audio.h"
#include "Config.h"
#include "Coco.h"

#include "defines.h"
#include "macros.h"

const char RateList[4][7] = { "Mute", "11025", "22050", "44100" };
const unsigned short iRateList[4] = { 0, 11025, 22050, 44100 };

AudioState* InitializeInstance(AudioState*);

static AudioState* instance = InitializeInstance(new AudioState());

extern "C" {
  __declspec(dllexport) AudioState* __cdecl GetAudioState() {
    return instance;
  }
}

AudioState* InitializeInstance(AudioState* p) {
  p->AudioPause = 0;
  p->AuxBufferPointer = 0;
  p->BitRate = 0;
  p->BlockSize = 0;
  p->BuffOffset = 0;
  p->CardCount = 0;
  p->CurrentRate = 0;
  p->InitPassed = 0;
  p->SndBuffLength = 0;
  p->SndLength1 = 0;
  p->SndLength2 = 0;
  p->WritePointer = 0;

  p->Cards = NULL;
  p->SndPointer1 = NULL;
  p->SndPointer2 = NULL;
  p->lpdsbuffer1 = NULL;
  p->lpdsbuffer2 = NULL;

  STRARRAYCOPY(RateList);
  ARRAYCOPY(iRateList);

  return p;
}

extern "C" {
  __declspec(dllexport) unsigned short __cdecl GetSoundStatus(void)
  {
    return(instance->CurrentRate);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl PauseAudio(unsigned char pause)
  {
    instance->AudioPause = pause;

    if (instance->InitPassed)
    {
      if (instance->AudioPause == 1) {
        instance->hr = instance->lpdsbuffer1->Stop();
      }
      else {
        instance->hr = instance->lpdsbuffer1->Play(0, 0, DSBPLAY_LOOPING);
      }
    }

    return(instance->AudioPause);
  }
}

extern "C" {
  __declspec(dllexport) const char* __cdecl GetRateList(unsigned char index) {
    return instance->RateList[index];
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl SoundDeInit(void)
  {
    if (instance->InitPassed)
    {
      instance->InitPassed = 0;

      instance->lpdsbuffer1->Stop();
      instance->lpds->Release();
    }

    return(0);
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl GetFreeBlockCount(void) //return 0 on full buffer
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
}

extern "C" {
  __declspec(dllexport) void __cdecl FlushAudioBuffer(unsigned int* aBuffer, unsigned short length)
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
}

extern "C" {
  __declspec(dllexport) void __cdecl PurgeAuxBuffer(void)
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
}

extern "C" {
  __declspec(dllexport) void __cdecl ResetAudio(void)
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
}

//void TestLibrary()
//{
//  _GUID guid = _GUID();
//  LPDIRECTSOUND	lpds;
//
//  HRESULT result = DirectSoundCreate(&guid, &lpds, NULL);	// create a directsound object
//}
