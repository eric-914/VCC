#include "library/vccstate.h"

#include "library/ConfigAccessors.h"
#include "WriteIniFile.h"

void SaveConfig(void) {
  OPENFILENAME ofn;
  char curini[MAX_PATH];
  char newini[MAX_PATH + 4];  // Save room for '.ini' if needed

  VccState* vccState = GetVccState();

  GetIniFilePath(curini);  // EJJ get current ini file path
  strcpy(newini, curini);   // Let GetOpenFilename suggest it

  memset(&ofn, 0, sizeof(ofn));

  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = vccState->EmuState.WindowHandle;
  ofn.lpstrFilter = "INI\0*.ini\0\0";      // filter string
  ofn.nFilterIndex = 1;                    // current filter index
  ofn.lpstrFile = newini;                  // contains full path on return
  ofn.nMaxFile = MAX_PATH;                 // sizeof lpstrFile
  ofn.lpstrFileTitle = NULL;               // filename and extension only
  ofn.nMaxFileTitle = MAX_PATH;            // sizeof lpstrFileTitle
  ofn.lpstrInitialDir = AppDirectory();    // EJJ initial directory
  ofn.lpstrTitle = TEXT("Save Vcc Config"); // title bar string
  ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

  if (GetOpenFileName(&ofn)) {
    if (ofn.nFileExtension == 0) {
      strcat(newini, ".ini");  //Add extension if none
    }

    WriteIniFile(); // Flush current config

    if (_stricmp(curini, newini) != 0) {
      if (!CopyFile(curini, newini, false)) { // Copy it to new file
        MessageBox(0, "Copy config failed", "error", 0);
      }
    }
  }
}
