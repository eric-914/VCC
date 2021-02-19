#include "AudioState.h"

#define SIZEOF(x)  (sizeof(x) / sizeof((x)[0]))

const DWORD SndLength1 = 0;
const DWORD SndLength2 = 0;
const DWORD SndBuffLength = 0;
const DWORD WritePointer = 0;
const DWORD BuffOffset = 0;

const char AuxBufferPointer = 0;
const int CardCount = 0;

const unsigned short CurrentRate = 0;
const unsigned char AudioPause = 0;
const unsigned short BitRate = 0;
const unsigned char InitPassed = 0;
const unsigned short BlockSize = 0;

const LPDIRECTSOUNDBUFFER	lpdsbuffer1 = NULL;
const LPDIRECTSOUNDCAPTUREBUFFER	lpdsbuffer2 = NULL;

const char RateList[4][7] = { "Mute", "11025", "22050", "44100" };
const unsigned short iRateList[4] = { 0, 11025, 22050, 44100 };

AudioState* InitializeInstance(AudioState* audio);

static AudioState* instance = InitializeInstance(new AudioState());

extern "C" {
  __declspec(dllexport) AudioState* __cdecl GetAudioState() {
    return instance;
  }
}

AudioState* InitializeInstance(AudioState* a) {
  a->SndLength1 = SndLength1;
  a->SndLength2 = SndLength2;
  a->SndBuffLength = SndBuffLength;

  a->SndPointer1 = NULL;
  a->SndPointer2 = NULL;
  a->Cards = NULL;

  a->AuxBufferPointer = AuxBufferPointer;
  a->CardCount = CardCount;
  a->CurrentRate = CurrentRate;
  a->AudioPause = AudioPause;
  a->BitRate = BitRate;
  a->InitPassed = InitPassed;
  a->BlockSize = BlockSize;

  a->WritePointer = WritePointer;
  a->BuffOffset = BuffOffset;

  a->lpdsbuffer1 = lpdsbuffer1;
  a->lpdsbuffer2 = lpdsbuffer2;

  for (int i = 0; i < SIZEOF(RateList); i++) {
    strcpy(a->RateList[i], RateList[i]);
  }

  for (int i = 0; i < SIZEOF(iRateList); i++) {
    a->iRateList[i] = iRateList[i];
  }

  return a;
}
