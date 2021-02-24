#pragma once

#define MAXCARDS	12

#include "di.version.h"
#include <windows.h>
#include <dsound.h>

#include "SoundCardList.h"

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

  SoundCardList* Cards;

  unsigned short AuxBuffer[6][44100 / 60]; //Biggest block size possible

  char RateList[4][7];
  unsigned short iRateList[4];
} AudioState;

extern "C" __declspec(dllexport) AudioState * __cdecl GetAudioState();

extern "C" __declspec(dllexport) const char* __cdecl GetRateList(unsigned char);

extern "C" __declspec(dllexport) int __cdecl GetFreeBlockCount();
extern "C" __declspec(dllexport) int __cdecl GetSoundCardList(SoundCardList*);
extern "C" __declspec(dllexport) int __cdecl SoundDeInit();
extern "C" __declspec(dllexport) int __cdecl SoundInit(HWND, _GUID*, unsigned short);

extern "C" __declspec(dllexport) unsigned char __cdecl PauseAudio(unsigned char);
extern "C" __declspec(dllexport) unsigned short __cdecl GetSoundStatus();

extern "C" __declspec(dllexport) void __cdecl FlushAudioBuffer(unsigned int*, unsigned short);
extern "C" __declspec(dllexport) void __cdecl PurgeAuxBuffer();
extern "C" __declspec(dllexport) void __cdecl ResetAudio();
