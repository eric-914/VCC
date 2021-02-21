#include <string>

#include "library/Clipboard.h"
#include "library/Graphics.h"
#include "library/MMU.h"

#include "MemRead8.h"

void CopyText() {
  int idx;
  int tmp;
  int lines;
  int offset;
  int lastchar;
  string out;
  string tmpline;

  ClipboardState* clipboardState = GetClipboardState();
  GraphicsState* graphicsState = GetGraphicsState();

  int bytesPerRow = graphicsState->BytesperRow;
  int graphicsMode = graphicsState->GraphicsMode;
  unsigned int screenstart = graphicsState->StartofVidram;

  if (graphicsMode != 0) {
    MessageBox(0, "ERROR: Graphics screen can not be copied.\nCopy can ONLY use a hardware text screen.", "Clipboard", 0);

    return;
  }

  lines = bytesPerRow == 32 ? 15 : 23;

  string dbug = "StartofVidram is: " + to_string(screenstart) + "\nGraphicsMode is: " + to_string(graphicsMode) + "\n";

  OutputDebugString(dbug.c_str());

  // Read the lo-res text screen...
  if (bytesPerRow == 32) {
    offset = 0;

    char pcchars[] =
    {
      '@','a','b','c','d','e','f','g',
      'h','i','j','k','l','m','n','o',
      'p','q','r','s','t','u','v','w',
      'x','y','z','[','\\',']',' ',' ',
      ' ','!','\"','#','$','%','&','\'',
      '(',')','*','+',',','-','.','/',
      '0','1','2','3','4','5','6','7',
      '8','9',':',';','<','=','>','?',
      '@','A','B','C','D','E','F','G',
      'H','I','J','K','L','M','N','O',
      'P','Q','R','S','T','U','V','W',
      'X','Y','Z','[','\\',']',' ',' ',
      ' ','!','\"','#','$','%','&','\'',
      '(',')','*','+',',','-','.','/',
      '0','1','2','3','4','5','6','7',
      '8','9',':',';','<','=','>','?',
      '@','a','b','c','d','e','f','g',
      'h','i','j','k','l','m','n','o',
      'p','q','r','s','t','u','v','w',
      'x','y','z','[','\\',']',' ',' ',
      ' ','!','\"','#','$','%','&','\'',
      '(',')','*','+',',','-','.','/',
      '0','1','2','3','4','5','6','7',
      '8','9',':',';','<','=','>','?'
    };

    for (int y = 0; y <= lines; y++) {
      lastchar = 0;
      tmpline.clear();
      tmp = 0;

      for (idx = 0; idx < bytesPerRow; idx++) {
        tmp = MemRead8(0x0400 + y * bytesPerRow + idx);

        if (tmp == 32 || tmp == 64 || tmp == 96) {
          tmp = 30 + offset;
        }
        else {
          lastchar = idx + 1;
        }

        tmpline += pcchars[tmp - offset];
      }

      tmpline = tmpline.substr(0, lastchar);

      if (lastchar != 0) {
        out += tmpline; out += "\n";
      }
    }

    if (out == "") {
      MessageBox(0, "No text found on screen.", "Clipboard", 0);
    }
  }
  else if (bytesPerRow == 40 || bytesPerRow == 80) {
    offset = 32;
    char pcchars[] =
    {
      ' ','!','\"','#','$','%','&','\'',
      '(',')','*','+',',','-','.','/',
      '0','1','2','3','4','5','6','7',
      '8','9',':',';','<','=','>','?',
      '@','A','B','C','D','E','F','G',
      'H','I','J','K','L','M','N','O',
      'P','Q','R','S','T','U','V','W',
      'X','Y','Z','[','\\',']',' ',' ',
      '^','a','b','c','d','e','f','g',
      'h','i','j','k','l','m','n','o',
      'p','q','r','s','t','u','v','w',
      'x','y','z','{','|','}','~','_',
      (char)'Ç',(char)'ü',(char)'é',(char)'â',(char)'ä',(char)'à',(char)'å',(char)'ç',
      (char)'ê',(char)'ë',(char)'è',(char)'ï',(char)'î',(char)'ß',(char)'Ä',(char)'Â',
      (char)'Ó',(char)'æ',(char)'Æ',(char)'ô',(char)'ö',(char)'ø',(char)'û',(char)'ù',
      (char)'Ø',(char)'Ö',(char)'Ü',(char)'§',(char)'£',(char)'±',(char)'º',(char)'¥',
      ' ',' ','!','\"','#','$','%','&',
      '\'','(',')','*','+',',','-','.',
      '/','0','1','2','3','4','5','6',
      '7','8','9',':',';','<','=','>',
      '?','@','A','B','C','D','E','F',
      'G','H','I','J','K','L','M','N',
      'O','P','Q','R','S','T','U','V',
      'W','X','Y','Z','[','\\',']',' ',
      ' ','^','a','b','c','d','e','f',
      'g','h','i','j','k','l','m','n',
      'o','p','q','r','s','t','u','v',
      'w','x','y','z','{','|','}','~','_'
    };

    for (int y = 0; y <= lines; y++) {
      lastchar = 0;
      tmpline.clear();
      tmp = 0;

      for (idx = 0; idx < bytesPerRow * 2; idx += 2) {
        tmp = GetMem(screenstart + y * (bytesPerRow * 2) + idx);

        if (tmp == 32 || tmp == 64 || tmp == 96) {
          tmp = offset;
        }
        else {
          lastchar = idx / 2 + 1;
        }

        tmpline += pcchars[tmp - offset];
      }

      tmpline = tmpline.substr(0, lastchar);

      if (lastchar != 0) {
        out += tmpline; out += "\n";
      }
    }
  }

  bool succ = SetClipboard(out.c_str());
}
