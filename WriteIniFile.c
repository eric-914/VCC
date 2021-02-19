#include "configstate.h"

#include "library/PakInterfaceAccessors.h"
#include "library/DirectDrawAccessors.h"

#include "library/joystickstate.h"
#include "library/fileoperations.h"

unsigned char WriteIniFile(void)
{
  ConfigState* configState = GetConfigState();

  POINT tp = GetCurrentWindowSize();
  configState->CurrentConfig.Resize = 1;

  GetCurrentModule(configState->CurrentConfig.ModulePath);
  FileValidatePath(configState->CurrentConfig.ModulePath);
  FileValidatePath(configState->CurrentConfig.ExternalBasicImage);

  WritePrivateProfileString("Version", "Release", configState->AppName, configState->IniFilePath);

  FileWritePrivateProfileInt("CPU", "DoubleSpeedClock", configState->CurrentConfig.CPUMultiplyer, configState->IniFilePath);
  FileWritePrivateProfileInt("CPU", "FrameSkip", configState->CurrentConfig.FrameSkip, configState->IniFilePath);
  FileWritePrivateProfileInt("CPU", "Throttle", configState->CurrentConfig.SpeedThrottle, configState->IniFilePath);
  FileWritePrivateProfileInt("CPU", "CpuType", configState->CurrentConfig.CpuType, configState->IniFilePath);
  FileWritePrivateProfileInt("CPU", "MaxOverClock", configState->CurrentConfig.MaxOverclock, configState->IniFilePath);

  WritePrivateProfileString("Audio", "SndCard", configState->CurrentConfig.SoundCardName, configState->IniFilePath);
  FileWritePrivateProfileInt("Audio", "Rate", configState->CurrentConfig.AudioRate, configState->IniFilePath);

  FileWritePrivateProfileInt("Video", "MonitorType", configState->CurrentConfig.MonitorType, configState->IniFilePath);
  FileWritePrivateProfileInt("Video", "PaletteType", configState->CurrentConfig.PaletteType, configState->IniFilePath);
  FileWritePrivateProfileInt("Video", "ScanLines", configState->CurrentConfig.ScanLines, configState->IniFilePath);
  FileWritePrivateProfileInt("Video", "AllowResize", configState->CurrentConfig.Resize, configState->IniFilePath);
  FileWritePrivateProfileInt("Video", "ForceAspect", configState->CurrentConfig.Aspect, configState->IniFilePath);
  FileWritePrivateProfileInt("Video", "RememberSize", configState->CurrentConfig.RememberSize, configState->IniFilePath);
  FileWritePrivateProfileInt("Video", "WindowSizeX", tp.x, configState->IniFilePath);
  FileWritePrivateProfileInt("Video", "WindowSizeY", tp.y, configState->IniFilePath);

  FileWritePrivateProfileInt("Memory", "RamSize", configState->CurrentConfig.RamSize, configState->IniFilePath);

  WritePrivateProfileString("Memory", "ExternalBasicImage", configState->CurrentConfig.ExternalBasicImage, configState->IniFilePath);

  FileWritePrivateProfileInt("Misc", "AutoStart", configState->CurrentConfig.AutoStart, configState->IniFilePath);
  FileWritePrivateProfileInt("Misc", "CartAutoStart", configState->CurrentConfig.CartAutoStart, configState->IniFilePath);
  FileWritePrivateProfileInt("Misc", "KeyMapIndex", configState->CurrentConfig.KeyMap, configState->IniFilePath);

  WritePrivateProfileString("Module", "OnBoot", configState->CurrentConfig.ModulePath, configState->IniFilePath);

  JoystickState* joystickState = GetJoystickState();

  FileWritePrivateProfileInt("LeftJoyStick", "UseMouse", joystickState->Left.UseMouse, configState->IniFilePath);
  FileWritePrivateProfileInt("LeftJoyStick", "Left", joystickState->Left.Left, configState->IniFilePath);
  FileWritePrivateProfileInt("LeftJoyStick", "Right", joystickState->Left.Right, configState->IniFilePath);
  FileWritePrivateProfileInt("LeftJoyStick", "Up", joystickState->Left.Up, configState->IniFilePath);
  FileWritePrivateProfileInt("LeftJoyStick", "Down", joystickState->Left.Down, configState->IniFilePath);
  FileWritePrivateProfileInt("LeftJoyStick", "Fire1", joystickState->Left.Fire1, configState->IniFilePath);
  FileWritePrivateProfileInt("LeftJoyStick", "Fire2", joystickState->Left.Fire2, configState->IniFilePath);
  FileWritePrivateProfileInt("LeftJoyStick", "DiDevice", joystickState->Left.DiDevice, configState->IniFilePath);
  FileWritePrivateProfileInt("LeftJoyStick", "HiResDevice", joystickState->Left.HiRes, configState->IniFilePath);
  FileWritePrivateProfileInt("RightJoyStick", "UseMouse", joystickState->Right.UseMouse, configState->IniFilePath);
  FileWritePrivateProfileInt("RightJoyStick", "Left", joystickState->Right.Left, configState->IniFilePath);
  FileWritePrivateProfileInt("RightJoyStick", "Right", joystickState->Right.Right, configState->IniFilePath);
  FileWritePrivateProfileInt("RightJoyStick", "Up", joystickState->Right.Up, configState->IniFilePath);
  FileWritePrivateProfileInt("RightJoyStick", "Down", joystickState->Right.Down, configState->IniFilePath);
  FileWritePrivateProfileInt("RightJoyStick", "Fire1", joystickState->Right.Fire1, configState->IniFilePath);
  FileWritePrivateProfileInt("RightJoyStick", "Fire2", joystickState->Right.Fire2, configState->IniFilePath);
  FileWritePrivateProfileInt("RightJoyStick", "DiDevice", joystickState->Right.DiDevice, configState->IniFilePath);
  FileWritePrivateProfileInt("RightJoyStick", "HiResDevice", joystickState->Right.HiRes, configState->IniFilePath);

  //  Flush inifile
  WritePrivateProfileString(NULL, NULL, NULL, configState->IniFilePath);

  return(0);
}
