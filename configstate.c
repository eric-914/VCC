#include "configstate.h"
#include "resources/resource.h"

#include "library/cassettedef.h"

#define SIZEOF(x)  (sizeof(x) / sizeof((x)[0]))

const unsigned short int Cpuchoice[2] = { IDC_6809, IDC_6309 };
const unsigned short int Monchoice[2] = { IDC_COMPOSITE, IDC_RGB };
const unsigned short int PaletteChoice[2] = { IDC_ORG_PALETTE, IDC_UPD_PALETTE };
const unsigned short int Ramchoice[4] = { IDC_128K, IDC_512K, IDC_2M, IDC_8M };
const unsigned int LeftJoystickEmulation[3] = { IDC_LEFTSTANDARD, IDC_LEFTTHIRES, IDC_LEFTCCMAX };
const unsigned int RightJoystickEmulation[3] = { IDC_RIGHTSTANDARD, IDC_RIGHTTHRES, IDC_RIGHTCCMAX };
const char TextMode = 1;
const char PrtMon = 0;
const unsigned char NumberofJoysticks = 0;
const unsigned int TapeCounter = 0;
const unsigned char Tmode = STOP;
const char Tmodes[4][10] = { "STOP","PLAY","REC","STOP" };
const int NumberOfSoundCards = 0;
const unsigned char TranslateScan2Disp[SCAN_TRANS_COUNT] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,32,38,20,33,35,40,36,24,30,31,42,43,55,52,16,34,19,21,22,23,25,26,27,45,46,0,51,44,41,39,18,37,17,29,28,47,48,49,51,0,53,54,50,66,67,0,0,0,0,0,0,0,0,0,0,58,64,60,0,62,0,63,0,59,65,61,56,57 };

ConfigState* InitializeInstance(ConfigState* coco);

static ConfigState* instance = InitializeInstance(new ConfigState());

extern "C" {
  __declspec(dllexport) ConfigState* __cdecl GetConfigState() {
    return instance;
  }
}

ConfigState* InitializeInstance(ConfigState* c) {
  c->hDlgBar = NULL;
  c->hDlgTape = NULL;

  for (int i = 0; i < SIZEOF(Cpuchoice); i++) {
    c->Cpuchoice[i] = Cpuchoice[i];
  }

  for (int i = 0; i < SIZEOF(Monchoice); i++) {
    c->Monchoice[i] = Monchoice[i];
  }

  for (int i = 0; i < SIZEOF(PaletteChoice); i++) {
    c->PaletteChoice[i] = PaletteChoice[i];
  }

  for (int i = 0; i < SIZEOF(Ramchoice); i++) {
    c->Ramchoice[i] = Ramchoice[i];
  }

  for (int i = 0; i < SIZEOF(LeftJoystickEmulation); i++) {
    c->LeftJoystickEmulation[i] = LeftJoystickEmulation[i];
  }

  for (int i = 0; i < SIZEOF(RightJoystickEmulation); i++) {
    c->RightJoystickEmulation[i] = RightJoystickEmulation[i];
  }

  for (int i = 0; i < SIZEOF(Tmodes); i++) {
    strcpy(c->Tmodes[i], Tmodes[i]);
  }

  for (int i = 0; i < SIZEOF(TranslateScan2Disp); i++) {
    c->TranslateScan2Disp[i] = TranslateScan2Disp[i];
  }

  c->TextMode = TextMode;
  c->PrtMon = PrtMon;
  c->NumberofJoysticks = NumberofJoysticks;

  strcpy(c->IniFilePath, "");
  strcpy(c->TapeFileName, "");
  strcpy(c->ExecDirectory, "");
  strcpy(c->SerialCaptureFile, "");
  strcpy(c->OutBuffer, "");
  strcpy(c->AppName, "");

  c->TapeCounter = TapeCounter;
  c->Tmode = Tmode;
  c->NumberOfSoundCards = NumberOfSoundCards;

  return c;
}