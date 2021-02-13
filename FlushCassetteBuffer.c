#include "library/cassettedef.h"
#include "library/cassettestate.h"

//--CASSETTE--//

#include "WavtoCas.h"

extern void UpdateTapeCounter(unsigned int counter, unsigned char tapeMode);

void FlushCassetteBuffer(unsigned char* buffer, unsigned int length)
{
  CassetteState* cassetteState = GetCassetteState();

  if (cassetteState->TapeMode != REC) {
    return;
  }

  switch (cassetteState->FileType)
  {
  case WAV:
    SetFilePointer(cassetteState->TapeHandle, cassetteState->TapeOffset + 44, 0, FILE_BEGIN);
    WriteFile(cassetteState->TapeHandle, buffer, length, &(cassetteState->BytesMoved), NULL);

    if (length != cassetteState->BytesMoved) {
      return;
    }

    cassetteState->TapeOffset += length;

    if (cassetteState->TapeOffset > cassetteState->TotalSize) {
      cassetteState->TotalSize = cassetteState->TapeOffset;
    }

    break;

  case CAS:
    WavtoCas(buffer, length);

    break;
  }

  UpdateTapeCounter(cassetteState->TapeOffset, cassetteState->TapeMode);
}
