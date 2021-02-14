#pragma once

#include <windows.h>

void GetProfileText(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString);
void SetProfileText(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpString);

unsigned short GetProfileShort(LPCSTR lpAppName, LPCSTR lpKeyName, int nDefault);
unsigned char GetProfileByte(LPCSTR lpAppName, LPCSTR lpKeyName, int nDefault);