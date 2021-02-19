#pragma once

/*
  Debug trace macros
*/
#if (defined _DEBUG)
#	define XTRACE(x,...) _xDbgTrace( __FILE__, __LINE__, x, ##__VA_ARGS__ )
#else
#	define XTRACE(x,...)
#endif

extern "C"
{
  __declspec(dllexport) void __cdecl _xDbgTrace(const void* pFile, const int iLine, const void* pFormat, ...);
}
