#include "library/cassettestate.h"

//--CASSETTE--//

unsigned int GetTapeCounter(void)
{
  CassetteState* cassetteState = GetCassetteState();

  return(cassetteState->TapeOffset);
}
