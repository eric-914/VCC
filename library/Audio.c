#include "Audio.h"
#include "Config.h"
#include "Coco.h"
#include "Callbacks.h"

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

    if ((!instance->InitPassed) || (instance->AudioPause)) {
      return(AUDIOBUFFERS);
    }

    retVal = instance->lpdsbuffer1->GetCurrentPosition(&playCursor, &writeCursor);

    if (instance->BuffOffset <= playCursor) {
      maxSize = playCursor - instance->BuffOffset;
    }
    else {
      maxSize = instance->SndBuffLength - instance->BuffOffset + playCursor;
    }

    return(maxSize / instance->BlockSize);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl FlushAudioBuffer(unsigned int* aBuffer, unsigned short length)
  {
    unsigned short leftAverage = 0, rightAverage = 0, index = 0;
    unsigned char flag = 0;
    unsigned char* Abuffer2 = (unsigned char*)aBuffer;

    leftAverage = aBuffer[0] >> 16;
    rightAverage = aBuffer[0] & 0xFFFF;

    UpdateSoundBar(leftAverage, rightAverage);

    if ((!instance->InitPassed) || (instance->AudioPause)) {
      return;
    }

    if (GetFreeBlockCount() <= 0)	//this should only kick in when frame skipping or unthrottled
    {
      memcpy(instance->AuxBuffer[instance->AuxBufferPointer], Abuffer2, length);	//Saving buffer to aux stack

      instance->AuxBufferPointer++;		//and chase your own tail
      instance->AuxBufferPointer %= 5;	//At this point we are so far behind we may as well drop the buffer

      return;
    }

    instance->hr = instance->lpdsbuffer1->Lock(instance->BuffOffset, length, &(instance->SndPointer1), &(instance->SndLength1), &(instance->SndPointer2), &(instance->SndLength2), 0);

    if (instance->hr != DS_OK) {
      return;
    }

    memcpy(instance->SndPointer1, Abuffer2, instance->SndLength1);	// copy first section of circular buffer

    if (instance->SndPointer2 != NULL) { // copy last section of circular buffer if wrapped
      memcpy(instance->SndPointer2, Abuffer2 + instance->SndLength1, instance->SndLength2);
    }

    instance->hr = instance->lpdsbuffer1->Unlock(instance->SndPointer1, instance->SndLength1, instance->SndPointer2, instance->SndLength2);// unlock the buffer

    instance->BuffOffset = (instance->BuffOffset + length) % instance->SndBuffLength;	//Where to write next
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl PurgeAuxBuffer(void)
  {
    if ((!instance->InitPassed) || (instance->AudioPause)) {
      return;
    }

    return; //TODO: Why?

    instance->AuxBufferPointer--;			//Normally points to next free block Point to last used block

    if (instance->AuxBufferPointer >= 0)	//zero is a valid data block
    {
      while ((GetFreeBlockCount() <= 0)) {};

      instance->hr = instance->lpdsbuffer1->Lock(instance->BuffOffset, instance->BlockSize, &(instance->SndPointer1), &(instance->SndLength1), &(instance->SndPointer2), &(instance->SndLength2), 0);

      if (instance->hr != DS_OK) {
        return;
      }

      memcpy(instance->SndPointer1, instance->AuxBuffer[instance->AuxBufferPointer], instance->SndLength1);

      if (instance->SndPointer2 != NULL) {
        memcpy(instance->SndPointer2, (instance->AuxBuffer[instance->AuxBufferPointer] + (instance->SndLength1 >> 2)), instance->SndLength2);
      }

      instance->BuffOffset = (instance->BuffOffset + instance->BlockSize) % instance->SndBuffLength;

      instance->hr = instance->lpdsbuffer1->Unlock(instance->SndPointer1, instance->SndLength1, instance->SndPointer2, instance->SndLength2);

      instance->AuxBufferPointer--;
    }

    instance->AuxBufferPointer = 0;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl ResetAudio(void)
  {
    SetAudioRate(instance->iRateList[instance->CurrentRate]);

    //	SetAudioRate(44100);
    if (instance->InitPassed) {
      instance->lpdsbuffer1->SetCurrentPosition(0);
    }

    instance->BuffOffset = 0;
    instance->AuxBufferPointer = 0;
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl GetSoundCardList(SoundCardList* list)
  {
    instance->CardCount = 0;
    instance->Cards = list;

    DirectSoundEnumerate(DirectSoundEnumerateCallback, NULL);

    return(instance->CardCount);
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl SoundInit(HWND hWnd, _GUID* guid, unsigned short rate)
{
  rate = (rate & 3);

  if (rate != 0) {	//Force 44100 or Mute
    rate = 3;
  }

  instance->CurrentRate = rate;

  if (instance->InitPassed)
  {
    instance->InitPassed = 0;
    instance->lpdsbuffer1->Stop();

    if (instance->lpdsbuffer1 != NULL)
    {
      instance->hr = instance->lpdsbuffer1->Release();
      instance->lpdsbuffer1 = NULL;
    }

    if (instance->lpds != NULL)
    {
      instance->hr = instance->lpds->Release();
      instance->lpds = NULL;
    }
  }

  instance->SndLength1 = 0;
  instance->SndLength2 = 0;
  instance->BuffOffset = 0;
  instance->AuxBufferPointer = 0;
  instance->BitRate = instance->iRateList[rate];
  instance->BlockSize = instance->BitRate * 4 / TARGETFRAMERATE;
  instance->SndBuffLength = (instance->BlockSize * AUDIOBUFFERS);

  if (rate)
  {
    instance->hr = DirectSoundCreate(guid, &(instance->lpds), NULL);	// create a directsound object

    if (instance->hr != DS_OK) {
      return(1);
    }

    instance->hr = instance->lpds->SetCooperativeLevel(hWnd, DSSCL_NORMAL); // set cooperation level normal DSSCL_EXCLUSIVE

    if (instance->hr != DS_OK) {
      return(1);
    }

    // set up the format data structure
    memset(&(instance->pcmwf), 0, sizeof(WAVEFORMATEX));
    instance->pcmwf.wFormatTag = WAVE_FORMAT_PCM;
    instance->pcmwf.nChannels = 2;
    instance->pcmwf.nSamplesPerSec = instance->BitRate;
    instance->pcmwf.wBitsPerSample = 16;
    instance->pcmwf.nBlockAlign = (instance->pcmwf.wBitsPerSample * instance->pcmwf.nChannels) >> 3;
    instance->pcmwf.nAvgBytesPerSec = instance->pcmwf.nSamplesPerSec * instance->pcmwf.nBlockAlign;
    instance->pcmwf.cbSize = 0;

    // create the secondary buffer 
    memset(&(instance->dsbd), 0, sizeof(DSBUFFERDESC));
    instance->dsbd.dwSize = sizeof(DSBUFFERDESC);
    instance->dsbd.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_LOCSOFTWARE | DSBCAPS_STATIC | DSBCAPS_GLOBALFOCUS;
    instance->dsbd.dwBufferBytes = instance->SndBuffLength;
    instance->dsbd.lpwfxFormat = &(instance->pcmwf);

    instance->hr = instance->lpds->CreateSoundBuffer(&(instance->dsbd), &(instance->lpdsbuffer1), NULL);

    if (instance->hr != DS_OK) {
      return(1);
    }

    // Clear out sound buffers
    instance->hr = instance->lpdsbuffer1->Lock(0, instance->SndBuffLength, &(instance->SndPointer1), &(instance->SndLength1), &(instance->SndPointer2), &(instance->SndLength2), DSBLOCK_ENTIREBUFFER);

    if (instance->hr != DS_OK) {
      return(1);
    }

    memset(instance->SndPointer1, 0, instance->SndBuffLength);
    instance->hr = instance->lpdsbuffer1->Unlock(instance->SndPointer1, instance->SndLength1, instance->SndPointer2, instance->SndLength2);

    if (instance->hr != DS_OK) {
      return(1);
    }

    instance->lpdsbuffer1->SetCurrentPosition(0);
    instance->hr = instance->lpdsbuffer1->Play(0, 0, DSBPLAY_LOOPING);	// play the sound in looping mode

    if (instance->hr != DS_OK) {
      return(1);
    }

    instance->InitPassed = 1;
    instance->AudioPause = 0;
  }

  SetAudioRate(instance->iRateList[rate]);

  return(0);
}
}
