#include "library/Audio.h"
#include "library/Coco.h"

#include "library/defines.h"

int SoundInit(HWND hWnd, _GUID* guid, unsigned short rate)
{
  AudioState* instance = GetAudioState();

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

void TestVCC()
{
  _GUID guid = _GUID();
  LPDIRECTSOUND	lpds;

  HRESULT result = DirectSoundCreate(&guid, &lpds, NULL);	// create a directsound object
}
