#include "library/cassettestate.h"

//--CASSETTE--//

extern void SyncFileBuffer(void);

void CloseTapeFile(void)
{
  CassetteState* cassetteState = GetCassetteState();

  if (cassetteState->TapeHandle == NULL) {
    return;
  }

  SyncFileBuffer();
  CloseHandle(cassetteState->TapeHandle);

  cassetteState->TapeHandle = NULL;
  cassetteState->TotalSize = 0;
}
