/*
Copyright 2015 by Joseph Forgione
This file is part of VCC (Virtual Color Computer).

    VCC (Virtual Color Computer) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    VCC (Virtual Color Computer) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with VCC (Virtual Color Computer).  If not, see <http://www.gnu.org/licenses/>.
*/

#include <windows.h>
#include <iostream>

#include "mmustate.h"
#include "tcc1014mmu.h"

#include "ConfigAccessors.h"
#include "ProfileAccessors.h"
#include "port_read.h"
#include "port_write.h"
#include "PakInterfaceAccessors.h"
#include "SetVideoBank.h"

#include "library\fileoperations.h"
#include "library\graphicsstate.h"

void UpdateMmuArray(void);
int load_int_rom(TCHAR filename[MAX_PATH]);

/*****************************************************************************************
* MmuInit Initilize and allocate memory for RAM Internal and External ROM Images.        *
* Copy Rom Images to buffer space and reset GIME MMU registers to 0                      *
* Returns NULL if any of the above fail.                                                 *
*****************************************************************************************/
unsigned char* MmuInit(unsigned char RamConfig)
{
  unsigned int index = 0;
  unsigned int ramSize;

  MmuState* mmuState = GetMmuState();

  ramSize = mmuState->MemConfig[RamConfig];

  mmuState->CurrentRamConfig = RamConfig;

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

void MmuReset(void)
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

void SetVectors(unsigned char data)
{
  MmuState* mmuState = GetMmuState();

  mmuState->RamVectors = !!data; //Bit 3 of $FF90 MC3
}

void SetMmuRegister(unsigned char Register, unsigned char data)
{
  unsigned char BankRegister, Task;

  MmuState* mmuState = GetMmuState();

  BankRegister = Register & 7;
  Task = !!(Register & 8);

  mmuState->MmuRegisters[Task][BankRegister] = mmuState->MmuPrefix | (data & mmuState->RamMask[mmuState->CurrentRamConfig]); //gime.c returns what was written so I can get away with this
}

void SetRomMap(unsigned char data)
{
  MmuState* mmuState = GetMmuState();

  mmuState->RomMap = (data & 3);

  UpdateMmuArray();
}

void SetMapType(unsigned char type)
{
  MmuState* mmuState = GetMmuState();

  mmuState->MapType = type;

  UpdateMmuArray();
}

void Set_MmuTask(unsigned char task)
{
  MmuState* mmuState = GetMmuState();

  mmuState->MmuTask = task;
  mmuState->MmuState = (!mmuState->MmuEnabled) << 1 | mmuState->MmuTask;
}

void Set_MmuEnabled(unsigned char usingmmu)
{
  MmuState* mmuState = GetMmuState();

  mmuState->MmuEnabled = usingmmu;
  mmuState->MmuState = (!mmuState->MmuEnabled) << 1 | mmuState->MmuTask;
}

unsigned char* Getint_rom_pointer(void)
{
  MmuState* mmuState = GetMmuState();

  return(mmuState->InternalRomBuffer);
}

void CopyRom(void)
{
  char ExecPath[MAX_PATH];
  char COCO3ROMPath[MAX_PATH];
  unsigned short temp = 0;

  GetProfileText("DefaultPaths", "COCO3ROMPath", "", COCO3ROMPath);

  strcat(COCO3ROMPath, "\\coco3.rom");

  if (COCO3ROMPath != "") {
    temp = load_int_rom(COCO3ROMPath);  //Try loading from the user defined path first.
  }

  if (temp) {
    OutputDebugString(" Found coco3.rom in COCO3ROMPath\n");
  }

  if (temp == 0) {
    temp = load_int_rom(BasicRomName());  //Try to load the image
  }

  if (temp == 0) {
    // If we can't find it use default copy
    GetModuleFileName(NULL, ExecPath, MAX_PATH);

    FilePathRemoveFileSpec(ExecPath);

    strcat(ExecPath, "coco3.rom");

    temp = load_int_rom(ExecPath);
  }

  if (temp == 0)
  {
    MessageBox(0, "Missing file coco3.rom", "Error", 0);

    exit(0);
  }
}

int load_int_rom(TCHAR filename[MAX_PATH])
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

// Coco3 MMU Code
unsigned char MemRead8(unsigned short address)
{
  MmuState* mmuState = GetMmuState();

  if (address < 0xFE00)
  {
    if (mmuState->MemPageOffsets[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]] == 1) {
      return(mmuState->MemPages[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]][address & 0x1FFF]);
    }

    return(PackMem8Read(mmuState->MemPageOffsets[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]] + (address & 0x1FFF)));
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

  return(PackMem8Read(mmuState->MemPageOffsets[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]] + (address & 0x1FFF)));
}

void MemWrite8(unsigned char data, unsigned short address)
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

unsigned char __fastcall fMemRead8(unsigned short address)
{
  MmuState* mmuState = GetMmuState();

  if (address < 0xFE00) {
    if (mmuState->MemPageOffsets[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]] == 1) {
      return(mmuState->MemPages[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]][address & 0x1FFF]);
    }

    return(PackMem8Read(mmuState->MemPageOffsets[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]] + (address & 0x1FFF)));
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

  return(PackMem8Read(mmuState->MemPageOffsets[mmuState->MmuRegisters[mmuState->MmuState][address >> 13]] + (address & 0x1FFF)));
}

void __fastcall fMemWrite8(unsigned char data, unsigned short address)
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

/*****************************************************************
* 16 bit memory handling routines                                *
*****************************************************************/

unsigned short MemRead16(unsigned short addr)
{
  return (MemRead8(addr) << 8 | MemRead8(addr + 1));
}

void MemWrite16(unsigned short data, unsigned short addr)
{
  MemWrite8(data >> 8, addr);
  MemWrite8(data & 0xFF, addr + 1);
}

unsigned short GetMem(long address) {
  MmuState* mmuState = GetMmuState();

  return(mmuState->Memory[address]);
}

void SetMem(long address, unsigned short data) {
  MmuState* mmuState = GetMmuState();

  mmuState->Memory[address] = (unsigned char)data;
}

void SetDistoRamBank(unsigned char data)
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

void SetMmuPrefix(unsigned char data)
{
  MmuState* mmuState = GetMmuState();

  mmuState->MmuPrefix = (data & 3) << 8;
}

void UpdateMmuArray(void)
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
