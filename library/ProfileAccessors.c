#include <windows.h>

#include "configstate.h"

extern "C" {
  __declspec(dllexport) void __cdecl GetProfileText(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString) {
    ConfigState* configState = GetConfigState();

    GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpReturnedString, MAX_PATH, configState->IniFilePath);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetProfileText(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpString) {
    ConfigState* configState = GetConfigState();

    WritePrivateProfileString(lpAppName, lpKeyName, lpString, configState->IniFilePath);
  }
}

extern "C" {
  __declspec(dllexport) unsigned short __cdecl GetProfileShort(LPCSTR lpAppName, LPCSTR lpKeyName, int nDefault) {
    ConfigState* configState = GetConfigState();

    return GetPrivateProfileInt(lpAppName, lpKeyName, nDefault, configState->IniFilePath);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl GetProfileByte(LPCSTR lpAppName, LPCSTR lpKeyName, int nDefault) {
    ConfigState* configState = GetConfigState();

    return GetPrivateProfileInt(lpAppName, lpKeyName, nDefault, configState->IniFilePath);
  }
}
