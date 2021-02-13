#include "library/cassettedef.h"
#include "library/cassettestate.h"

//--CASSETTE--//

#include "CloseTapeFile.h"

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
