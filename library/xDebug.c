/**
  xDebug.c

  uEng - micro cross platform engine
  Copyright 2010 by Wes Gale All rights reserved.
  Used by permission in VccX
*/

#include "xDebug.h"

#include <stdio.h>
#include <Windows.h>

int	g_bTraceMessageLogging = TRUE;

extern "C" {
  __declspec(dllexport) void __cdecl _xDbgTrace(const void* pFile, const int iLine, const void* pFormat, ...)
  {
    va_list	args;
    char temp[1024];

    va_start(args, pFormat);

    sprintf(temp, "%s(%d) : ", (char*)pFile, iLine);
    OutputDebugString(temp);

    vsprintf(temp, (char*)pFormat, args);
    OutputDebugString(temp);

    va_end(args);
  }
}