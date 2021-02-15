#include <Windows.h>

#include "AudioState.h"
#include "SetAudioRate.h"

#include "library/defines.h"

int SoundInit(HWND main_window_handle, _GUID* guid, unsigned short rate)
{
  AudioState* audioState = GetAudioState();

  rate = (rate & 3);

  if (rate != 0) {	//Force 44100 or Mute
    rate = 3;
  }

  audioState->CurrentRate = rate;

  if (audioState->InitPassed)
  {
    audioState->InitPassed = 0;
    audioState->lpdsbuffer1->Stop();

    if (audioState->lpdsbuffer1 != NULL)
    {
      audioState->hr = audioState->lpdsbuffer1->Release();
      audioState->lpdsbuffer1 = NULL;
    }

    if (audioState->lpds != NULL)
    {
      audioState->hr = audioState->lpds->Release();
      audioState->lpds = NULL;
    }
  }

  audioState->SndLength1 = 0;
  audioState->SndLength2 = 0;
  audioState->BuffOffset = 0;
  audioState->AuxBufferPointer = 0;
  audioState->BitRate = audioState->iRateList[rate];
  audioState->BlockSize = audioState->BitRate * 4 / TARGETFRAMERATE;
  audioState->SndBuffLength = (audioState->BlockSize * AUDIOBUFFERS);

  if (rate)
  {
    audioState->hr = DirectSoundCreate(guid, &(audioState->lpds), NULL);	// create a directsound object

    if (audioState->hr != DS_OK) {
      return(1);
    }

    audioState->hr = audioState->lpds->SetCooperativeLevel(main_window_handle, DSSCL_NORMAL); // set cooperation level normal DSSCL_EXCLUSIVE

    if (audioState->hr != DS_OK) {
      return(1);
    }

    // set up the format data structure
    memset(&(audioState->pcmwf), 0, sizeof(WAVEFORMATEX));
    audioState->pcmwf.wFormatTag = WAVE_FORMAT_PCM;
    audioState->pcmwf.nChannels = 2;
    audioState->pcmwf.nSamplesPerSec = audioState->BitRate;
    audioState->pcmwf.wBitsPerSample = 16;
    audioState->pcmwf.nBlockAlign = (audioState->pcmwf.wBitsPerSample * audioState->pcmwf.nChannels) >> 3;
    audioState->pcmwf.nAvgBytesPerSec = audioState->pcmwf.nSamplesPerSec * audioState->pcmwf.nBlockAlign;
    audioState->pcmwf.cbSize = 0;

    // create the secondary buffer 
    memset(&(audioState->dsbd), 0, sizeof(DSBUFFERDESC));
    audioState->dsbd.dwSize = sizeof(DSBUFFERDESC);
    audioState->dsbd.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_LOCSOFTWARE | DSBCAPS_STATIC | DSBCAPS_GLOBALFOCUS;
    audioState->dsbd.dwBufferBytes = audioState->SndBuffLength;
    audioState->dsbd.lpwfxFormat = &(audioState->pcmwf);

    audioState->hr = audioState->lpds->CreateSoundBuffer(&(audioState->dsbd), &(audioState->lpdsbuffer1), NULL);

    if (audioState->hr != DS_OK) {
      return(1);
    }

    // Clear out sound buffers
    audioState->hr = audioState->lpdsbuffer1->Lock(0, audioState->SndBuffLength, &(audioState->SndPointer1), &(audioState->SndLength1), &(audioState->SndPointer2), &(audioState->SndLength2), DSBLOCK_ENTIREBUFFER);

    if (audioState->hr != DS_OK) {
      return(1);
    }

    memset(audioState->SndPointer1, 0, audioState->SndBuffLength);
    audioState->hr = audioState->lpdsbuffer1->Unlock(audioState->SndPointer1, audioState->SndLength1, audioState->SndPointer2, audioState->SndLength2);

    if (audioState->hr != DS_OK) {
      return(1);
    }

    audioState->lpdsbuffer1->SetCurrentPosition(0);
    audioState->hr = audioState->lpdsbuffer1->Play(0, 0, DSBPLAY_LOOPING);	// play the sound in looping mode

    if (audioState->hr != DS_OK) {
      return(1);
    }

    audioState->InitPassed = 1;
    audioState->AudioPause = 0;
  }

  SetAudioRate(audioState->iRateList[rate]);

  return(0);
}
