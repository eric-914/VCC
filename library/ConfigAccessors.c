#include "configstate.h"

extern "C" {
  __declspec(dllexport) char* __cdecl BasicRomName(void)
  {
    return(GetConfigState()->CurrentConfig.ExternalBasicImage);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl GetIniFilePath(char* path)
  {
    strcpy(path, GetConfigState()->IniFilePath);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetIniFilePath(char* path)
  {
    //  Path must be to an existing ini file
    strcpy(GetConfigState()->IniFilePath, path);
  }
}

extern "C" {
  __declspec(dllexport) char* __cdecl AppDirectory()
  {
    // This only works after LoadConfig has been called
    return GetConfigState()->AppDataPath;
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl GetCurrentKeyboardLayout() {
    return(GetConfigState()->CurrentConfig.KeyMap);
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl GetPaletteType() {
    return(GetConfigState()->CurrentConfig.PaletteType);
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl GetRememberSize() {
    return((int)(GetConfigState()->CurrentConfig.RememberSize));
  }
}

extern "C" {
  __declspec(dllexport) POINT __cdecl GetIniWindowSize() {
    POINT out = POINT();

    ConfigState* configState = GetConfigState();

    out.x = configState->CurrentConfig.WindowSizeX;
    out.y = configState->CurrentConfig.WindowSizeY;

    return(out);
  }
}