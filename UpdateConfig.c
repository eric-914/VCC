#include "configstate.h"

#include "library/systemstate.h"

extern void SetPaletteType();

extern unsigned char SetResize(unsigned char resizeable);
extern unsigned char SetAspect(unsigned char forceAspect);
extern unsigned char SetScanLines(SystemState* systemState, unsigned char lines);
extern unsigned char SetFrameSkip(unsigned char skip);
extern unsigned char SetAutoStart(unsigned char autostart);
extern unsigned char SetSpeedThrottle(unsigned char throttle);
extern unsigned char SetCPUMultiplyer(unsigned char multiplyer);
extern unsigned char SetRamSize(unsigned char size);
extern unsigned char SetCpuType(unsigned char cpuType);
extern unsigned char SetMonitorType(unsigned char type);
extern unsigned char SetCartAutoStart(unsigned char autostart);

extern void DoReboot(void);

void UpdateConfig(SystemState* systemState)
{
  ConfigState* configState = GetConfigState();

  SetPaletteType();
  SetResize(configState->CurrentConfig.Resize);
  SetAspect(configState->CurrentConfig.Aspect);
  SetScanLines(systemState, configState->CurrentConfig.ScanLines);
  SetFrameSkip(configState->CurrentConfig.FrameSkip);
  SetAutoStart(configState->CurrentConfig.AutoStart);
  SetSpeedThrottle(configState->CurrentConfig.SpeedThrottle);
  SetCPUMultiplyer(configState->CurrentConfig.CPUMultiplyer);
  SetRamSize(configState->CurrentConfig.RamSize);
  SetCpuType(configState->CurrentConfig.CpuType);
  SetMonitorType(configState->CurrentConfig.MonitorType);
  SetCartAutoStart(configState->CurrentConfig.CartAutoStart);

  if (configState->CurrentConfig.RebootNow) {
    DoReboot();
  }

  configState->CurrentConfig.RebootNow = 0;
}
