/*
Copyright 2015 by Joseph Forgione
This file is part of VCC (Virtual Color Computer).

    VCC (Virtual Color Computer) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    VCC (Virtual Color Computer) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with VCC (Virtual Color Computer).  If not, see <http://www.gnu.org/licenses/>.
*/

#include <dsound.h>
#include <stdio.h>

#include "config.h"
#include "coco3.h"
#include "audio.h"

#include "library\defines.h"

#define MAXCARDS	12

//PlayBack
static LPDIRECTSOUND	lpds;           // directsound interface pointer
static DSBUFFERDESC		dsbd;           // directsound description
static DSCAPS			    dscaps;         // directsound caps
static DSBCAPS			  dsbcaps;        // directsound buffer caps

//Record
static LPDIRECTSOUNDCAPTURE8	lpdsin;
static DSCBUFFERDESC			    dsbdin;           // directsound description

HRESULT hr;
static LPDIRECTSOUNDBUFFER	lpdsbuffer1 = NULL;			  //the sound buffers
static LPDIRECTSOUNDCAPTUREBUFFER	lpdsbuffer2 = NULL;	//the sound buffers for capture
static WAVEFORMATEX pcmwf;								            //generic waveformat structure
static void* SndPointer1 = NULL, * SndPointer2 = NULL;
static unsigned short BitRate = 0;
static DWORD SndLenth1 = 0, SndLenth2 = 0, SndBuffLenth = 0;
static unsigned char InitPassed = 0;
static unsigned short BlockSize = 0;
static unsigned short AuxBuffer[6][44100 / 60]; //Biggest block size possible
static DWORD WritePointer = 0;
static DWORD BuffOffset = 0;
static char AuxBufferPointer = 0;
static int CardCount = 0;
static unsigned short CurrentRate = 0;
static unsigned char AudioPause = 0;
static SndCardList* Cards = NULL;
BOOL CALLBACK DSEnumCallback(LPGUID, LPCSTR, LPCSTR, LPVOID);

static char RateList[4][7] = { "Mute", "11025", "22050", "44100" };
static unsigned short iRateList[4] = { 0, 11025, 22050, 44100 };

int SoundInit(HWND main_window_handle, _GUID* guid, unsigned short rate)
{
  rate = (rate & 3);

  if (rate != 0)	//Force 44100 or Mute
    rate = 3;

  CurrentRate = rate;

  if (InitPassed)
  {
    InitPassed = 0;
    lpdsbuffer1->Stop();

    if (lpdsbuffer1 != NULL)
    {
      hr = lpdsbuffer1->Release();
      lpdsbuffer1 = NULL;
    }

    if (lpds != NULL)
    {
      hr = lpds->Release();
      lpds = NULL;
    }
  }

  SndLenth1 = 0;
  SndLenth2 = 0;
  BuffOffset = 0;
  AuxBufferPointer = 0;
  BitRate = iRateList[rate];
  BlockSize = BitRate * 4 / TARGETFRAMERATE;
  SndBuffLenth = (BlockSize * AUDIOBUFFERS);

  if (rate)
  {
    hr = DirectSoundCreate(guid, &lpds, NULL);	// create a directsound object

    if (hr != DS_OK)
      return(1);

    hr = lpds->SetCooperativeLevel(main_window_handle, DSSCL_NORMAL); // set cooperation level normal DSSCL_EXCLUSIVE

    if (hr != DS_OK)
      return(1);

    // set up the format data structure
    memset(&pcmwf, 0, sizeof(WAVEFORMATEX));
    pcmwf.wFormatTag = WAVE_FORMAT_PCM;
    pcmwf.nChannels = 2;
    pcmwf.nSamplesPerSec = BitRate;
    pcmwf.wBitsPerSample = 16;
    pcmwf.nBlockAlign = (pcmwf.wBitsPerSample * pcmwf.nChannels) >> 3;
    pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;
    pcmwf.cbSize = 0;

    // create the secondary buffer 
    memset(&dsbd, 0, sizeof(DSBUFFERDESC));
    dsbd.dwSize = sizeof(DSBUFFERDESC);
    dsbd.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_LOCSOFTWARE | DSBCAPS_STATIC | DSBCAPS_GLOBALFOCUS;
    dsbd.dwBufferBytes = SndBuffLenth;
    dsbd.lpwfxFormat = &pcmwf;

    hr = lpds->CreateSoundBuffer(&dsbd, &lpdsbuffer1, NULL);

    if (hr != DS_OK)
      return(1);

    // Clear out sound buffers
    hr = lpdsbuffer1->Lock(0, SndBuffLenth, &SndPointer1, &SndLenth1, &SndPointer2, &SndLenth2, DSBLOCK_ENTIREBUFFER);

    if (hr != DS_OK)
      return(1);

    memset(SndPointer1, 0, SndBuffLenth);
    hr = lpdsbuffer1->Unlock(SndPointer1, SndLenth1, SndPointer2, SndLenth2);

    if (hr != DS_OK)
      return(1);

    lpdsbuffer1->SetCurrentPosition(0);
    hr = lpdsbuffer1->Play(0, 0, DSBPLAY_LOOPING);	// play the sound in looping mode

    if (hr != DS_OK)
      return(1);

    InitPassed = 1;
    AudioPause = 0;
  }

  SetAudioRate(iRateList[rate]);

  return(0);
}

void FlushAudioBuffer(unsigned int* aBuffer, unsigned short length)
{
  unsigned short LeftAverage = 0, RightAverage = 0, Index = 0;
  unsigned char Flag = 0;
  unsigned char* Abuffer2 = (unsigned char*)aBuffer;
  LeftAverage = aBuffer[0] >> 16;
  RightAverage = aBuffer[0] & 0xFFFF;
  UpdateSoundBar(LeftAverage, RightAverage);

  if ((!InitPassed) | (AudioPause))
    return;

  if (GetFreeBlockCount() <= 0)	//this should only kick in when frame skipping or unthrottled
  {
    memcpy(AuxBuffer[AuxBufferPointer], Abuffer2, length);	//Saving buffer to aux stack
    AuxBufferPointer++;		//and chase your own tail
    AuxBufferPointer %= 5;	//At this point we are so far behind we may as well drop the buffer
    return;
  }

  hr = lpdsbuffer1->Lock(BuffOffset, length, &SndPointer1, &SndLenth1, &SndPointer2, &SndLenth2, 0);

  if (hr != DS_OK)
    return;

  memcpy(SndPointer1, Abuffer2, SndLenth1);	// copy first section of circular buffer

  if (SndPointer2 != NULL)						// copy last section of circular buffer if wrapped
    memcpy(SndPointer2, Abuffer2 + SndLenth1, SndLenth2);

  hr = lpdsbuffer1->Unlock(SndPointer1, SndLenth1, SndPointer2, SndLenth2);// unlock the buffer
  BuffOffset = (BuffOffset + length) % SndBuffLenth;	//Where to write next
}

int GetFreeBlockCount(void) //return 0 on full buffer
{
  unsigned long WriteCursor = 0, PlayCursor = 0;
  long RetVal = 0, MaxSize = 0;

  if ((!InitPassed) | (AudioPause))
    return(AUDIOBUFFERS);

  RetVal = lpdsbuffer1->GetCurrentPosition(&PlayCursor, &WriteCursor);

  if (BuffOffset <= PlayCursor)
    MaxSize = PlayCursor - BuffOffset;
  else
    MaxSize = SndBuffLenth - BuffOffset + PlayCursor;

  return(MaxSize / BlockSize);
}

void PurgeAuxBuffer(void)
{
  if ((!InitPassed) | (AudioPause))
    return;

  return; //TODO: Wait, what?

  AuxBufferPointer--;			//Normally points to next free block Point to last used block

  if (AuxBufferPointer >= 0)	//zero is a valid data block
  {
    while ((GetFreeBlockCount() <= 0));

    hr = lpdsbuffer1->Lock(BuffOffset, BlockSize, &SndPointer1, &SndLenth1, &SndPointer2, &SndLenth2, 0);

    if (hr != DS_OK)
      return;

    memcpy(SndPointer1, AuxBuffer[AuxBufferPointer], SndLenth1);

    if (SndPointer2 != NULL)
      memcpy(SndPointer2, (AuxBuffer[AuxBufferPointer] + (SndLenth1 >> 2)), SndLenth2);

    BuffOffset = (BuffOffset + BlockSize) % SndBuffLenth;
    hr = lpdsbuffer1->Unlock(SndPointer1, SndLenth1, SndPointer2, SndLenth2);
    AuxBufferPointer--;
  }

  AuxBufferPointer = 0;
}

int GetSoundCardList(SndCardList* list)
{
  CardCount = 0;
  Cards = list;
  DirectSoundEnumerate(DSEnumCallback, NULL);

  return(CardCount);
}

BOOL CALLBACK DSEnumCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
{
  strncpy(Cards[CardCount].CardName, lpcstrDescription, 63);
  Cards[CardCount++].Guid = lpGuid;

  return (CardCount < MAXCARDS);
}

int SoundDeInit(void)
{
  if (InitPassed)
  {
    InitPassed = 0;
    lpdsbuffer1->Stop();
    lpds->Release();
  }

  return(0);
}

int SoundInInit(HWND main_window_handle, _GUID* guid)
{
  hr = DirectSoundCaptureCreate(guid, &lpdsin, NULL);

  if (hr != DS_OK)
    return(1);

  dsbdin.dwSize = sizeof(DSCBUFFERDESC); // Size of the structure
  dsbdin.dwFlags = 0;
  dsbdin.dwReserved = 0;
  dsbdin.lpwfxFormat = &pcmwf; //fix
  dsbdin.dwBufferBytes = SndBuffLenth;
  hr = lpdsin->CreateCaptureBuffer(&dsbdin, &lpdsbuffer2, NULL);

  if (hr != DS_OK)
    return(1);

  lpdsbuffer2->Start(hr);

  return(0);
}

unsigned short GetSoundStatus(void)
{
  return(CurrentRate);
}

void ResetAudio(void)
{
  SetAudioRate(iRateList[CurrentRate]);
  //	SetAudioRate(44100);
  if (InitPassed)
    lpdsbuffer1->SetCurrentPosition(0);

  BuffOffset = 0;
  AuxBufferPointer = 0;
}

unsigned char PauseAudio(unsigned char pause)
{
  AudioPause = pause;

  if (InitPassed)
  {
    if (AudioPause == 1)
      hr = lpdsbuffer1->Stop();
    else
      hr = lpdsbuffer1->Play(0, 0, DSBPLAY_LOOPING);
  }

  return(AudioPause);
}

const char* GetRateList(unsigned char index) {
  return RateList[index];
}