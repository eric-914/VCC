#include "configstate.h"

#include "SetAutoStart.h"
#include "SetCpuType.h"
#include "SetRamSize.h"
#include "SetCPUMultiplayer.h"
#include "SetFrameSkip.h"
#include "SetSpeedThrottle.h"
#include "SetMonitorType.h"
#include "SetPaletteType.h"
#include "SetScanLines.h"
#include "Reboot.h"

#include "library/systemstate.h"

extern unsigned char SetResize(unsigned char resizeable);
extern unsigned char SetAspect(unsigned char forceAspect);
extern unsigned char SetCartAutoStart(unsigned char autostart);

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
  SetCPUMultiplayer(configState->CurrentConfig.CPUMultiplyer);
  SetRamSize(configState->CurrentConfig.RamSize);
  SetCpuType(configState->CurrentConfig.CpuType);
  SetMonitorType(configState->CurrentConfig.MonitorType);
  SetCartAutoStart(configState->CurrentConfig.CartAutoStart);

  if (configState->CurrentConfig.RebootNow) {
    Reboot();
  }

  configState->CurrentConfig.RebootNow = 0;
}
