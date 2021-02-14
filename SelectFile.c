#include "configstate.h"

#include "library/systemstate.h"

extern void GetProfileText(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString);
extern int OpenPrintFile(char* FileName);

int SelectFile(SystemState* systemState, char* filename)
{
  OPENFILENAME ofn;
  char dummy[MAX_PATH] = "";
  char tempFileName[MAX_PATH] = "";
  char capFilePath[MAX_PATH];

  ConfigState* configState = GetConfigState();

  GetProfileText("DefaultPaths", "CapFilePath", "", capFilePath);

  memset(&ofn, 0, sizeof(ofn));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = systemState->WindowHandle; // GetTopWindow(NULL);
  ofn.Flags = OFN_HIDEREADONLY;
  ofn.hInstance = GetModuleHandle(0);
  ofn.lpstrDefExt = "txt";
  ofn.lpstrFilter = "Text File\0*.txt\0\0";
  ofn.nFilterIndex = 0;					      // current filter index
  ofn.lpstrFile = tempFileName;		    // contains full path and filename on return
  ofn.nMaxFile = MAX_PATH;			      // sizeof lpstrFile
  ofn.lpstrFileTitle = NULL;				  // filename and extension only
  ofn.nMaxFileTitle = MAX_PATH;			  // sizeof lpstrFileTitle
  ofn.lpstrInitialDir = capFilePath;  // initial directory
  ofn.lpstrTitle = "Open print capture file";		// title bar string

  if (GetOpenFileName(&ofn)) {
    if (!(OpenPrintFile(tempFileName))) {
      MessageBox(0, "Can't Open File", "Can't open the file specified.", 0);
    }

    std::string tmp = ofn.lpstrFile;
    size_t idx = tmp.find_last_of("\\");

    tmp = tmp.substr(0, idx);

    strcpy(capFilePath, tmp.c_str());

    if (capFilePath != "") {
      WritePrivateProfileString("DefaultPaths", "CapFilePath", capFilePath, configState->IniFilePath);
    }
  }

  strcpy(filename, tempFileName);

  return(1);
}
