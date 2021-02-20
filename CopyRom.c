#include <windows.h>

#include "LoadInternalRom.h"
#include "library/Config.h"

#include "library/fileoperations.h"

void CopyRom(void)
{
  char ExecPath[MAX_PATH];
  char COCO3ROMPath[MAX_PATH];
  unsigned short temp = 0;

  GetProfileText("DefaultPaths", "COCO3ROMPath", "", COCO3ROMPath);

  strcat(COCO3ROMPath, "\\coco3.rom");

  if (COCO3ROMPath != "") {
    temp = LoadInternalRom(COCO3ROMPath);  //Try loading from the user defined path first.
  }

  if (temp) {
    OutputDebugString(" Found coco3.rom in COCO3ROMPath\n");
  }

  if (temp == 0) {
    temp = LoadInternalRom(BasicRomName());  //Try to load the image
  }

  if (temp == 0) {
    // If we can't find it use default copy
    GetModuleFileName(NULL, ExecPath, MAX_PATH);

    FilePathRemoveFileSpec(ExecPath);

    strcat(ExecPath, "coco3.rom");

    temp = LoadInternalRom(ExecPath);
  }

  if (temp == 0)
  {
    MessageBox(0, "Missing file coco3.rom", "Error", 0);

    exit(0);
  }
}
