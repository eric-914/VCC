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
#include "fileops.h"

void ValidatePath(char* path)
{
  char FullExePath[MAX_PATH] = "";
  char TempPath[MAX_PATH] = "";

  GetModuleFileName(NULL, FullExePath, MAX_PATH);
  PathRemoveFileSpec(FullExePath);	  //Get path to executable
  strcpy(TempPath, path);
  PathRemoveFileSpec(TempPath);		    //Get path to Incomming file

  if (!strcmp(TempPath, FullExePath))	// If they match remove the Path
    PathStripPath(path);
}

int CheckPath(char* path)	//Return 1 on Error
{
  char TempPath[MAX_PATH] = "";
  HANDLE hr = NULL;

  if ((strlen(path) == 0) | (strlen(path) > MAX_PATH))
    return(1);

  hr = CreateFile(path, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (hr == INVALID_HANDLE_VALUE) //File Doesn't exist
  {
    GetModuleFileName(NULL, TempPath, MAX_PATH);
    PathRemoveFileSpec(TempPath);

    if ((strlen(TempPath)) + (strlen(path)) > MAX_PATH)	//Resulting path is to large Bail.
      return(1);

    strcat(TempPath, path);
    hr = CreateFile(TempPath, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hr == INVALID_HANDLE_VALUE)
      return(1);

    strcpy(path, TempPath);
  }

  CloseHandle(hr);

  return(0);
}

// These are here to remove dependance on shlwapi.dll. ASCII only
void PathStripPath(char* textBuffer)
{
  char TempBuffer[MAX_PATH] = "";
  short Index = (short)strlen(textBuffer);

  if ((Index > MAX_PATH) | (Index == 0))	//Test for overflow
    return;

  for (; Index >= 0; Index--)
    if (textBuffer[Index] == '\\')
      break;

  if (Index < 0)	//delimiter not found
    return;

  strcpy(TempBuffer, &textBuffer[Index + 1]);
  strcpy(textBuffer, TempBuffer);
}

BOOL PathRemoveFileSpec(char* path)
{
  size_t Index = strlen(path), Lenth = Index;

  if ((Index == 0) | (Index > MAX_PATH))
    return(false);

  while ((Index > 0) & (path[Index] != '\\'))
    Index--;

  while ((Index > 0) & (path[Index] == '\\'))
    Index--;

  if (Index == 0)
    return(false);

  path[Index + 2] = 0;

  return(!(strlen(path) == Lenth));
}

BOOL PathRemoveExtension(char* path)
{
  size_t Index = strlen(path), Lenth = Index;

  if ((Index == 0) | (Index > MAX_PATH))
    return(false);

  while ((Index > 0) & (path[Index--] != '.'));
  
  path[Index + 1] = 0;
  
  return(!(strlen(path) == Lenth));
}

char* PathFindExtension(char* path)
{
  size_t Index = strlen(path), Lenth = Index;

  if ((Index == 0) | (Index > MAX_PATH))
    return(&path[strlen(path) + 1]);
  
  while ((Index > 0) & (path[Index--] != '.'));
  
  return(&path[Index + 1]);
}

DWORD WritePrivateProfileInt(LPCTSTR sectionName, LPCTSTR keyName, int keyValue, LPCTSTR iniFileName)
{
  char Buffer[32] = "";
  sprintf(Buffer, "%i", keyValue);
  
  return(WritePrivateProfileString(sectionName, keyName, Buffer, iniFileName));
}
