#pragma once

#include <windows.h>

typedef struct
{
  HWND			WindowHandle;
  HWND			ConfigDialog;
  HINSTANCE		Resources;
  unsigned char* RamBuffer;
  unsigned short* WRamBuffer;
  unsigned char	RamSize;
  double			CPUCurrentSpeed;
  unsigned char	DoubleSpeedMultiplyer;
  unsigned char	DoubleSpeedFlag;
  unsigned char	TurboSpeedFlag;
  unsigned char	CpuType;
  unsigned char	FrameSkip;
  unsigned char	BitDepth;
  unsigned char* PTRsurface8;
  unsigned short* PTRsurface16;
  unsigned int* PTRsurface32;
  long			SurfacePitch;
  unsigned short	LineCounter;
  unsigned char	ScanLines;
  unsigned char	EmulationRunning;
  unsigned char	ResetPending;
  POINT			WindowSize;
  unsigned char	FullScreen;
  char			StatusLine[256];
} SystemState;
