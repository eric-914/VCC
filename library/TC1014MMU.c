#include <windows.h>
#include <stdio.h>

#include "TC1014MMU.h"
#include "Graphics.h"
#include "Config.h"
#include "IOBus.h"
#include "PAKInterface.h"

#include "fileoperations.h"

#include "macros.h"

const unsigned int MemConfig[4] = { 0x20000, 0x80000, 0x200000, 0x800000 };
const unsigned short RamMask[4] = { 15, 63, 255, 1023 };
const unsigned char StateSwitch[4] = { 8, 56, 56, 56 };
const unsigned char VectorMask[4] = { 15, 63, 63, 63 };
const unsigned char VectorMaska[4] = { 12, 60, 60, 60 };
const unsigned int VidMask[4] = { 0x1FFFF, 0x7FFFF, 0x1FFFFF, 0x7FFFFF };

TC1014MmuState* InitializeInstance(TC1014MmuState*);

static TC1014MmuState* instance = InitializeInstance(new TC1014MmuState());

extern "C" {
  __declspec(dllexport) TC1014MmuState* __cdecl GetTC1014MmuState() {
    return instance;
  }
}

TC1014MmuState* InitializeInstance(TC1014MmuState* p) {
  p->MmuState = 0;
  p->MmuTask = 0;
  p->MmuEnabled = 0;
  p->RamVectors = 0;
  p->RomMap = 0;
  p->MapType = 0;
  p->CurrentRamConfig = 1;
  p->MmuPrefix = 0;

  p->Memory = NULL;
  p->InternalRomBuffer = NULL;

  ARRAYCOPY(MemConfig);
  ARRAYCOPY(RamMask);
  ARRAYCOPY(StateSwitch);
  ARRAYCOPY(VectorMask);
  ARRAYCOPY(VectorMaska);
  ARRAYCOPY(VidMask);

  return p;
}

extern "C" {
  __declspec(dllexport) unsigned short __cdecl GetMem(long address) {
    return(instance->Memory[address]);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char* __cdecl GetInternalRomPointer(void)
  {
    return(instance->InternalRomBuffer);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetMmuEnabled(unsigned char usingmmu)
  {
    instance->MmuEnabled = usingmmu;
    instance->MmuState = (!instance->MmuEnabled) << 1 | instance->MmuTask;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetMmuPrefix(unsigned char data)
  {
    instance->MmuPrefix = (data & 3) << 8;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl UpdateMmuArray(void)
  {
    if (instance->MapType) {
      instance->MemPages[instance->VectorMask[instance->CurrentRamConfig] - 3] = instance->Memory + (0x2000 * (instance->VectorMask[instance->CurrentRamConfig] - 3));
      instance->MemPages[instance->VectorMask[instance->CurrentRamConfig] - 2] = instance->Memory + (0x2000 * (instance->VectorMask[instance->CurrentRamConfig] - 2));
      instance->MemPages[instance->VectorMask[instance->CurrentRamConfig] - 1] = instance->Memory + (0x2000 * (instance->VectorMask[instance->CurrentRamConfig] - 1));
      instance->MemPages[instance->VectorMask[instance->CurrentRamConfig]] = instance->Memory + (0x2000 * instance->VectorMask[instance->CurrentRamConfig]);

      instance->MemPageOffsets[instance->VectorMask[instance->CurrentRamConfig] - 3] = 1;
      instance->MemPageOffsets[instance->VectorMask[instance->CurrentRamConfig] - 2] = 1;
      instance->MemPageOffsets[instance->VectorMask[instance->CurrentRamConfig] - 1] = 1;
      instance->MemPageOffsets[instance->VectorMask[instance->CurrentRamConfig]] = 1;

      return;
    }

    switch (instance->RomMap)
    {
    case 0:
    case 1:	//16K Internal 16K External
      instance->MemPages[instance->VectorMask[instance->CurrentRamConfig] - 3] = instance->InternalRomBuffer;
      instance->MemPages[instance->VectorMask[instance->CurrentRamConfig] - 2] = instance->InternalRomBuffer + 0x2000;
      instance->MemPages[instance->VectorMask[instance->CurrentRamConfig] - 1] = NULL;
      instance->MemPages[instance->VectorMask[instance->CurrentRamConfig]] = NULL;

      instance->MemPageOffsets[instance->VectorMask[instance->CurrentRamConfig] - 3] = 1;
      instance->MemPageOffsets[instance->VectorMask[instance->CurrentRamConfig] - 2] = 1;
      instance->MemPageOffsets[instance->VectorMask[instance->CurrentRamConfig] - 1] = 0;
      instance->MemPageOffsets[instance->VectorMask[instance->CurrentRamConfig]] = 0x2000;

      return;

    case 2:	// 32K Internal
      instance->MemPages[instance->VectorMask[instance->CurrentRamConfig] - 3] = instance->InternalRomBuffer;
      instance->MemPages[instance->VectorMask[instance->CurrentRamConfig] - 2] = instance->InternalRomBuffer + 0x2000;
      instance->MemPages[instance->VectorMask[instance->CurrentRamConfig] - 1] = instance->InternalRomBuffer + 0x4000;
      instance->MemPages[instance->VectorMask[instance->CurrentRamConfig]] = instance->InternalRomBuffer + 0x6000;

      instance->MemPageOffsets[instance->VectorMask[instance->CurrentRamConfig] - 3] = 1;
      instance->MemPageOffsets[instance->VectorMask[instance->CurrentRamConfig] - 2] = 1;
      instance->MemPageOffsets[instance->VectorMask[instance->CurrentRamConfig] - 1] = 1;
      instance->MemPageOffsets[instance->VectorMask[instance->CurrentRamConfig]] = 1;

      return;

    case 3:	//32K External
      instance->MemPages[instance->VectorMask[instance->CurrentRamConfig] - 1] = NULL;
      instance->MemPages[instance->VectorMask[instance->CurrentRamConfig]] = NULL;
      instance->MemPages[instance->VectorMask[instance->CurrentRamConfig] - 3] = NULL;
      instance->MemPages[instance->VectorMask[instance->CurrentRamConfig] - 2] = NULL;

      instance->MemPageOffsets[instance->VectorMask[instance->CurrentRamConfig] - 1] = 0;
      instance->MemPageOffsets[instance->VectorMask[instance->CurrentRamConfig]] = 0x2000;
      instance->MemPageOffsets[instance->VectorMask[instance->CurrentRamConfig] - 3] = 0x4000;
      instance->MemPageOffsets[instance->VectorMask[instance->CurrentRamConfig] - 2] = 0x6000;

      return;
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetVectors(unsigned char data)
  {
    instance->RamVectors = !!data; //Bit 3 of $FF90 MC3
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetMmuRegister(unsigned char Register, unsigned char data)
  {
    unsigned char bankRegister, task;

    bankRegister = Register & 7;
    task = !!(Register & 8);

    instance->MmuRegisters[task][bankRegister] = instance->MmuPrefix | (data & instance->RamMask[instance->CurrentRamConfig]); //gime.c returns what was written so I can get away with this
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetMmuTask(unsigned char task)
  {
    instance->MmuTask = task;
    instance->MmuState = (!instance->MmuEnabled) << 1 | instance->MmuTask;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetRomMap(unsigned char data)
  {
    instance->RomMap = (data & 3);

    UpdateMmuArray();
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetMapType(unsigned char type)
  {
    instance->MapType = type;

    UpdateMmuArray();
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MmuReset(void)
  {
    unsigned int index1 = 0, index2 = 0;

    instance->MmuTask = 0;
    instance->MmuEnabled = 0;
    instance->RamVectors = 0;
    instance->MmuState = 0;
    instance->RomMap = 0;
    instance->MapType = 0;
    instance->MmuPrefix = 0;

    for (index1 = 0;index1 < 8;index1++) {
      for (index2 = 0;index2 < 4;index2++) {
        instance->MmuRegisters[index2][index1] = index1 + instance->StateSwitch[instance->CurrentRamConfig];
      }
    }

    for (index1 = 0;index1 < 1024;index1++)
    {
      instance->MemPages[index1] = instance->Memory + ((index1 & instance->RamMask[instance->CurrentRamConfig]) * 0x2000);
      instance->MemPageOffsets[index1] = 1;
    }

    SetRomMap(0);
    SetMapType(0);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetDistoRamBank(unsigned char data)
  {
    switch (instance->CurrentRamConfig)
    {
    case 0:	// 128K
      return;
      break;

    case 1:	//512K
      return;
      break;

    case 2:	//2048K
      SetVideoBank(data & 3);
      SetMmuPrefix(0);

      return;

    case 3:	//8192K	//No Can 3 
      SetVideoBank(data & 0x0F);
      SetMmuPrefix((data & 0x30) >> 4);

      return;
    }
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl LoadInternalRom(char* filename)
  {
    unsigned short index = 0;
    FILE* rom_handle = fopen(filename, "rb");

    if (rom_handle == NULL) {
      return(0);
    }

    while ((feof(rom_handle) == 0) && (index < 0x8000)) {
      instance->InternalRomBuffer[index++] = fgetc(rom_handle);
    }

    fclose(rom_handle);

    return(index);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl CopyRom(void)
  {
    char ExecPath[MAX_PATH];
    char COCO3ROMPath[MAX_PATH];
    unsigned short temp = 0;

    GetProfileText("DefaultPaths", "COCO3ROMPath", "", COCO3ROMPath);

    strcat(COCO3ROMPath, "\\coco3.rom");

    if (COCO3ROMPath != "") {
      temp = LoadInternalRom(COCO3ROMPath);  //Try loading from the user defined path first.
    }

    if (temp) {
      OutputDebugString(" Found coco3.rom in COCO3ROMPath\n");
    }

    if (temp == 0) {
      temp = LoadInternalRom(BasicRomName());  //Try to load the image
    }

    if (temp == 0) {
      // If we can't find it use default copy
      GetModuleFileName(NULL, ExecPath, MAX_PATH);

      FilePathRemoveFileSpec(ExecPath);

      strcat(ExecPath, "coco3.rom");

      temp = LoadInternalRom(ExecPath);
    }

    if (temp == 0)
    {
      MessageBox(0, "Missing file coco3.rom", "Error", 0);

      exit(0);
    }
  }
}

/*****************************************************************************************
* MmuInit Initialize and allocate memory for RAM Internal and External ROM Images.        *
* Copy Rom Images to buffer space and reset GIME MMU registers to 0                      *
* Returns NULL if any of the above fail.                                                 *
*****************************************************************************************/
extern "C" {
  __declspec(dllexport) unsigned char* __cdecl MmuInit(unsigned char ramConfig)
  {
    unsigned int index = 0;
    unsigned int ramSize;

    ramSize = instance->MemConfig[ramConfig];

    instance->CurrentRamConfig = ramConfig;

    if (instance->Memory != NULL) {
      free(instance->Memory);
    }

    instance->Memory = (unsigned char*)malloc(ramSize);

    if (instance->Memory == NULL) {
      return(NULL);
    }

    for (index = 0;index < ramSize;index++)
    {
      instance->Memory[index] = index & 1 ? 0 : 0xFF;
    }

    GetGraphicsState()->VidMask = instance->VidMask[instance->CurrentRamConfig];

    if (instance->InternalRomBuffer != NULL) {
      free(instance->InternalRomBuffer);
    }

    instance->InternalRomBuffer = NULL;
    instance->InternalRomBuffer = (unsigned char*)malloc(0x8000);

    if (instance->InternalRomBuffer == NULL) {
      return(NULL);
    }

    memset(instance->InternalRomBuffer, 0xFF, 0x8000);
    CopyRom();
    MmuReset();

    return(instance->Memory);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl MemRead8(unsigned short address)
  {
    if (address < 0xFE00)
    {
      if (instance->MemPageOffsets[instance->MmuRegisters[instance->MmuState][address >> 13]] == 1) {
        return(instance->MemPages[instance->MmuRegisters[instance->MmuState][address >> 13]][address & 0x1FFF]);
      }

      return(PakMem8Read(instance->MemPageOffsets[instance->MmuRegisters[instance->MmuState][address >> 13]] + (address & 0x1FFF)));
    }

    if (address > 0xFEFF) {
      return (port_read(address));
    }

    if (instance->RamVectors) { //Address must be $FE00 - $FEFF
      return(instance->Memory[(0x2000 * instance->VectorMask[instance->CurrentRamConfig]) | (address & 0x1FFF)]);
    }

    if (instance->MemPageOffsets[instance->MmuRegisters[instance->MmuState][address >> 13]] == 1) {
      return(instance->MemPages[instance->MmuRegisters[instance->MmuState][address >> 13]][address & 0x1FFF]);
    }

    return(PakMem8Read(instance->MemPageOffsets[instance->MmuRegisters[instance->MmuState][address >> 13]] + (address & 0x1FFF)));
  }
}

extern "C" {
  __declspec(dllexport) unsigned short __cdecl MemRead16(unsigned short addr)
  {
    return (MemRead8(addr) << 8 | MemRead8(addr + 1));
  }
}

extern "C" {
  __declspec(dllexport) unsigned int __cdecl MemRead32(unsigned short addr)
  {
    return MemRead16(addr) << 16 | MemRead16(addr + 2);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MemWrite8(unsigned char data, unsigned short address)
  {
    if (address < 0xFE00)
    {
      if (instance->MapType || (instance->MmuRegisters[instance->MmuState][address >> 13] < instance->VectorMaska[instance->CurrentRamConfig]) || (instance->MmuRegisters[instance->MmuState][address >> 13] > instance->VectorMask[instance->CurrentRamConfig])) {
        instance->MemPages[instance->MmuRegisters[instance->MmuState][address >> 13]][address & 0x1FFF] = data;
      }

      return;
    }

    if (address > 0xFEFF)
    {
      port_write(data, address);

      return;
    }

    if (instance->RamVectors) { //Address must be $FE00 - $FEFF
      instance->Memory[(0x2000 * instance->VectorMask[instance->CurrentRamConfig]) | (address & 0x1FFF)] = data;
    }
    else if (instance->MapType || (instance->MmuRegisters[instance->MmuState][address >> 13] < instance->VectorMaska[instance->CurrentRamConfig]) || (instance->MmuRegisters[instance->MmuState][address >> 13] > instance->VectorMask[instance->CurrentRamConfig])) {
      instance->MemPages[instance->MmuRegisters[instance->MmuState][address >> 13]][address & 0x1FFF] = data;
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MemWrite16(unsigned short data, unsigned short addr)
  {
    MemWrite8(data >> 8, addr);
    MemWrite8(data & 0xFF, addr + 1);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MemWrite32(unsigned int data, unsigned short addr)
  {
    MemWrite16(data >> 16, addr);
    MemWrite16(data & 0xFFFF, addr + 2);
  }
}