#include <windows.h>

#include "configstate.h"

#include "library/joystickstate.h"
#include "library/keyboarddef.h"

#include "SetWindowSize.h"
#include "ProfileAccessors.h"

extern int InsertModule(char* modulePath);

extern "C" void vccKeyboardBuildRuntimeTable(keyboardlayout_e keyBoardLayout);
extern "C" __declspec(dllexport) int __cdecl FileCheckPath(char* path);

unsigned char ReadIniFile(void)
{
  HANDLE hr = NULL;
  POINT p = POINT();
  unsigned char index = 0;

  ConfigState* configState = GetConfigState();

  //Loads the config structure from the hard disk
  configState->CurrentConfig.CPUMultiplyer = GetProfileByte("CPU", "DoubleSpeedClock", 2);
  configState->CurrentConfig.FrameSkip = GetProfileByte("CPU", "FrameSkip", 1);
  configState->CurrentConfig.SpeedThrottle = GetProfileByte("CPU", "Throttle", 1);
  configState->CurrentConfig.CpuType = GetProfileByte("CPU", "CpuType", 0);
  configState->CurrentConfig.MaxOverclock = GetProfileShort("CPU", "MaxOverClock", 227);

  configState->CurrentConfig.AudioRate = GetProfileShort("Audio", "Rate", 3);

  GetPrivateProfileString("Audio", "SndCard", "", configState->CurrentConfig.SoundCardName, 63, configState->IniFilePath);

  configState->CurrentConfig.MonitorType = GetProfileByte("Video", "MonitorType", 1);
  configState->CurrentConfig.PaletteType = GetProfileByte("Video", "PaletteType", 1);
  configState->CurrentConfig.ScanLines = GetProfileByte("Video", "ScanLines", 0);

  configState->CurrentConfig.Resize = GetProfileByte("Video", "AllowResize", 0);
  configState->CurrentConfig.Aspect = GetProfileByte("Video", "ForceAspect", 0);
  configState->CurrentConfig.RememberSize = GetProfileShort("Video", "RememberSize", 0);
  configState->CurrentConfig.WindowSizeX = GetProfileShort("Video", "WindowSizeX", 640);
  configState->CurrentConfig.WindowSizeY = GetProfileShort("Video", "WindowSizeY", 480);
  configState->CurrentConfig.AutoStart = GetProfileByte("Misc", "AutoStart", 1);
  configState->CurrentConfig.CartAutoStart = GetProfileByte("Misc", "CartAutoStart", 1);

  configState->CurrentConfig.RamSize = GetProfileByte("Memory", "RamSize", 1);

  GetProfileText("Memory", "ExternalBasicImage", "", configState->CurrentConfig.ExternalBasicImage);

  GetProfileText("Module", "OnBoot", "", configState->CurrentConfig.ModulePath);

  configState->CurrentConfig.KeyMap = GetProfileByte("Misc", "KeyMapIndex", 0);

  if (configState->CurrentConfig.KeyMap > 3) {
    configState->CurrentConfig.KeyMap = 0;	//Default to DECB Mapping
  }

  vccKeyboardBuildRuntimeTable((keyboardlayout_e)(configState->CurrentConfig.KeyMap));

  FileCheckPath(configState->CurrentConfig.ModulePath);
  FileCheckPath(configState->CurrentConfig.ExternalBasicImage);

  JoystickState* joystickState = GetJoystickState();

  joystickState->Left.UseMouse = GetProfileByte("LeftJoyStick", "UseMouse", 1);
  joystickState->Left.Left = GetProfileByte("LeftJoyStick", "Left", 75);
  joystickState->Left.Right = GetProfileByte("LeftJoyStick", "Right", 77);
  joystickState->Left.Up = GetProfileByte("LeftJoyStick", "Up", 72);
  joystickState->Left.Down = GetProfileByte("LeftJoyStick", "Down", 80);
  joystickState->Left.Fire1 = GetProfileByte("LeftJoyStick", "Fire1", 59);
  joystickState->Left.Fire2 = GetProfileByte("LeftJoyStick", "Fire2", 60);
  joystickState->Left.DiDevice = GetProfileByte("LeftJoyStick", "DiDevice", 0);
  joystickState->Left.HiRes = GetProfileByte("LeftJoyStick", "HiResDevice", 0);
  joystickState->Right.UseMouse = GetProfileByte("RightJoyStick", "UseMouse", 1);
  joystickState->Right.Left = GetProfileByte("RightJoyStick", "Left", 75);
  joystickState->Right.Right = GetProfileByte("RightJoyStick", "Right", 77);
  joystickState->Right.Up = GetProfileByte("RightJoyStick", "Up", 72);
  joystickState->Right.Down = GetProfileByte("RightJoyStick", "Down", 80);
  joystickState->Right.Fire1 = GetProfileByte("RightJoyStick", "Fire1", 59);
  joystickState->Right.Fire2 = GetProfileByte("RightJoyStick", "Fire2", 60);
  joystickState->Right.DiDevice = GetProfileByte("RightJoyStick", "DiDevice", 0);
  joystickState->Right.HiRes = GetProfileByte("RightJoyStick", "HiResDevice", 0);

  GetProfileText("DefaultPaths", "CassPath", "", configState->CurrentConfig.CassPath);
  GetProfileText("DefaultPaths", "FloppyPath", "", configState->CurrentConfig.FloppyPath);
  GetProfileText("DefaultPaths", "COCO3ROMPath", "", configState->CurrentConfig.COCO3ROMPath);

  for (index = 0; index < configState->NumberOfSoundCards; index++) {
    if (!strcmp(configState->SoundCards[index].CardName, configState->CurrentConfig.SoundCardName)) {
      configState->CurrentConfig.SndOutDev = index;
    }
  }

  configState->TempConfig = configState->CurrentConfig;

  InsertModule(configState->CurrentConfig.ModulePath);	// Should this be here?

  configState->CurrentConfig.Resize = 1; //Checkbox removed. Remove this from the ini? 

  if (configState->CurrentConfig.RememberSize) {
    p.x = configState->CurrentConfig.WindowSizeX;
    p.y = configState->CurrentConfig.WindowSizeY;

    SetWindowSize(p);
  }
  else {
    p.x = 640;
    p.y = 480;

    SetWindowSize(p);
  }

  return(0);
}
