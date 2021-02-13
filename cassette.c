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

#include "resource.h"
#include "coco3.h" //SetSndOutMode(0);
#include "config.h" //UpdateTapeCounter(TapeOffset, TapeMode); //GetProfileText("DefaultPaths", "CassPath", "", CassPath);
#include "cassettestate.h"

#include "library/cassettedef.h"
#include "library/defines.h"

void SyncFileBuffer(void);

void Motor(unsigned char state)
{
  CassetteState* cassetteState = GetCassetteState();

  cassetteState->MotorState = state;

  switch (cassetteState->MotorState)
  {
  case 0:
    SetSndOutMode(0);

    switch (cassetteState->TapeMode)
    {
    case STOP:
      break;

    case PLAY:
      cassetteState->Quiet = 30;
      cassetteState->TempIndex = 0;
      break;

    case REC:
      SyncFileBuffer();
      break;

    case EJECT:
      break;
    }
    break;	//MOTOROFF

  case 1:
    switch (cassetteState->TapeMode)
    {
    case STOP:
      SetSndOutMode(0);
      break;

    case PLAY:
      SetSndOutMode(2);
      break;

    case REC:
      SetSndOutMode(1);
      break;

    case EJECT:
      SetSndOutMode(0);
    }

    break;	//MOTORON	
  }
}

unsigned int GetTapeCounter(void)
{
  CassetteState* cassetteState = GetCassetteState();

  return(cassetteState->TapeOffset);
}

void SetTapeCounter(unsigned int count)
{
  CassetteState* cassetteState = GetCassetteState();

  cassetteState->TapeOffset = count;

  if (cassetteState->TapeOffset > cassetteState->TotalSize) {
    cassetteState->TotalSize = cassetteState->TapeOffset;
  }

  UpdateTapeCounter(cassetteState->TapeOffset, cassetteState->TapeMode);
}

void CloseTapeFile(void)
{
  CassetteState* cassetteState = GetCassetteState();

  if (cassetteState->TapeHandle == NULL) {
    return;
  }

  SyncFileBuffer();
  CloseHandle(cassetteState->TapeHandle);

  cassetteState->TapeHandle = NULL;
  cassetteState->TotalSize = 0;
}

int MountTape(char* filename)	//Return 1 on sucess 0 on fail
{
  char extension[4] = "";
  unsigned char index = 0;

  CassetteState* cassetteState = GetCassetteState();

  if (cassetteState->TapeHandle != NULL)
  {
    cassetteState->TapeMode = STOP;

    CloseTapeFile();
  }

  cassetteState->WriteProtect = 0;
  cassetteState->FileType = 0;	//0=wav 1=cas
  cassetteState->TapeHandle = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

  if (cassetteState->TapeHandle == INVALID_HANDLE_VALUE)	//Can't open read/write. try read only
  {
    cassetteState->TapeHandle = CreateFile(filename, GENERIC_READ, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    cassetteState->WriteProtect = 1;
  }

  if (cassetteState->TapeHandle == INVALID_HANDLE_VALUE)
  {
    MessageBox(0, "Can't Mount", "Error", 0);

    return(0);	//Give up
  }

  cassetteState->TotalSize = SetFilePointer(cassetteState->TapeHandle, 0, 0, FILE_END);
  cassetteState->TapeOffset = 0;

  strcpy(extension, &filename[strlen(filename) - 3]);

  for (index = 0; index < strlen(extension); index++) {
    extension[index] = toupper(extension[index]);
  }

  if (strcmp(extension, "WAV"))
  {
    cassetteState->FileType = CAS;
    cassetteState->LastTrans = 0;
    cassetteState->Mask = 0;
    cassetteState->Byte = 0;
    cassetteState->LastSample = 0;
    cassetteState->TempIndex = 0;

    if (cassetteState->CasBuffer != NULL) {
      free(cassetteState->CasBuffer);
    }

    cassetteState->CasBuffer = (unsigned char*)malloc(WRITEBUFFERSIZE);

    SetFilePointer(cassetteState->TapeHandle, 0, 0, FILE_BEGIN);
    ReadFile(cassetteState->TapeHandle, cassetteState->CasBuffer, cassetteState->TotalSize, &(cassetteState->BytesMoved), NULL);	//Read the whole file in for .CAS files

    if (cassetteState->BytesMoved != cassetteState->TotalSize) {
      return(0);
    }
  }

  return(1);
}

unsigned int LoadTape(void)
{
  static unsigned char DialogOpen = 0;
  unsigned int RetVal = 0;

  HANDLE hr = NULL;
  OPENFILENAME ofn;

  CassetteState* cassetteState = GetCassetteState();

  GetProfileText("DefaultPaths", "CassPath", "", cassetteState->CassPath);

  if (DialogOpen == 1) {	//Only allow 1 dialog open 
    return(0);
  }

  DialogOpen = 1;
  memset(&ofn, 0, sizeof(ofn));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = NULL;
  ofn.Flags = OFN_HIDEREADONLY;
  ofn.hInstance = GetModuleHandle(0);
  ofn.lpstrDefExt = "";
  ofn.lpstrFilter = "Cassette Files (*.cas)\0*.cas\0Wave Files (*.wav)\0*.wav\0\0";
  ofn.nFilterIndex = 0;								  // current filter index
  ofn.lpstrFile = cassetteState->TapeFileName;					// contains full path and filename on return
  ofn.nMaxFile = MAX_PATH;						  // sizeof lpstrFile
  ofn.lpstrFileTitle = NULL;						// filename and extension only
  ofn.nMaxFileTitle = MAX_PATH;					// sizeof lpstrFileTitle
  ofn.lpstrInitialDir = cassetteState->CassPath;				// initial directory
  ofn.lpstrTitle = "Insert Tape Image";	// title bar string

  RetVal = GetOpenFileName(&ofn);

  if (RetVal)
  {
    if (MountTape(cassetteState->TapeFileName) == 0) {
      MessageBox(NULL, "Can't open file", "Error", 0);
    }
  }

  DialogOpen = 0;
  string tmp = ofn.lpstrFile;
  size_t idx;
  idx = tmp.find_last_of("\\");
  tmp = tmp.substr(0, idx);
  strcpy(cassetteState->CassPath, tmp.c_str());

  if (cassetteState->CassPath != "") {
    WriteProfileString("DefaultPaths", "CassPath", cassetteState->CassPath);
  }

  return(RetVal);
}

void SetTapeMode(unsigned char mode)	//Handles button pressed from Dialog
{
  CassetteState* cassetteState = GetCassetteState();

  cassetteState->TapeMode = mode;

  switch (cassetteState->TapeMode)
  {
  case STOP:
    break;

  case PLAY:
    if (cassetteState->TapeHandle == NULL) {
      if (!LoadTape()) {
        cassetteState->TapeMode = STOP;
      }
      else {
        cassetteState->TapeMode = mode;
      }
    }

    if (cassetteState->MotorState) {
      Motor(1);
    }

    break;

  case REC:
    if (cassetteState->TapeHandle == NULL) {
      if (!LoadTape()) {
        cassetteState->TapeMode = STOP;
      }
      else {
        cassetteState->TapeMode = mode;
      }
    }
    break;

  case EJECT:
    CloseTapeFile();
    strcpy(cassetteState->TapeFileName, "EMPTY");

    break;
  }

  UpdateTapeCounter(cassetteState->TapeOffset, cassetteState->TapeMode);
}

void WavtoCas(unsigned char* waveBuffer, unsigned int length)
{
  unsigned char bit = 0, sample = 0;
  unsigned int index = 0, width = 0;

  CassetteState* cassetteState = GetCassetteState();

  for (index = 0; index < length; index++) {
    sample = waveBuffer[index];

    if ((cassetteState->LastSample <= 0x80) && (sample > 0x80))	//Low to High transition
    {
      width = index - cassetteState->LastTrans;

      if ((width < 10) || (width > 50))	//Invalid Sample Skip it
      {
        cassetteState->LastSample = 0;
        cassetteState->LastTrans = index;
        cassetteState->Mask = 0;
        cassetteState->Byte = 0;
      }
      else
      {
        bit = 1;

        if (width > 30) {
          bit = 0;
        }

        cassetteState->Byte = cassetteState->Byte | (bit << cassetteState->Mask);
        cassetteState->Mask++;
        cassetteState->Mask &= 7;

        if (cassetteState->Mask == 0)
        {
          cassetteState->CasBuffer[cassetteState->TapeOffset++] = cassetteState->Byte;
          cassetteState->Byte = 0;

          if (cassetteState->TapeOffset >= WRITEBUFFERSIZE) {	//Don't blow past the end of the buffer
            cassetteState->TapeMode = STOP;
          }
        }
      }

      cassetteState->LastTrans = index;
    }

    cassetteState->LastSample = sample;
  }

  cassetteState->LastTrans -= length;

  if (cassetteState->TapeOffset > cassetteState->TotalSize) {
    cassetteState->TotalSize = cassetteState->TapeOffset;
  }
}

void CastoWav(unsigned char* buffer, unsigned int bytesToConvert, unsigned long* bytesConverted)
{
  unsigned char byte = 0;
  char mask = 0;

  CassetteState* cassetteState = GetCassetteState();

  if (cassetteState->Quiet > 0)
  {
    cassetteState->Quiet--;

    memset(buffer, 0, bytesToConvert);

    return;
  }

  if ((cassetteState->TapeOffset > cassetteState->TotalSize) || (cassetteState->TotalSize == 0))	//End of tape return nothing
  {
    memset(buffer, 0, bytesToConvert);

    cassetteState->TapeMode = STOP;	//Stop at end of tape

    return;
  }

  while ((cassetteState->TempIndex < bytesToConvert) && (cassetteState->TapeOffset <= cassetteState->TotalSize))
  {
    byte = cassetteState->CasBuffer[(cassetteState->TapeOffset++) % cassetteState->TotalSize];

    for (mask = 0; mask <= 7; mask++)
    {
      if ((byte & (1 << mask)) == 0)
      {
        memcpy(&(cassetteState->TempBuffer[cassetteState->TempIndex]), cassetteState->Zero, 40);

        cassetteState->TempIndex += 40;
      }
      else
      {
        memcpy(&(cassetteState->TempBuffer[cassetteState->TempIndex]), cassetteState->One, 21);

        cassetteState->TempIndex += 21;
      }
    }
  }

  if (cassetteState->TempIndex >= bytesToConvert)
  {
    memcpy(buffer, cassetteState->TempBuffer, bytesToConvert); //Fill the return Buffer
    memcpy(cassetteState->TempBuffer, &(cassetteState->TempBuffer[bytesToConvert]), cassetteState->TempIndex - bytesToConvert);	//Slide the overage to the front

    cassetteState->TempIndex -= bytesToConvert; //Point to the Next free byte in the tempbuffer
  }
  else	//We ran out of source bytes
  {
    memcpy(buffer, cassetteState->TempBuffer, cassetteState->TempIndex);						//Partial Fill of return buffer;
    memset(&buffer[cassetteState->TempIndex], 0, bytesToConvert - cassetteState->TempIndex);		//and silence for the rest

    cassetteState->TempIndex = 0;
  }
}

void FlushCassetteBuffer(unsigned char* buffer, unsigned int length)
{
  CassetteState* cassetteState = GetCassetteState();

  if (cassetteState->TapeMode != REC) {
    return;
  }

  switch (cassetteState->FileType)
  {
  case WAV:
    SetFilePointer(cassetteState->TapeHandle, cassetteState->TapeOffset + 44, 0, FILE_BEGIN);
    WriteFile(cassetteState->TapeHandle, buffer, length, &(cassetteState->BytesMoved), NULL);

    if (length != cassetteState->BytesMoved) {
      return;
    }

    cassetteState->TapeOffset += length;

    if (cassetteState->TapeOffset > cassetteState->TotalSize) {
      cassetteState->TotalSize = cassetteState->TapeOffset;
    }

    break;

  case CAS:
    WavtoCas(buffer, length);

    break;
  }

  UpdateTapeCounter(cassetteState->TapeOffset, cassetteState->TapeMode);
}

void LoadCassetteBuffer(unsigned char* cassBuffer)
{
  CassetteState* cassetteState = GetCassetteState();

  unsigned long bytesMoved = 0;

  if (cassetteState->TapeMode != PLAY) {
    return;
  }

  switch (cassetteState->FileType)
  {
  case WAV:
    SetFilePointer(cassetteState->TapeHandle, cassetteState->TapeOffset + 44, 0, FILE_BEGIN);
    ReadFile(cassetteState->TapeHandle, cassBuffer, TAPEAUDIORATE / 60, &bytesMoved, NULL);

    cassetteState->TapeOffset += bytesMoved;

    if (cassetteState->TapeOffset > cassetteState->TotalSize) {
      cassetteState->TapeOffset = cassetteState->TotalSize;
    }

    break;

  case CAS:
    CastoWav(cassBuffer, TAPEAUDIORATE / 60, &bytesMoved);

    break;
  }

  UpdateTapeCounter(cassetteState->TapeOffset, cassetteState->TapeMode);
}

void GetTapeName(char* name)
{
  CassetteState* cassetteState = GetCassetteState();

  strcpy(name, cassetteState->TapeFileName);
}

void SyncFileBuffer(void)
{
  char buffer[64] = "";
  unsigned long bytesMoved = 0;
  unsigned int fileSize;
  unsigned short waveType = 1;		//WAVE type format
  unsigned int formatSize = 16;		//size of WAVE section chunck
  unsigned short channels = 1;		//mono/stereo
  unsigned int bitRate = TAPEAUDIORATE;		//sample rate
  unsigned short bitsperSample = 8;	//Bits/sample
  unsigned int bytesperSec = bitRate * channels * (bitsperSample / 8);		//bytes/sec
  unsigned short blockAlign = (bitsperSample * channels) / 8;		//Block alignment
  unsigned int chunkSize;

  CassetteState* cassetteState = GetCassetteState();

  fileSize = cassetteState->TotalSize + 40 - 8;
  chunkSize = fileSize;

  SetFilePointer(cassetteState->TapeHandle, 0, 0, FILE_BEGIN);

  switch (cassetteState->FileType)
  {
  case CAS:
    cassetteState->CasBuffer[cassetteState->TapeOffset] = cassetteState->Byte;	//capture the last byte
    cassetteState->LastTrans = 0;	//reset all static inter-call variables
    cassetteState->Mask = 0;
    cassetteState->Byte = 0;
    cassetteState->LastSample = 0;
    cassetteState->TempIndex = 0;

    WriteFile(cassetteState->TapeHandle, cassetteState->CasBuffer, cassetteState->TapeOffset, &bytesMoved, NULL);

    break;

  case WAV:
    sprintf(buffer, "RIFF");

    WriteFile(cassetteState->TapeHandle, buffer, 4, &bytesMoved, NULL);
    WriteFile(cassetteState->TapeHandle, &fileSize, 4, &bytesMoved, NULL);

    sprintf(buffer, "WAVE");

    WriteFile(cassetteState->TapeHandle, buffer, 4, &bytesMoved, NULL);

    sprintf(buffer, "fmt ");

    WriteFile(cassetteState->TapeHandle, buffer, 4, &bytesMoved, NULL);
    WriteFile(cassetteState->TapeHandle, &formatSize, 4, &bytesMoved, NULL);
    WriteFile(cassetteState->TapeHandle, &waveType, 2, &bytesMoved, NULL);
    WriteFile(cassetteState->TapeHandle, &channels, 2, &bytesMoved, NULL);
    WriteFile(cassetteState->TapeHandle, &bitRate, 4, &bytesMoved, NULL);
    WriteFile(cassetteState->TapeHandle, &bytesperSec, 4, &bytesMoved, NULL);
    WriteFile(cassetteState->TapeHandle, &blockAlign, 2, &bytesMoved, NULL);
    WriteFile(cassetteState->TapeHandle, &bitsperSample, 2, &bytesMoved, NULL);

    sprintf(buffer, "data");

    WriteFile(cassetteState->TapeHandle, buffer, 4, &bytesMoved, NULL);
    WriteFile(cassetteState->TapeHandle, &chunkSize, 4, &bytesMoved, NULL);

    break;
  }

  FlushFileBuffers(cassetteState->TapeHandle);
}
