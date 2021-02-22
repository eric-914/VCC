#include <windows.h>
#include <stdio.h>

#include "MMU.h"
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

MmuState* InitializeInstance(MmuState*);

static MmuState* instance = InitializeInstance(new MmuState());

extern "C" {
  __declspec(dllexport) MmuState* __cdecl GetMmuState() {
    return instance;
  }
}

MmuState* InitializeInstance(MmuState* p) {
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
    MmuState* mmuState = GetMmuState();

    return(mmuState->Memory[address]);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char* __cdecl GetInternalRomPointer(void)
  {
    MmuState* mmuState = GetMmuState();

    return(mmuState->InternalRomBuffer);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetMmuEnabled(unsigned char usingmmu)
  {
    MmuState* mmuState = GetMmuState();

    mmuState->MmuEnabled = usingmmu;
    mmuState->MmuState = (!mmuState->MmuEnabled) << 1 | mmuState->MmuTask;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetMmuPrefix(unsigned char data)
  {
    MmuState* mmuState = GetMmuState();

    mmuState->MmuPrefix = (data & 3) << 8;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl UpdateMmuArray(void)
  {
    MmuState* mmuState = GetMmuState();

    if (mmuState->MapType) {
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 3] = mmuState->Memory + (0x2000 * (mmuState->VectorMask[mmuState->CurrentRamConfig] - 3));
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 2] = mmuState->Memory + (0x2000 * (mmuState->VectorMask[mmuState->CurrentRamConfig] - 2));
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 1] = mmuState->Memory + (0x2000 * (mmuState->VectorMask[mmuState->CurrentRamConfig] - 1));
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig]] = mmuState->Memory + (0x2000 * mmuState->VectorMask[mmuState->CurrentRamConfig]);

      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 3] = 1;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 2] = 1;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 1] = 1;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig]] = 1;

      return;
    }

    switch (mmuState->RomMap)
    {
    case 0:
    case 1:	//16K Internal 16K External
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 3] = mmuState->InternalRomBuffer;
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 2] = mmuState->InternalRomBuffer + 0x2000;
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 1] = NULL;
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig]] = NULL;

      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 3] = 1;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 2] = 1;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 1] = 0;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig]] = 0x2000;

      return;

    case 2:	// 32K Internal
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 3] = mmuState->InternalRomBuffer;
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 2] = mmuState->InternalRomBuffer + 0x2000;
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 1] = mmuState->InternalRomBuffer + 0x4000;
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig]] = mmuState->InternalRomBuffer + 0x6000;

      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 3] = 1;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 2] = 1;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 1] = 1;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig]] = 1;

      return;

    case 3:	//32K External
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 1] = NULL;
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig]] = NULL;
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 3] = NULL;
      mmuState->MemPages[mmuState->VectorMask[mmuState->CurrentRamConfig] - 2] = NULL;

      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 1] = 0;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig]] = 0x2000;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 3] = 0x4000;
      mmuState->MemPageOffsets[mmuState->VectorMask[mmuState->CurrentRamConfig] - 2] = 0x6000;

      return;
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetVectors(unsigned char data)
  {
    MmuState* mmuState = GetMmuState();

    mmuState->RamVectors = !!data; //Bit 3 of $FF90 MC3
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetMmuRegister(unsigned char Register, unsigned char data)
  {
    unsigned char BankRegister, Task;

    MmuState* mmuState = GetMmuState();

    BankRegister = Register & 7;
    Task = !!(Register & 8);

    mmuState->MmuRegisters[Task][BankRegister] = mmuState->MmuPrefix | (data & mmuState->RamMask[mmuState->CurrentRamConfig]); //gime.c returns what was written so I can get away with this
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetMmuTask(unsigned char task)
  {
    MmuState* mmuState = GetMmuState();

    mmuState->MmuTask = task;
    mmuState->MmuState = (!mmuState->MmuEnabled) << 1 | mmuState->MmuTask;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetRomMap(unsigned char data)
  {
    MmuState* mmuState = GetMmuState();

    mmuState->RomMap = (data & 3);

    UpdateMmuArray();
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetMapType(unsigned char type)
  {
    MmuState* mmuState = GetMmuState();

    mmuState->MapType = type;

    UpdateMmuArray();
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MmuReset(void)
  {
    unsigned int index1 = 0, index2 = 0;

    MmuState* mmuState = GetMmuState();

    mmuState->MmuTask = 0;
    mmuState->MmuEnabled = 0;
    mmuState->RamVectors = 0;
    mmuState->MmuState = 0;
    mmuState->RomMap = 0;
    mmuState->MapType = 0;
    mmuState->MmuPrefix = 0;

    for (index1 = 0;index1 < 8;index1++) {
      for (index2 = 0;index2 < 4;index2++) {
        mmuState->MmuRegisters[index2][index1] = index1 + mmuState->StateSwitch[mmuState->CurrentRamConfig];
      }
    }

    for (index1 = 0;index1 < 1024;index1++)
    {
      mmuState->MemPages[index1] = mmuState->Memory + ((index1 & mmuState->RamMask[mmuState->CurrentRamConfig]) * 0x2000);
      mmuState->MemPageOffsets[index1] = 1;
    }

    SetRomMap(0);
    SetMapType(0);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetDistoRamBank(unsigned char data)
  {
    MmuState* mmuState = GetMmuState();

    switch (mmuState->CurrentRamConfig)
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
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl MemRead8(unsigned short address)
  {
    MmuState* mmuState = GetMmuState();

    if (address < 0xFE00)
    {
      if (mmuState->MemPageOffsets[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]] == 1) {
        return(mmuState->MemPages[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]][address & 0x1FFF]);
      }

      return(PakMem8Read(mmuState->MemPageOffsets[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]] + (address & 0x1FFF)));
    }

    if (address > 0xFEFF) {
      return (port_read(address));
    }

    if (mmuState->RamVectors) { //Address must be $FE00 - $FEFF
      return(mmuState->Memory[(0x2000 * mmuState->VectorMask[mmuState->CurrentRamConfig]) | (address & 0x1FFF)]);
    }

    if (mmuState->MemPageOffsets[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]] == 1) {
      return(mmuState->MemPages[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]][address & 0x1FFF]);
    }

    return(PakMem8Read(mmuState->MemPageOffsets[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]] + (address & 0x1FFF)));
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
    MmuState* mmuState = GetMmuState();

    if (address < 0xFE00)
    {
      if (mmuState->MapType || (mmuState->MmuRegisters[mmuState->MmuState][address >> 13] < mmuState->VectorMaska[mmuState->CurrentRamConfig]) || (mmuState->MmuRegisters[mmuState->MmuState][address >> 13] > mmuState->VectorMask[mmuState->CurrentRamConfig])) {
        mmuState->MemPages[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]][address & 0x1FFF] = data;
      }

      return;
    }

    if (address > 0xFEFF)
    {
      port_write(data, address);

      return;
    }

    if (mmuState->RamVectors) { //Address must be $FE00 - $FEFF
      mmuState->Memory[(0x2000 * mmuState->VectorMask[mmuState->CurrentRamConfig]) | (address & 0x1FFF)] = data;
    }
    else if (mmuState->MapType || (mmuState->MmuRegisters[mmuState->MmuState][address >> 13] < mmuState->VectorMaska[mmuState->CurrentRamConfig]) || (mmuState->MmuRegisters[mmuState->MmuState][address >> 13] > mmuState->VectorMask[mmuState->CurrentRamConfig])) {
      mmuState->MemPages[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]][address & 0x1FFF] = data;
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