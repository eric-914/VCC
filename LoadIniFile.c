#include "library/vccstate.h"
#include "library/Config.h"

#include "ReadIniFile.h"
#include "UpdateConfig.h"

// LoadIniFile allows user to browse for an ini file and reloads the config from it.
void LoadIniFile(void)
{
  OPENFILENAME ofn;
  char szFileName[MAX_PATH] = "";

  VccState* vccState = GetVccState();

  GetIniFilePath(szFileName); // EJJ load current ini file path

  memset(&ofn, 0, sizeof(ofn));

  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = vccState->EmuState.WindowHandle;
  ofn.lpstrFilter = "INI\0*.ini\0\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = szFileName;
  ofn.nMaxFile = MAX_PATH;
  ofn.nMaxFileTitle = MAX_PATH;
  ofn.lpstrFileTitle = NULL;
  ofn.lpstrInitialDir = AppDirectory();
  ofn.lpstrTitle = TEXT("Load Vcc Config File");
  ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;

  if (GetOpenFileName(&ofn)) {
    WriteIniFile();               // Flush current profile
    SetIniFilePath(szFileName);   // Set new ini file path
    ReadIniFile(&(vccState->EmuState));                // Load it
    UpdateConfig(&(vccState->EmuState));

    vccState->EmuState.ResetPending = 2;
  }
}
