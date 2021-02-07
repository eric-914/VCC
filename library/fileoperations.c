#include <windows.h>

#include "..\fileops\fileops.h"

extern "C" {
  __declspec(dllexport) void __cdecl FilePathStripPath(char* path) {
    PathStripPath(path);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl FileValidatePath(char* path) {
    ValidatePath(path);
  }
}

extern "C" {
  __declspec(dllexport) int __cdecl FileCheckPath(char* path) {
    return CheckPath(path);
  }
}

extern "C" {
  __declspec(dllexport) BOOL __cdecl FilePathRemoveFileSpec(char* path) {
    return PathRemoveFileSpec(path);
  }
}

extern "C" {
  __declspec(dllexport) BOOL __cdecl FilePathRemoveExtension(char* path) {
    return PathRemoveExtension(path);
  }
}

extern "C" {
  __declspec(dllexport) char* __cdecl FilePathFindExtension(char* path) {
    return PathFindExtension(path);
  }
}

extern "C" {
  __declspec(dllexport) DWORD __cdecl FileWritePrivateProfileInt(LPCTSTR sectionName, LPCTSTR keyName, int keyValue, LPCTSTR iniFileName) {
    return WritePrivateProfileInt(sectionName, keyName, keyValue, iniFileName);
  }
}