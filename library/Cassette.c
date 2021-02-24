#include <iostream>

#include "Cassette.h"

#include "defines.h"
#include "macros.h"

#include "Config.h"
#include "CoCo.h"

const unsigned char One[21] = { 0x80, 0xA8, 0xC8, 0xE8, 0xE8, 0xF8, 0xF8, 0xE8, 0xC8, 0xA8, 0x78, 0x50, 0x50, 0x30, 0x10, 0x00, 0x00, 0x10, 0x30, 0x30, 0x50 };
const unsigned char Zero[40] = { 0x80, 0x90, 0xA8, 0xB8, 0xC8, 0xD8, 0xE8, 0xE8, 0xF0, 0xF8, 0xF8, 0xF8, 0xF0, 0xE8, 0xD8, 0xC8, 0xB8, 0xA8, 0x90, 0x78, 0x78, 0x68, 0x50, 0x40, 0x30, 0x20, 0x10, 0x08, 0x00, 0x00, 0x00, 0x08, 0x10, 0x10, 0x20, 0x30, 0x40, 0x50, 0x68, 0x68 };

CassetteState* InitializeInstance(CassetteState*);

static CassetteState* instance = InitializeInstance(new CassetteState());

extern "C" {
  __declspec(dllexport) CassetteState* __cdecl GetCassetteState() {
    return instance;
  }
}

CassetteState* InitializeInstance(CassetteState* p) {
  p->Byte = 0;
  p->BytesMoved = 0;
  p->FileType = 0;
  p->LastSample = 0;
  p->LastTrans = 0;
  p->Mask = 0;
  p->MotorState = 0;
  p->Quiet = 30;
  p->TapeMode = STOP;
  p->TapeOffset = 0;
  p->TempIndex = 0;
  p->TotalSize = 0;
  p->WriteProtect = 0;

  p->CasBuffer = NULL;
  p->TapeHandle = NULL;

  strcpy(p->TapeFileName, "");

  ARRAYCOPY(Zero);
  ARRAYCOPY(One);

  return p;
}

extern "C" {
  __declspec(dllexport) void __cdecl CastoWav(unsigned char* buffer, unsigned int bytesToConvert, unsigned long* bytesConverted)
  {
    unsigned char byte = 0;
    char mask = 0;

    if (instance->Quiet > 0)
    {
      instance->Quiet--;

      memset(buffer, 0, bytesToConvert);

      return;
    }

    if ((instance->TapeOffset > instance->TotalSize) || (instance->TotalSize == 0))	//End of tape return nothing
    {
      memset(buffer, 0, bytesToConvert);

      instance->TapeMode = STOP;	//Stop at end of tape

      return;
    }

    while ((instance->TempIndex < bytesToConvert) && (instance->TapeOffset <= instance->TotalSize))
    {
      byte = instance->CasBuffer[(instance->TapeOffset++) % instance->TotalSize];

      for (mask = 0; mask <= 7; mask++)
      {
        if ((byte & (1 << mask)) == 0)
        {
          memcpy(&(instance->TempBuffer[instance->TempIndex]), instance->Zero, 40);

          instance->TempIndex += 40;
        }
        else
        {
          memcpy(&(instance->TempBuffer[instance->TempIndex]), instance->One, 21);

          instance->TempIndex += 21;
        }
      }
    }

    if (instance->TempIndex >= bytesToConvert)
    {
      memcpy(buffer, instance->TempBuffer, bytesToConvert); //Fill the return Buffer
      memcpy(instance->TempBuffer, &(instance->TempBuffer[bytesToConvert]), instance->TempIndex - bytesToConvert);	//Slide the overage to the front

      instance->TempIndex -= bytesToConvert; //Point to the Next free byte in the tempbuffer
    }
    else	//We ran out of source bytes
    {
      memcpy(buffer, instance->TempBuffer, instance->TempIndex);						//Partial Fill of return buffer;
      memset(&buffer[instance->TempIndex], 0, bytesToConvert - instance->TempIndex);		//and silence for the rest

      instance->TempIndex = 0;
    }
  }
}

extern "C" {
  __declspec(dllexport) unsigned int __cdecl GetTapeCounter(void)
  {
    return(instance->TapeOffset);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl GetTapeName(char* name)
  {
    strcpy(name, instance->TapeFileName);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SyncFileBuffer(void)
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

    fileSize = instance->TotalSize + 40 - 8;
    chunkSize = fileSize;

    SetFilePointer(instance->TapeHandle, 0, 0, FILE_BEGIN);

    switch (instance->FileType)
    {
    case CAS:
      instance->CasBuffer[instance->TapeOffset] = instance->Byte;	//capture the last byte
      instance->LastTrans = 0;	//reset all static inter-call variables
      instance->Mask = 0;
      instance->Byte = 0;
      instance->LastSample = 0;
      instance->TempIndex = 0;

      WriteFile(instance->TapeHandle, instance->CasBuffer, instance->TapeOffset, &bytesMoved, NULL);

      break;

    case WAV:
      sprintf(buffer, "RIFF");

      WriteFile(instance->TapeHandle, buffer, 4, &bytesMoved, NULL);
      WriteFile(instance->TapeHandle, &fileSize, 4, &bytesMoved, NULL);

      sprintf(buffer, "WAVE");

      WriteFile(instance->TapeHandle, buffer, 4, &bytesMoved, NULL);

      sprintf(buffer, "fmt ");

      WriteFile(instance->TapeHandle, buffer, 4, &bytesMoved, NULL);
      WriteFile(instance->TapeHandle, &formatSize, 4, &bytesMoved, NULL);
      WriteFile(instance->TapeHandle, &waveType, 2, &bytesMoved, NULL);
      WriteFile(instance->TapeHandle, &channels, 2, &bytesMoved, NULL);
      WriteFile(instance->TapeHandle, &bitRate, 4, &bytesMoved, NULL);
      WriteFile(instance->TapeHandle, &bytesperSec, 4, &bytesMoved, NULL);
      WriteFile(instance->TapeHandle, &blockAlign, 2, &bytesMoved, NULL);
      WriteFile(instance->TapeHandle, &bitsperSample, 2, &bytesMoved, NULL);

      sprintf(buffer, "data");

      WriteFile(instance->TapeHandle, buffer, 4, &bytesMoved, NULL);
      WriteFile(instance->TapeHandle, &chunkSize, 4, &bytesMoved, NULL);

      break;
    }

    FlushFileBuffers(instance->TapeHandle);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl CloseTapeFile(void)
  {
    if (instance->TapeHandle == NULL) {
      return;
    }

    SyncFileBuffer();
    CloseHandle(instance->TapeHandle);

    instance->TapeHandle = NULL;
    instance->TotalSize = 0;
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl MountTape(char* filename)	//Return 1 on sucess 0 on fail
  {
    char extension[4] = "";
    unsigned char index = 0;

    if (instance->TapeHandle != NULL)
    {
      instance->TapeMode = STOP;

      CloseTapeFile();
    }

    instance->WriteProtect = 0;
    instance->FileType = 0;	//0=wav 1=cas
    instance->TapeHandle = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if (instance->TapeHandle == INVALID_HANDLE_VALUE)	//Can't open read/write. try read only
    {
      instance->TapeHandle = CreateFile(filename, GENERIC_READ, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
      instance->WriteProtect = 1;
    }

    if (instance->TapeHandle == INVALID_HANDLE_VALUE)
    {
      MessageBox(0, "Can't Mount", "Error", 0);

      return(0);	//Give up
    }

    instance->TotalSize = SetFilePointer(instance->TapeHandle, 0, 0, FILE_END);
    instance->TapeOffset = 0;

    strcpy(extension, &filename[strlen(filename) - 3]);

    for (index = 0; index < strlen(extension); index++) {
      extension[index] = toupper(extension[index]);
    }

    if (strcmp(extension, "WAV"))
    {
      instance->FileType = CAS;
      instance->LastTrans = 0;
      instance->Mask = 0;
      instance->Byte = 0;
      instance->LastSample = 0;
      instance->TempIndex = 0;

      if (instance->CasBuffer != NULL) {
        free(instance->CasBuffer);
      }

      instance->CasBuffer = (unsigned char*)malloc(WRITEBUFFERSIZE);

      SetFilePointer(instance->TapeHandle, 0, 0, FILE_BEGIN);

      ReadFile(instance->TapeHandle, instance->CasBuffer, instance->TotalSize, &(instance->BytesMoved), NULL);	//Read the whole file in for .CAS files

      if (instance->BytesMoved != instance->TotalSize) {
        return(0);
      }
    }

    return(1);
  }
}

extern "C" {
  __declspec(dllexport) unsigned int __cdecl LoadTape(void)
  {
    static unsigned char DialogOpen = 0;
    unsigned int RetVal = 0;

    HANDLE hr = NULL;
    OPENFILENAME ofn;

    GetProfileText("DefaultPaths", "CassPath", "", instance->CassPath);

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
    ofn.lpstrFile = instance->TapeFileName;					// contains full path and filename on return
    ofn.nMaxFile = MAX_PATH;						  // sizeof lpstrFile
    ofn.lpstrFileTitle = NULL;						// filename and extension only
    ofn.nMaxFileTitle = MAX_PATH;					// sizeof lpstrFileTitle
    ofn.lpstrInitialDir = instance->CassPath;				// initial directory
    ofn.lpstrTitle = "Insert Tape Image";	// title bar string

    RetVal = GetOpenFileName(&ofn);

    if (RetVal)
    {
      if (MountTape(instance->TapeFileName) == 0) {
        MessageBox(NULL, "Can't open file", "Error", 0);
      }
    }

    DialogOpen = 0;
    std::string tmp = ofn.lpstrFile;
    size_t idx;
    idx = tmp.find_last_of("\\");
    tmp = tmp.substr(0, idx);
    strcpy(instance->CassPath, tmp.c_str());

    if (instance->CassPath != "") {
      WriteProfileString("DefaultPaths", "CassPath", instance->CassPath);
    }

    return(RetVal);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl WavtoCas(unsigned char* waveBuffer, unsigned int length)
  {
    unsigned char bit = 0, sample = 0;
    unsigned int index = 0, width = 0;

    for (index = 0; index < length; index++) {
      sample = waveBuffer[index];

      if ((instance->LastSample <= 0x80) && (sample > 0x80))	//Low to High transition
      {
        width = index - instance->LastTrans;

        if ((width < 10) || (width > 50))	//Invalid Sample Skip it
        {
          instance->LastSample = 0;
          instance->LastTrans = index;
          instance->Mask = 0;
          instance->Byte = 0;
        }
        else
        {
          bit = 1;

          if (width > 30) {
            bit = 0;
          }

          instance->Byte = instance->Byte | (bit << instance->Mask);
          instance->Mask++;
          instance->Mask &= 7;

          if (instance->Mask == 0)
          {
            instance->CasBuffer[instance->TapeOffset++] = instance->Byte;
            instance->Byte = 0;

            if (instance->TapeOffset >= WRITEBUFFERSIZE) {	//Don't blow past the end of the buffer
              instance->TapeMode = STOP;
            }
          }
        }

        instance->LastTrans = index;
      }

      instance->LastSample = sample;
    }

    instance->LastTrans -= length;

    if (instance->TapeOffset > instance->TotalSize) {
      instance->TotalSize = instance->TapeOffset;
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl FlushCassetteBuffer(unsigned char* buffer, unsigned int length)
  {
    if (instance->TapeMode != REC) {
      return;
    }

    switch (instance->FileType)
    {
    case WAV:
      SetFilePointer(instance->TapeHandle, instance->TapeOffset + 44, 0, FILE_BEGIN);
      WriteFile(instance->TapeHandle, buffer, length, &(instance->BytesMoved), NULL);

      if (length != instance->BytesMoved) {
        return;
      }

      instance->TapeOffset += length;

      if (instance->TapeOffset > instance->TotalSize) {
        instance->TotalSize = instance->TapeOffset;
      }

      break;

    case CAS:
      WavtoCas(buffer, length);

      break;
    }

    UpdateTapeCounter(instance->TapeOffset, instance->TapeMode);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl LoadCassetteBuffer(unsigned char* cassBuffer)
  {
    unsigned long bytesMoved = 0;

    if (instance->TapeMode != PLAY) {
      return;
    }

    switch (instance->FileType)
    {
    case WAV:
      SetFilePointer(instance->TapeHandle, instance->TapeOffset + 44, 0, FILE_BEGIN);
      ReadFile(instance->TapeHandle, cassBuffer, TAPEAUDIORATE / 60, &bytesMoved, NULL);

      instance->TapeOffset += bytesMoved;

      if (instance->TapeOffset > instance->TotalSize) {
        instance->TapeOffset = instance->TotalSize;
      }

      break;

    case CAS:
      CastoWav(cassBuffer, TAPEAUDIORATE / 60, &bytesMoved);

      break;
    }

    UpdateTapeCounter(instance->TapeOffset, instance->TapeMode);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetTapeCounter(unsigned int count)
  {
    instance->TapeOffset = count;

    if (instance->TapeOffset > instance->TotalSize) {
      instance->TotalSize = instance->TapeOffset;
    }

    UpdateTapeCounter(instance->TapeOffset, instance->TapeMode);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl Motor(unsigned char state)
  {
    instance->MotorState = state;

    switch (instance->MotorState)
    {
    case 0:
      SetSndOutMode(0);

      switch (instance->TapeMode)
      {
      case STOP:
        break;

      case PLAY:
        instance->Quiet = 30;
        instance->TempIndex = 0;
        break;

      case REC:
        SyncFileBuffer();
        break;

      case EJECT:
        break;
      }
      break;	//MOTOROFF

    case 1:
      switch (instance->TapeMode)
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
}

extern "C" {
  __declspec(dllexport) void __cdecl SetTapeMode(unsigned char mode)	//Handles button pressed from Dialog
  {
    instance->TapeMode = mode;

    switch (instance->TapeMode)
    {
    case STOP:
      break;

    case PLAY:
      if (instance->TapeHandle == NULL) {
        if (!LoadTape()) {
          instance->TapeMode = STOP;
        }
        else {
          instance->TapeMode = mode;
        }
      }

      if (instance->MotorState) {
        Motor(1);
      }

      break;

    case REC:
      if (instance->TapeHandle == NULL) {
        if (!LoadTape()) {
          instance->TapeMode = STOP;
        }
        else {
          instance->TapeMode = mode;
        }
      }
      break;

    case EJECT:
      CloseTapeFile();
      strcpy(instance->TapeFileName, "EMPTY");

      break;
    }

    UpdateTapeCounter(instance->TapeOffset, instance->TapeMode);
  }
}
