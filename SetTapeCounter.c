#include "library/Cassette.h"
#include "library/Config.h"

void SetTapeCounter(unsigned int count)
{
  CassetteState* cassetteState = GetCassetteState();

  cassetteState->TapeOffset = count;

  if (cassetteState->TapeOffset > cassetteState->TotalSize) {
    cassetteState->TotalSize = cassetteState->TapeOffset;
  }

  UpdateTapeCounter(cassetteState->TapeOffset, cassetteState->TapeMode);
}
