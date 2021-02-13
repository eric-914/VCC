#include "library/cassettedef.h"
#include "library/cassettestate.h"
#include "library/defines.h"

//--CASSETTE--//

#include "CastoWav.h"

extern void UpdateTapeCounter(unsigned int counter, unsigned char tapeMode);

void LoadCassetteBuffer(unsigned char* cassBuffer)
{
  CassetteState* cassetteState = GetCassetteState();

  unsigned long bytesMoved = 0;

  if (cassetteState->TapeMode != PLAY) {
    return;
  }

  switch (cassetteState->FileType)
  {
  case WAV:
    SetFilePointer(cassetteState->TapeHandle, cassetteState->TapeOffset + 44, 0, FILE_BEGIN);
    ReadFile(cassetteState->TapeHandle, cassBuffer, TAPEAUDIORATE / 60, &bytesMoved, NULL);

    cassetteState->TapeOffset += bytesMoved;

    if (cassetteState->TapeOffset > cassetteState->TotalSize) {
      cassetteState->TapeOffset = cassetteState->TotalSize;
    }

    break;

  case CAS:
    CastoWav(cassBuffer, TAPEAUDIORATE / 60, &bytesMoved);

    break;
  }

  UpdateTapeCounter(cassetteState->TapeOffset, cassetteState->TapeMode);
}
