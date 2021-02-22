#include "library/Config.h"
#include "library/DirectDraw.h"
#include "library/MC6821.h"
#include "library/systemstate.h"
#include "library/VCC.h"
#include "library/Graphics.h"

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
