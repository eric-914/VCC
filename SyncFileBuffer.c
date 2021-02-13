#include <iostream>

#include "library/cassettedef.h"
#include "library/cassettestate.h"
#include "library/defines.h"

//--CASSETTE--//

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
