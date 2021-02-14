#include <windows.h>
#include <direct.h>
#include <ShlObj.h>

#include "configstate.h"
#include "resource.h"

#include "BuildTransDisp2ScanTable.h"
#include "ReadIniFile.h"
#include "WriteIniFile.h"
#include "RefreshJoystickStatus.h"
#include "UpdateConfig.h"

#include "library/commandline.h"
#include "library/systemstate.h"

extern int GetSoundCardList(SndCardList* list);
extern int SoundInit(HWND main_window_handle, _GUID* guid, unsigned short rate);

extern "C" __declspec(dllexport) BOOL __cdecl FilePathRemoveFileSpec(char* path);

void LoadConfig(SystemState* systemState, CmdLineArguments cmdArg)
{
  HANDLE hr = NULL;
  int lasterror;
  char iniFileName[] = "Vcc.ini";

  ConfigState* configState = GetConfigState();

  BuildTransDisp2ScanTable();

  LoadString(NULL, IDS_APP_TITLE, configState->AppName, MAX_LOADSTRING);
  GetModuleFileName(NULL, configState->ExecDirectory, MAX_PATH);

  FilePathRemoveFileSpec(configState->ExecDirectory);

  if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, configState->AppDataPath))) {
    OutputDebugString(configState->AppDataPath);
  }

  strcpy(configState->CurrentConfig.PathtoExe, configState->ExecDirectory);
  strcat(configState->AppDataPath, "\\VCC");

  if (_mkdir(configState->AppDataPath) != 0) {
    OutputDebugString("Unable to create VCC config folder.");
  }

  if (*cmdArg.IniFile) {
    GetFullPathNameA(cmdArg.IniFile, MAX_PATH, configState->IniFilePath, 0);
  }
  else {
    strcpy(configState->IniFilePath, configState->AppDataPath);
    strcat(configState->IniFilePath, "\\");
    strcat(configState->IniFilePath, iniFileName);
  }

  systemState->ScanLines = 0;

  configState->NumberOfSoundCards = GetSoundCardList(configState->SoundCards);

  ReadIniFile();

  configState->CurrentConfig.RebootNow = 0;

  UpdateConfig(systemState);
  RefreshJoystickStatus();

  SoundInit(systemState->WindowHandle, configState->SoundCards[configState->CurrentConfig.SndOutDev].Guid, configState->CurrentConfig.AudioRate);

  //  Try to open the config file.  Create it if necessary.  Abort if failure.
  hr = CreateFile(configState->IniFilePath,
    GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
    NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  lasterror = GetLastError();

  if (hr == INVALID_HANDLE_VALUE) { // Fatal could not open ini file
    MessageBox(0, "Could not open ini file", "Error", 0);

    exit(0);
  }
  else {
    CloseHandle(hr);

    if (lasterror != ERROR_ALREADY_EXISTS) WriteIniFile();  //!=183
  }
}
