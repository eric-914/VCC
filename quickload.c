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
#include <stdio.h>

#include "pakinterface.h" //InsertModule
#include "tcc1014mmu.h" //MemWrite8

#include "library/cpudef.h"
#include "library/fileoperations.h"

static unsigned short XferAddress = 0;

unsigned char QuickLoad(char* binFileName)
{
  FILE* binImage = NULL;
  unsigned int memIndex = 0;
  unsigned char fileType = 0;
  unsigned short fileLength = 0;
  short startAddress = 0;
  char Extension[MAX_PATH] = "";
  unsigned char* MemImage = NULL;

  HANDLE hr = CreateFile(binFileName, NULL, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (hr == INVALID_HANDLE_VALUE) {
    return(1);				//File Not Found
  }

  CloseHandle(hr);
  binImage = fopen(binFileName, "rb");
  
  if (binImage == NULL) {
    return(2);				//Can't Open File
  }

  MemImage = (unsigned char*)malloc(65535);
  
  if (MemImage == NULL)
  {
    MessageBox(NULL, "Can't alocate ram", "Error", 0);
    return(3);				//Not enough memory
  }

  strcpy(Extension, FilePathFindExtension(binFileName));
  _strlwr(Extension);

  if ((strcmp(Extension, ".rom") == 0) || (strcmp(Extension, ".ccc") == 0) || (strcmp(Extension, "*.pak") == 0))
  {
    InsertModule(binFileName);

    return(0);
  }

  if (strcmp(Extension, ".bin") == 0)
  {
    while (true)
    {
      fread(MemImage, sizeof(char), 5, binImage);
      fileType = MemImage[0];
      fileLength = (MemImage[1] << 8) + MemImage[2];
      startAddress = (MemImage[3] << 8) + MemImage[4];

      switch (fileType)
      {
      case 0:
        fread(&MemImage[0], sizeof(char), fileLength, binImage);
        
        for (memIndex = 0; memIndex < fileLength; memIndex++) { //Kluge!!!
          MemWrite8(MemImage[memIndex], startAddress++);
        }

        break;

      case 255:
        XferAddress = startAddress;

        if ((XferAddress == 0) || (XferAddress > 32767) || (fileLength != 0))
        {
          MessageBox(NULL, ".Bin file is corrupt or invalid Transfer Address", "Error", 0);

          return(3);
        }

        fclose(binImage);
        free(MemImage);
        GetCPU()->CPUForcePC(XferAddress);

        return(0);

      default:
        MessageBox(NULL, ".Bin file is corrupt or invalid", "Error", 0);
        fclose(binImage);
        free(MemImage);

        return(3);
      }
    }
  }

  return(255); //Invalid File type
}

unsigned short GetXferAddr(void)
{
  return(XferAddress);
}
