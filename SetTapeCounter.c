#include "library/cassettestate.h"

//--CASSETTE--//

extern void UpdateTapeCounter(unsigned int counter, unsigned char tapeMode);

void SetTapeCounter(unsigned int count)
{
  CassetteState* cassetteState = GetCassetteState();

  cassetteState->TapeOffset = count;

  if (cassetteState->TapeOffset > cassetteState->TotalSize) {
    cassetteState->TotalSize = cassetteState->TapeOffset;
  }

  UpdateTapeCounter(cassetteState->TapeOffset, cassetteState->TapeMode);
}
