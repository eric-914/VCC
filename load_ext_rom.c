#include <iostream>

#include "library/PAKInterface.h"
#include "library/systemstate.h"

#include "UnloadDll.h"

/**
Load a ROM pack
return total bytes loaded, or 0 on failure
*/
int load_ext_rom(SystemState* systemState, char filename[MAX_PATH])
{
  constexpr size_t PAK_MAX_MEM = 0x40000;

  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  // If there is an existing ROM, ditch it
  if (pakInterfaceState->ExternalRomBuffer != nullptr) {
    free(pakInterfaceState->ExternalRomBuffer);
  }

  // Allocate memory for the ROM
  pakInterfaceState->ExternalRomBuffer = (uint8_t*)malloc(PAK_MAX_MEM);

  // If memory was unable to be allocated, fail
  if (pakInterfaceState->ExternalRomBuffer == nullptr) {
    MessageBox(0, "cant allocate ram", "Ok", 0);

    return 0;
  }

  // Open the ROM file, fail if unable to
  FILE* rom_handle = fopen(filename, "rb");

  if (rom_handle == nullptr) return 0;

  // Load the file, one byte at a time.. (TODO: Get size and read entire block)
  int index = 0;

  while ((feof(rom_handle) == 0) && (index < PAK_MAX_MEM)) {
    pakInterfaceState->ExternalRomBuffer[index++] = fgetc(rom_handle);
  }

  fclose(rom_handle);

  UnloadDll(systemState);

  pakInterfaceState->BankedCartOffset = 0;
  pakInterfaceState->RomPackLoaded = true;

  return index;
}
