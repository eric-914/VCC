#include <windows.h>
#include <stdio.h>

#include "mmustate.h"

int LoadInternalRom(TCHAR filename[MAX_PATH])
{
  unsigned short index = 0;
  FILE* rom_handle = fopen(filename, "rb");

  MmuState* mmuState = GetMmuState();

  if (rom_handle == NULL) {
    return(0);
  }

  while ((feof(rom_handle) == 0) && (index < 0x8000)) {
    mmuState->InternalRomBuffer[index++] = fgetc(rom_handle);
  }

  fclose(rom_handle);

  return(index);
}
