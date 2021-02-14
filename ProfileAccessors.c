#include <windows.h>

#include "configstate.h"

void GetProfileText(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString) {
  ConfigState* configState = GetConfigState();

  GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpReturnedString, MAX_PATH, configState->IniFilePath);
}

void SetProfileText(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpString) {
  ConfigState* configState = GetConfigState();

  WritePrivateProfileString(lpAppName, lpKeyName, lpString, configState->IniFilePath);
}

unsigned short GetProfileShort(LPCSTR lpAppName, LPCSTR lpKeyName, int nDefault) {
  ConfigState* configState = GetConfigState();

  return GetPrivateProfileInt(lpAppName, lpKeyName, nDefault, configState->IniFilePath);
}

unsigned char GetProfileByte(LPCSTR lpAppName, LPCSTR lpKeyName, int nDefault) {
  ConfigState* configState = GetConfigState();

  return GetPrivateProfileInt(lpAppName, lpKeyName, nDefault, configState->IniFilePath);
}
