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
#include "resource.h" 
#include "defines.h"
#include "memboard.h"

typedef unsigned char (*MEMREAD8)(unsigned short);
typedef void (*MEMWRITE8)(unsigned char, unsigned short);
typedef void (*ASSERTINTERRUPT) (unsigned char, unsigned char);
typedef void (*DMAMEMPOINTERS) (MEMREAD8, MEMWRITE8);
typedef void (*DYNAMICMENUCALLBACK)(char*, int, int);
static void (*AssertInt)(unsigned char, unsigned char) = NULL;
static unsigned char (*MemRead8)(unsigned short);
static void (*MemWrite8)(unsigned char, unsigned short);
static unsigned char* Memory = NULL;
static char FileName[MAX_PATH] = "";
static char IniFile[MAX_PATH] = "";
static void (*DynamicMenuCallback)(char*, int, int) = NULL;

static HINSTANCE g_hinstDLL;

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,  // handle to DLL module
  DWORD fdwReason,     // reason for calling function
  LPVOID lpReserved)   // reserved
{
  switch (fdwReason)
  {
  case DLL_PROCESS_ATTACH:
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
    g_hinstDLL = hinstDLL;
    break;

  case DLL_PROCESS_DETACH:
    break;
  }

  return TRUE;
}

extern "C"
{
  __declspec(dllexport) void ModuleName(char* moduleName, char* catNumber, DYNAMICMENUCALLBACK menuCallback)
  {
    LoadString(g_hinstDLL, IDS_MODULE_NAME, moduleName, MAX_LOADSTRING);
    LoadString(g_hinstDLL, IDS_CATNUMBER, catNumber, MAX_LOADSTRING);
    InitMemBoard();
    DynamicMenuCallback = menuCallback;
  }
}


extern "C"
{
  __declspec(dllexport) void PackPortWrite(unsigned char port, unsigned char data)
  {
    switch (port)
    {
    case 0x40:
    case 0x41:
    case 0x42:
      WritePort(port, data);
      return;
      break;

    case 0x43:
      WriteArray(data);
      return;
      break;

    default:
      return;
      break;
    }
  }
}

extern "C"
{
  __declspec(dllexport) unsigned char PackPortRead(unsigned char port)
  {
    switch (port)
    {
    case 0x43:
      return(ReadArray());
      break;

    default:
      return(0);
      break;
    }
  }
}
