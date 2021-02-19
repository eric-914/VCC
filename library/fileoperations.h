#pragma once

#include <windows.h>

extern "C" __declspec(dllexport) void __cdecl FilePathStripPath(char* path);
extern "C" __declspec(dllexport) void __cdecl FileValidatePath(char* path);
extern "C" __declspec(dllexport) int __cdecl FileCheckPath(char* path);
extern "C" __declspec(dllexport) BOOL __cdecl FilePathRemoveFileSpec(char* path);
extern "C" __declspec(dllexport) BOOL __cdecl FilePathRemoveExtension(char* path);
extern "C" __declspec(dllexport) char* __cdecl FilePathFindExtension(char* path);
extern "C" __declspec(dllexport) DWORD __cdecl FileWritePrivateProfileInt(LPCTSTR sectionName, LPCTSTR keyName, int keyValue, LPCTSTR iniFileName);
