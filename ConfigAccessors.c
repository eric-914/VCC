#include "configstate.h"

char* BasicRomName(void)
{
  return(GetConfigState()->CurrentConfig.ExternalBasicImage);
}

void GetIniFilePath(char* path)
{
  strcpy(path, GetConfigState()->IniFilePath);
}

void SetIniFilePath(char* path)
{
  //  Path must be to an existing ini file
  strcpy(GetConfigState()->IniFilePath, path);
}

char* AppDirectory()
{
  // This only works after LoadConfig has been called
  return GetConfigState()->AppDataPath;
}

int GetKeyboardLayout() {
  return(GetConfigState()->CurrentConfig.KeyMap);
}

int GetPaletteType() {
  return(GetConfigState()->CurrentConfig.PaletteType);
}

int GetRememberSize() {
  return((int)(GetConfigState()->CurrentConfig.RememberSize));
}

POINT GetIniWindowSize() {
  POINT out = POINT();

  ConfigState* configState = GetConfigState();

  out.x = configState->CurrentConfig.WindowSizeX;
  out.y = configState->CurrentConfig.WindowSizeY;

  return(out);
}
