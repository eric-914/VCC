using namespace std;

#include <string>

#include "library/pakinterfacestate.h"

#include "library/ConfigAccessors.h"
#include "InsertModule.h"

#include "library/systemstate.h"

int LoadCart(SystemState* systemState)
{
  OPENFILENAME ofn;
  char szFileName[MAX_PATH] = "";
  char temp[MAX_PATH];

  PakInterfaceState* pakInterfaceState = GetPakInterfaceState();

  GetIniFilePath(temp);

  GetPrivateProfileString("DefaultPaths", "PakPath", "", pakInterfaceState->PakPath, MAX_PATH, temp);

  memset(&ofn, 0, sizeof(ofn));

  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = systemState->WindowHandle;
  ofn.lpstrFilter = "Program Packs\0*.ROM;*.ccc;*.DLL;*.pak\0\0";			// filter string
  ofn.nFilterIndex = 1;							          // current filter index
  ofn.lpstrFile = szFileName;				          // contains full path and filename on return
  ofn.nMaxFile = MAX_PATH;					          // sizeof lpstrFile
  ofn.lpstrFileTitle = NULL;						      // filename and extension only
  ofn.nMaxFileTitle = MAX_PATH;					      // sizeof lpstrFileTitle
  ofn.lpstrInitialDir = pakInterfaceState->PakPath;				      // initial directory
  ofn.lpstrTitle = TEXT("Load Program Pack");	// title bar string
  ofn.Flags = OFN_HIDEREADONLY;

  if (GetOpenFileName(&ofn)) {
    if (!InsertModule(systemState, szFileName)) {
      string tmp = ofn.lpstrFile;
      size_t idx = tmp.find_last_of("\\");
      tmp = tmp.substr(0, idx);

      strcpy(pakInterfaceState->PakPath, tmp.c_str());

      WritePrivateProfileString("DefaultPaths", "PakPath", pakInterfaceState->PakPath, temp);

      return(0);
    }
  }

  return(1);
}
