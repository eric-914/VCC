#pragma once

#include <windows.h>
#include <dsound.h>

#include "library/audiodef.h"

typedef struct {
  HRESULT hr;

  DWORD SndLength1;
  DWORD SndLength2;
  DWORD SndBuffLength;
  DWORD WritePointer;
  DWORD BuffOffset;

  void* SndPointer1;
  void* SndPointer2;

  char AuxBufferPointer;
  int CardCount;
  unsigned short CurrentRate;
  unsigned char AudioPause;
  unsigned short BitRate;
  unsigned char InitPassed;
  unsigned short BlockSize;

  //PlayBack
  LPDIRECTSOUND	lpds;           // directsound interface pointer
  DSBUFFERDESC	dsbd;           // directsound description
  DSCAPS			  dscaps;         // directsound caps
  DSBCAPS			  dsbcaps;        // directsound buffer caps

  //Record
  LPDIRECTSOUNDCAPTURE8	lpdsin;
  DSCBUFFERDESC			    dsbdin; // directsound description

  LPDIRECTSOUNDBUFFER	lpdsbuffer1;			    //the sound buffers
  LPDIRECTSOUNDCAPTUREBUFFER	lpdsbuffer2;	//the sound buffers for capture

  WAVEFORMATEX pcmwf; //generic waveformat structure

  SndCardList* Cards;

  unsigned short AuxBuffer[6][44100 / 60]; //Biggest block size possible

  char RateList[4][7];
  unsigned short iRateList[4];
} AudioState;

extern "C" __declspec(dllexport) AudioState * __cdecl GetAudioState();
