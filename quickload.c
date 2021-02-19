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

#include "library/cpudef.h"
#include "library/fileoperations.h"
#include "library/systemstate.h"

#include "MemWrite8.h"
#include "InsertModule.h"

unsigned char QuickLoad(SystemState* systemState, char* binFileName)
{
  FILE* binImage = NULL;
  unsigned int memIndex = 0;
  unsigned char fileType = 0;
  unsigned short fileLength = 0;
  short startAddress = 0;
  char extension[MAX_PATH] = "";
  unsigned char* memImage = NULL;
  unsigned short xferAddress = 0;

  HANDLE hr = CreateFile(binFileName, NULL, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (hr == INVALID_HANDLE_VALUE) {
    return(1);				//File Not Found
  }

  CloseHandle(hr);
  binImage = fopen(binFileName, "rb");
  
  if (binImage == NULL) {
    return(2);				//Can't Open File
  }

  memImage = (unsigned char*)malloc(65535);
  
  if (memImage == NULL)
  {
    MessageBox(NULL, "Can't alocate ram", "Error", 0);
    return(3);				//Not enough memory
  }

  strcpy(extension, FilePathFindExtension(binFileName));
  _strlwr(extension);

  if ((strcmp(extension, ".rom") == 0) || (strcmp(extension, ".ccc") == 0) || (strcmp(extension, "*.pak") == 0))
  {
    InsertModule(systemState, binFileName);

    return(0);
  }

  if (strcmp(extension, ".bin") == 0)
  {
    while (true)
    {
      fread(memImage, sizeof(char), 5, binImage);
      fileType = memImage[0];
      fileLength = (memImage[1] << 8) + memImage[2];
      startAddress = (memImage[3] << 8) + memImage[4];

      switch (fileType)
      {
      case 0:
        fread(&memImage[0], sizeof(char), fileLength, binImage);
        
        for (memIndex = 0; memIndex < fileLength; memIndex++) { //Kluge!!!
          MemWrite8(memImage[memIndex], startAddress++);
        }

        break;

      case 255:
        xferAddress = startAddress;

        if ((xferAddress == 0) || (xferAddress > 32767) || (fileLength != 0))
        {
          MessageBox(NULL, ".Bin file is corrupt or invalid Transfer Address", "Error", 0);

          return(3);
        }

        fclose(binImage);
        free(memImage);
        GetCPU()->CPUForcePC(xferAddress);

        return(0);

      default:
        MessageBox(NULL, ".Bin file is corrupt or invalid", "Error", 0);
        fclose(binImage);
        free(memImage);

        return(3);
      }
    }
  }

  return(255); //Invalid File type
}

