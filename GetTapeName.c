#include "library/cassettestate.h"

//--CASSETTE--//

void GetTapeName(char* name)
{
  CassetteState* cassetteState = GetCassetteState();

  strcpy(name, cassetteState->TapeFileName);
}
