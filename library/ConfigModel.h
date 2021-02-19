#pragma once

#include <windows.h>

typedef struct {
  unsigned char	CPUMultiplyer;
  unsigned short MaxOverclock;
  unsigned char	FrameSkip;
  unsigned char	SpeedThrottle;
  unsigned char	CpuType;
  unsigned char	MonitorType;
  unsigned char PaletteType;
  unsigned char	ScanLines;
  unsigned char	Resize;
  unsigned char	Aspect;
  unsigned short RememberSize;
  unsigned short WindowSizeX;
  unsigned short WindowSizeY;
  unsigned char	RamSize;
  unsigned char	AutoStart;
  unsigned char	CartAutoStart;
  unsigned char	RebootNow;
  unsigned char	SndOutDev;
  unsigned char	KeyMap;

  char SoundCardName[64];
  unsigned short AudioRate;
  char ExternalBasicImage[MAX_PATH];
  char ModulePath[MAX_PATH];
  char PathtoExe[MAX_PATH];
  char FloppyPath[MAX_PATH];
  char CassPath[MAX_PATH];
  char COCO3ROMPath[MAX_PATH];
} ConfigModel;
