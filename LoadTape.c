#include <windows.h>
#include <iostream>

#include "library/cassettestate.h"

//--CASSETTE--//

extern void GetProfileText(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString);
extern int MountTape(char* filename);

unsigned int LoadTape(void)
{
  static unsigned char DialogOpen = 0;
  unsigned int RetVal = 0;

  HANDLE hr = NULL;
  OPENFILENAME ofn;

  CassetteState* cassetteState = GetCassetteState();

  GetProfileText("DefaultPaths", "CassPath", "", cassetteState->CassPath);

  if (DialogOpen == 1) {	//Only allow 1 dialog open 
    return(0);
  }

  DialogOpen = 1;
  memset(&ofn, 0, sizeof(ofn));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = NULL;
  ofn.Flags = OFN_HIDEREADONLY;
  ofn.hInstance = GetModuleHandle(0);
  ofn.lpstrDefExt = "";
  ofn.lpstrFilter = "Cassette Files (*.cas)\0*.cas\0Wave Files (*.wav)\0*.wav\0\0";
  ofn.nFilterIndex = 0;								  // current filter index
  ofn.lpstrFile = cassetteState->TapeFileName;					// contains full path and filename on return
  ofn.nMaxFile = MAX_PATH;						  // sizeof lpstrFile
  ofn.lpstrFileTitle = NULL;						// filename and extension only
  ofn.nMaxFileTitle = MAX_PATH;					// sizeof lpstrFileTitle
  ofn.lpstrInitialDir = cassetteState->CassPath;				// initial directory
  ofn.lpstrTitle = "Insert Tape Image";	// title bar string

  RetVal = GetOpenFileName(&ofn);

  if (RetVal)
  {
    if (MountTape(cassetteState->TapeFileName) == 0) {
      MessageBox(NULL, "Can't open file", "Error", 0);
    }
  }

  DialogOpen = 0;
  std::string tmp = ofn.lpstrFile;
  size_t idx;
  idx = tmp.find_last_of("\\");
  tmp = tmp.substr(0, idx);
  strcpy(cassetteState->CassPath, tmp.c_str());

  if (cassetteState->CassPath != "") {
    WriteProfileString("DefaultPaths", "CassPath", cassetteState->CassPath);
  }

  return(RetVal);
}
