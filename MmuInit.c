#include <windows.h>

#include "library/mmustate.h"
#include "CopyRom.h"
#include "MmuReset.h"

#include "library/graphicsstate.h"

/*****************************************************************************************
* MmuInit Initilize and allocate memory for RAM Internal and External ROM Images.        *
* Copy Rom Images to buffer space and reset GIME MMU registers to 0                      *
* Returns NULL if any of the above fail.                                                 *
*****************************************************************************************/
unsigned char* MmuInit(unsigned char ramConfig)
{
  unsigned int index = 0;
  unsigned int ramSize;

  MmuState* mmuState = GetMmuState();

  ramSize = mmuState->MemConfig[ramConfig];

  mmuState->CurrentRamConfig = ramConfig;

  if (mmuState->Memory != NULL) {
    free(mmuState->Memory);
  }

  mmuState->Memory = (unsigned char*)malloc(ramSize);

  if (mmuState->Memory == NULL) {
    return(NULL);
  }

  for (index = 0;index < ramSize;index++)
  {
    mmuState->Memory[index] = index & 1 ? 0 : 0xFF;
  }

  GetGraphicsState()->VidMask = mmuState->VidMask[mmuState->CurrentRamConfig];

  if (mmuState->InternalRomBuffer != NULL) {
    free(mmuState->InternalRomBuffer);
  }

  mmuState->InternalRomBuffer = NULL;
  mmuState->InternalRomBuffer = (unsigned char*)malloc(0x8000);

  if (mmuState->InternalRomBuffer == NULL) {
    return(NULL);
  }

  memset(mmuState->InternalRomBuffer, 0xFF, 0x8000);
  CopyRom();
  MmuReset();

  return(mmuState->Memory);
}
