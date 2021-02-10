/*
Copyright 2015 by Joseph Forgione
This file is part of VCC (Virtual Color Computer).

    VCC (Virtual Color Computer) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    VCC (Virtual Color Computer) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with VCC (Virtual Color Computer).  If not, see <http://www.gnu.org/licenses/>.
*/

#include <windows.h>

#include "tcc1014graphics.h"
#include "coco3.h"
#include "cc2font.h"
#include "cc3font.h"
#include "config.h"
#include "DirectDrawInterface.h"

#include "library/defines.h"

void SetupDisplay(void); //This routine gets called every time a software video register get updated.
void MakeRGBPalette(void);
void MakeCMPpalette(void);
void SetBlinkState(unsigned char blinkState);
unsigned char CheckState(unsigned char attributes);

static unsigned char  ColorValues[4] = { 0,85,170,255 };
static unsigned char  ColorTable16Bit[4] = { 0,10,21,31 };	//Color brightness at 0 1 2 and 3 (2 bits)
static unsigned char  ColorTable32Bit[4] = { 0,85,170,255 };
static unsigned short Afacts16[2][4] = { 0,0xF800,0x001F,0xFFFF,0,0x001F,0xF800,0xFFFF };
static unsigned char  Afacts8[2][4] = { 0,0xA4,0x89,0xBF,0,137,164,191 };
static unsigned int   Afacts32[2][4] = { 0,0xFF8D1F,0x0667FF,0xFFFFFF,0,0x0667FF,0xFF8D1F,0xFFFFFF };
static unsigned char  Pallete[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };		//Coco 3 6 bit colors
static unsigned char  Pallete8Bit[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
static unsigned short Pallete16Bit[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };	//Color values translated to 16bit 32BIT
static unsigned int   Pallete32Bit[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };	//Color values translated to 24/32 bits

static unsigned char  PalleteLookup8[2][64];	//0 = RGB 1=comp 8BIT
static unsigned short PalleteLookup16[2][64];	//0 = RGB 1=comp 16BIT
static unsigned int   PalleteLookup32[2][64];	//0 = RGB 1=comp 32BIT

static unsigned char  Lpf[4] = { 192, 199, 225, 225 }; // #2 is really undefined but I gotta put something here.
static unsigned char  VcenterTable[4] = { 29, 23, 12, 12 };

struct GraphicsState
{
  unsigned char BlinkState = 1;
  unsigned char BoarderChange = 3;
  unsigned char Bpp = 0;
  unsigned char BytesperRow = 32;
  unsigned char CC2Offset = 0;
  unsigned char CC2VDGMode = 0;
  unsigned char CC2VDGPiaMode = 0;
  unsigned char CC3BoarderColor = 0;
  unsigned char CC3Vmode = 0;
  unsigned char CC3Vres = 0;
  unsigned char ColorInvert = 1;
  unsigned char CompatMode = 0;
  unsigned char ExtendedText = 1;
  unsigned char GraphicsMode = 0;
  unsigned char Hoffset = 0;
  unsigned char HorzCenter = 0;
  unsigned char HorzOffsetReg = 0;
  unsigned char InvertAll = 0;
  unsigned char LinesperRow = 1;
  unsigned char LinesperScreen = 0;
  unsigned char LowerCase = 0;
  unsigned char MasterMode = 0;
  unsigned char MonType = 1;
  unsigned char PalleteIndex = 0;
  unsigned char Stretch = 0;
  unsigned char TextBGColor = 0;
  unsigned char TextBGPallete = 0;
  unsigned char TextFGColor = 0;
  unsigned char TextFGPallete = 0;
  unsigned char VertCenter = 0;
  unsigned char VresIndex = 0;

  unsigned short PixelsperLine = 0;
  unsigned short TagY = 0;
  unsigned short VerticalOffsetRegister = 0;
  unsigned short VPitch = 32;

  unsigned int DistoOffset = 0;
  unsigned int NewStartofVidram = 0;
  unsigned int StartofVidram = 0;
  unsigned int VidMask = 0x1FFFF;

  unsigned int   BoarderColor32 = 0;
  unsigned short BoarderColor16 = 0;
  unsigned char  BoarderColor8 = 0;
};

static GraphicsState graphicsState;

void UpdateScreen8(SystemState* systemState)
{
  register unsigned int yStride = 0;
  unsigned char pixel = 0;
  unsigned char character = 0, attributes = 0;
  unsigned char TextPallete[2] = { 0,0 };
  unsigned short WidePixel = 0;
  char pix = 0, bit = 0, phase = 0;
  static char carry1 = 0, carry2 = 0;
  static char color = 0;
  unsigned char* buffer = systemState->RamBuffer;

  carry1 = 1;
  color = 0;

  if ((graphicsState.HorzCenter != 0) && (graphicsState.BoarderChange > 0))
    for (unsigned short x = 0; x < graphicsState.HorzCenter; x++)
    {
      systemState->PTRsurface8[x + (((systemState->LineCounter + graphicsState.VertCenter) * 2) * systemState->SurfacePitch)] = graphicsState.BoarderColor8;

      if (!systemState->ScanLines)
        systemState->PTRsurface8[x + (((systemState->LineCounter + graphicsState.VertCenter) * 2 + 1) * systemState->SurfacePitch)] = graphicsState.BoarderColor8;

      systemState->PTRsurface8[x + (graphicsState.PixelsperLine * (graphicsState.Stretch + 1)) + graphicsState.HorzCenter + (((systemState->LineCounter + graphicsState.VertCenter) * 2) * systemState->SurfacePitch)] = graphicsState.BoarderColor8;

      if (!systemState->ScanLines)
        systemState->PTRsurface8[x + (graphicsState.PixelsperLine * (graphicsState.Stretch + 1)) + graphicsState.HorzCenter + (((systemState->LineCounter + graphicsState.VertCenter) * 2 + 1) * systemState->SurfacePitch)] = graphicsState.BoarderColor8;
    }

  if (graphicsState.LinesperRow < 13) {
    graphicsState.TagY++;
  }

  if (!systemState->LineCounter)
  {
    graphicsState.StartofVidram = graphicsState.NewStartofVidram;
    graphicsState.TagY = systemState->LineCounter;
  }

  unsigned int start = graphicsState.StartofVidram + (graphicsState.TagY / graphicsState.LinesperRow) * (graphicsState.VPitch * graphicsState.ExtendedText);
  yStride = (((systemState->LineCounter + graphicsState.VertCenter) * 2) * systemState->SurfacePitch) + (graphicsState.HorzCenter)-1;

  switch (graphicsState.MasterMode) // (GraphicsMode <<7) | (CompatMode<<6)  | ((Bpp & 3)<<4) | (Stretch & 15);
  {
  case 0: //Width 80
    attributes = 0;

    for (unsigned short beam = 0; beam < graphicsState.BytesperRow * graphicsState.ExtendedText; beam += graphicsState.ExtendedText)
    {
      character = buffer[start + (unsigned char)(beam + graphicsState.Hoffset)];
      pixel = cc3Fontdata8x12[character * 12 + (systemState->LineCounter % graphicsState.LinesperRow)];

      if (graphicsState.ExtendedText == 2)
      {
        attributes = buffer[start + (unsigned char)(beam + graphicsState.Hoffset) + 1];

        if ((attributes & 64) && (systemState->LineCounter % graphicsState.LinesperRow == (graphicsState.LinesperRow - 1))) { //UnderLine
          pixel = 255;
        }

        if (CheckState(attributes)) {
          pixel = 0;
        }
      }

      TextPallete[1] = Pallete8Bit[8 + ((attributes & 56) >> 3)];
      TextPallete[0] = Pallete8Bit[attributes & 7];
      systemState->PTRsurface8[yStride += 1] = TextPallete[pixel >> 7];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 6) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 5) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 4) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 3) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 2) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 1) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[pixel & 1];

      if (!systemState->ScanLines)
      {
        yStride -= (8);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = TextPallete[pixel >> 7];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 6) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 5) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 4) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 3) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 2) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 1) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[pixel & 1];
        yStride -= systemState->SurfacePitch;
      }
    }

    break;
  case 1:
  case 2: //Width 40
    attributes = 0;

    for (unsigned short beam = 0; beam < graphicsState.BytesperRow * graphicsState.ExtendedText; beam += graphicsState.ExtendedText)
    {
      character = buffer[start + (unsigned char)(beam + graphicsState.Hoffset)];
      pixel = cc3Fontdata8x12[character * 12 + (systemState->LineCounter % graphicsState.LinesperRow)];

      if (graphicsState.ExtendedText == 2)
      {
        attributes = buffer[start + (unsigned char)(beam + graphicsState.Hoffset) + 1];

        if ((attributes & 64) && (systemState->LineCounter % graphicsState.LinesperRow == (graphicsState.LinesperRow - 1))) { //UnderLine
          pixel = 255;
        }

        if (CheckState(attributes)) {
          pixel = 0;
        }
      }

      TextPallete[1] = Pallete8Bit[8 + ((attributes & 56) >> 3)];
      TextPallete[0] = Pallete8Bit[attributes & 7];
      systemState->PTRsurface8[yStride += 1] = TextPallete[pixel >> 7];
      systemState->PTRsurface8[yStride += 1] = TextPallete[pixel >> 7];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 6) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 6) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 5) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 5) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 4) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 4) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 3) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 3) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 2) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 2) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 1) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 1) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel & 1)];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel & 1)];
      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = TextPallete[pixel >> 7];
        systemState->PTRsurface8[yStride += 1] = TextPallete[pixel >> 7];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 6) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 6) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 5) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 5) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 4) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 4) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 3) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 3) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 2) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 2) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 1) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 1) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel & 1)];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel & 1)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;
    //	case 0:		//Hi Res text GraphicsMode=0 CompatMode=0 Ignore Bpp and Stretch
    //	case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
  case 17:
  case 18:
  case 19:
  case 20:
  case 21:
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 27:
  case 28:
  case 29:
  case 30:
  case 31:
  case 32:
  case 33:
  case 34:
  case 35:
  case 36:
  case 37:
  case 38:
  case 39:
  case 40:
  case 41:
  case 42:
  case 43:
  case 44:
  case 45:
  case 46:
  case 47:
  case 48:
  case 49:
  case 50:
  case 51:
  case 52:
  case 53:
  case 54:
  case 55:
  case 56:
  case 57:
  case 58:
  case 59:
  case 60:
  case 61:
  case 62:
  case 63:
    return; //TODO: Wait, what?

    for (unsigned short beam = 0; beam < graphicsState.BytesperRow * graphicsState.ExtendedText; beam += graphicsState.ExtendedText)
    {
      character = buffer[start + (unsigned char)(beam + graphicsState.Hoffset)];

      if (graphicsState.ExtendedText == 2) {
        attributes = buffer[start + (unsigned char)(beam + graphicsState.Hoffset) + 1];
      }
      else {
        attributes = 0;
      }

      pixel = cc3Fontdata8x12[(character & 127) * 8 + (systemState->LineCounter % 8)];

      if ((attributes & 64) && (systemState->LineCounter % 8 == 7)) { //UnderLine
        pixel = 255;
      }

      if (CheckState(attributes)) {
        pixel = 0;
      }

      TextPallete[1] = Pallete8Bit[8 + ((attributes & 56) >> 3)];
      TextPallete[0] = Pallete8Bit[attributes & 7];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel & 128) / 128];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel & 64) / 64];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel & 32) / 32];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel & 16) / 16];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel & 8) / 8];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel & 4) / 4];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel & 2) / 2];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel & 1)];
    }
    break;

    //******************************************************************** Low Res Text
  case 64:		//Low Res text GraphicsMode=0 CompatMode=1 Ignore Bpp and Stretch
  case 65:
  case 66:
  case 67:
  case 68:
  case 69:
  case 70:
  case 71:
  case 72:
  case 73:
  case 74:
  case 75:
  case 76:
  case 77:
  case 78:
  case 79:
  case 80:
  case 81:
  case 82:
  case 83:
  case 84:
  case 85:
  case 86:
  case 87:
  case 88:
  case 89:
  case 90:
  case 91:
  case 92:
  case 93:
  case 94:
  case 95:
  case 96:
  case 97:
  case 98:
  case 99:
  case 100:
  case 101:
  case 102:
  case 103:
  case 104:
  case 105:
  case 106:
  case 107:
  case 108:
  case 109:
  case 110:
  case 111:
  case 112:
  case 113:
  case 114:
  case 115:
  case 116:
  case 117:
  case 118:
  case 119:
  case 120:
  case 121:
  case 122:
  case 123:
  case 124:
  case 125:
  case 126:
  case 127:
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam++)
    {
      character = buffer[start + (unsigned char)(beam + graphicsState.Hoffset)];

      switch ((character & 192) >> 6)
      {
      case 0:
        character = character & 63;
        TextPallete[0] = Pallete8Bit[graphicsState.TextBGPallete];
        TextPallete[1] = Pallete8Bit[graphicsState.TextFGPallete];

        if (graphicsState.LowerCase & (character < 32))
          pixel = ntsc_round_fontdata8x12[(character + 80) * 12 + (systemState->LineCounter % 12)];
        else
          pixel = ~ntsc_round_fontdata8x12[(character) * 12 + (systemState->LineCounter % 12)];
        break;

      case 1:
        character = character & 63;
        TextPallete[0] = Pallete8Bit[graphicsState.TextBGPallete];
        TextPallete[1] = Pallete8Bit[graphicsState.TextFGPallete];
        pixel = ntsc_round_fontdata8x12[(character) * 12 + (systemState->LineCounter % 12)];
        break;

      case 2:
      case 3:
        TextPallete[1] = Pallete8Bit[(character & 112) >> 4];
        TextPallete[0] = Pallete8Bit[8];
        character = 64 + (character & 0xF);
        pixel = ntsc_round_fontdata8x12[(character) * 12 + (systemState->LineCounter % 12)];
        break;

      }

      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 7)];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 7)];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 6) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 6) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 5) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 5) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 4) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 4) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 3) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 3) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 2) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 2) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 1) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 1) & 1];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel & 1)];
      systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel & 1)];

      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 7)];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 7)];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 6) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 6) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 5) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 5) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 4) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 4) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 3) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 3) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 2) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 2) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 1) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel >> 1) & 1];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel & 1)];
        systemState->PTRsurface8[yStride += 1] = TextPallete[(pixel & 1)];
        yStride -= systemState->SurfacePitch;
      }
    } // Next HorzBeam

    break;

  case 128 + 0: //Bpp=0 Sr=0 1BPP Stretch=1
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=1
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

    //case 192+1:
  case 128 + 1: //Bpp=0 Sr=1 1BPP Stretch=2
  case 128 + 2:	//Bpp=0 Sr=2 
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=2
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];

      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (32);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 3: //Bpp=0 Sr=3 1BPP Stretch=4
  case 128 + 4: //Bpp=0 Sr=4
  case 128 + 5: //Bpp=0 Sr=5
  case 128 + 6: //Bpp=0 Sr=6
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=4
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];

      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (64);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 7: //Bpp=0 Sr=7 1BPP Stretch=8 
  case 128 + 8: //Bpp=0 Sr=8
  case 128 + 9: //Bpp=0 Sr=9
  case 128 + 10: //Bpp=0 Sr=10
  case 128 + 11: //Bpp=0 Sr=11
  case 128 + 12: //Bpp=0 Sr=12
  case 128 + 13: //Bpp=0 Sr=13
  case 128 + 14: //Bpp=0 Sr=14
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=8
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (128);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[1 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 15: //Bpp=0 Sr=15 1BPP Stretch=16
  case 128 + 16: //BPP=1 Sr=0  2BPP Stretch=1
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=1
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (8);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 17: //Bpp=1 Sr=1  2BPP Stretch=2
  case 128 + 18: //Bpp=1 Sr=2
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=2
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 19: //Bpp=1 Sr=3  2BPP Stretch=4
  case 128 + 20: //Bpp=1 Sr=4
  case 128 + 21: //Bpp=1 Sr=5
  case 128 + 22: //Bpp=1 Sr=6
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=4
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (32);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 23: //Bpp=1 Sr=7  2BPP Stretch=8
  case 128 + 24: //Bpp=1 Sr=8
  case 128 + 25: //Bpp=1 Sr=9 
  case 128 + 26: //Bpp=1 Sr=10 
  case 128 + 27: //Bpp=1 Sr=11 
  case 128 + 28: //Bpp=1 Sr=12 
  case 128 + 29: //Bpp=1 Sr=13 
  case 128 + 30: //Bpp=1 Sr=14
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=8
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (64);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 31: //Bpp=1 Sr=15 2BPP Stretch=16 
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=16
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (128);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[3 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 32: //Bpp=2 Sr=0 4BPP Stretch=1
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //4bbp Stretch=1
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      if (!systemState->ScanLines)
      {
        yStride -= (4);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 33: //Bpp=2 Sr=1 4BPP Stretch=2 
  case 128 + 34: //Bpp=2 Sr=2
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //4bbp Stretch=2
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (8);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 35: //Bpp=2 Sr=3 4BPP Stretch=4
  case 128 + 36: //Bpp=2 Sr=4 
  case 128 + 37: //Bpp=2 Sr=5 
  case 128 + 38: //Bpp=2 Sr=6 
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //4bbp Stretch=4
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 39: //Bpp=2 Sr=7 4BPP Stretch=8
  case 128 + 40: //Bpp=2 Sr=8 
  case 128 + 41: //Bpp=2 Sr=9 
  case 128 + 42: //Bpp=2 Sr=10 
  case 128 + 43: //Bpp=2 Sr=11 
  case 128 + 44: //Bpp=2 Sr=12 
  case 128 + 45: //Bpp=2 Sr=13 
  case 128 + 46: //Bpp=2 Sr=14 
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //4bbp Stretch=8
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (32);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 47: //Bpp=2 Sr=15 4BPP Stretch=16
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //4bbp Stretch=16
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (64);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & WidePixel];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[15 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 48: //Bpp=3 Sr=0  Unsupported 
  case 128 + 49: //Bpp=3 Sr=1 
  case 128 + 50: //Bpp=3 Sr=2 
  case 128 + 51: //Bpp=3 Sr=3 
  case 128 + 52: //Bpp=3 Sr=4 
  case 128 + 53: //Bpp=3 Sr=5 
  case 128 + 54: //Bpp=3 Sr=6 
  case 128 + 55: //Bpp=3 Sr=7 
  case 128 + 56: //Bpp=3 Sr=8 
  case 128 + 57: //Bpp=3 Sr=9 
  case 128 + 58: //Bpp=3 Sr=10 
  case 128 + 59: //Bpp=3 Sr=11 
  case 128 + 60: //Bpp=3 Sr=12 
  case 128 + 61: //Bpp=3 Sr=13 
  case 128 + 62: //Bpp=3 Sr=14 
  case 128 + 63: //Bpp=3 Sr=15 
    return;
    break;

  case 192 + 0: //Bpp=0 Sr=0 1BPP Stretch=1
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=1
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 192 + 1: //Bpp=0 Sr=1 1BPP Stretch=2
  case 192 + 2:	//Bpp=0 Sr=2 
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=2
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];

      if (!graphicsState.MonType)
      { //Pcolor
        for (bit = 7;bit >= 0;bit--)
        {
          pix = (1 & (WidePixel >> bit));
          phase = (carry2 << 2) | (carry1 << 1) | pix;

          switch (phase)
          {
          case 0:
          case 4:
          case 6:
            color = 0;
            break;

          case 1:
          case 5:
            color = (bit & 1) + 1;
            break;

          case 2:
            break;

          case 3:
            color = 3;
            systemState->PTRsurface8[yStride - 1] = Afacts8[graphicsState.ColorInvert][3];

            if (!systemState->ScanLines) {
              systemState->PTRsurface8[yStride + systemState->SurfacePitch - 1] = Afacts8[graphicsState.ColorInvert][3];
            }

            systemState->PTRsurface8[yStride] = Afacts8[graphicsState.ColorInvert][3];

            if (!systemState->ScanLines) {
              systemState->PTRsurface8[yStride + systemState->SurfacePitch] = Afacts8[graphicsState.ColorInvert][3];
            }

            break;

          case 7:
            color = 3;
            break;
          }

          systemState->PTRsurface8[yStride += 1] = Afacts8[graphicsState.ColorInvert][color];

          if (!systemState->ScanLines)
            systemState->PTRsurface8[yStride + systemState->SurfacePitch] = Afacts8[graphicsState.ColorInvert][color];

          systemState->PTRsurface8[yStride += 1] = Afacts8[graphicsState.ColorInvert][color];

          if (!systemState->ScanLines)
            systemState->PTRsurface8[yStride + systemState->SurfacePitch] = Afacts8[graphicsState.ColorInvert][color];

          carry2 = carry1;
          carry1 = pix;
        }

        for (bit = 15; bit >= 8; bit--)
        {
          pix = (1 & (WidePixel >> bit));
          phase = (carry2 << 2) | (carry1 << 1) | pix;

          switch (phase)
          {
          case 0:
          case 4:
          case 6:
            color = 0;
            break;

          case 1:
          case 5:
            color = (bit & 1) + 1;
            break;

          case 2:
            break;

          case 3:
            color = 3;
            systemState->PTRsurface8[yStride - 1] = Afacts8[graphicsState.ColorInvert][3];

            if (!systemState->ScanLines)
              systemState->PTRsurface8[yStride + systemState->SurfacePitch - 1] = Afacts8[graphicsState.ColorInvert][3];

            systemState->PTRsurface8[yStride] = Afacts8[graphicsState.ColorInvert][3];

            if (!systemState->ScanLines)
              systemState->PTRsurface8[yStride + systemState->SurfacePitch] = Afacts8[graphicsState.ColorInvert][3];

            break;

          case 7:
            color = 3;
            break;
          }

          systemState->PTRsurface8[yStride += 1] = Afacts8[graphicsState.ColorInvert][color];

          if (!systemState->ScanLines)
            systemState->PTRsurface8[yStride + systemState->SurfacePitch] = Afacts8[graphicsState.ColorInvert][color];

          systemState->PTRsurface8[yStride += 1] = Afacts8[graphicsState.ColorInvert][color];

          if (!systemState->ScanLines)
            systemState->PTRsurface8[yStride + systemState->SurfacePitch] = Afacts8[graphicsState.ColorInvert][color];

          carry2 = carry1;
          carry1 = pix;
        }
      }
      else
      {
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];

        if (!systemState->ScanLines)
        {
          yStride -= (32);
          yStride += systemState->SurfacePitch;
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
          systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
          yStride -= systemState->SurfacePitch;
        }
      }

    }
    break;

  case 192 + 3: //Bpp=0 Sr=3 1BPP Stretch=4
  case 192 + 4: //Bpp=0 Sr=4
  case 192 + 5: //Bpp=0 Sr=5
  case 192 + 6: //Bpp=0 Sr=6
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=4
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];

      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (64);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 192 + 7: //Bpp=0 Sr=7 1BPP Stretch=8 
  case 192 + 8: //Bpp=0 Sr=8
  case 192 + 9: //Bpp=0 Sr=9
  case 192 + 10: //Bpp=0 Sr=10
  case 192 + 11: //Bpp=0 Sr=11
  case 192 + 12: //Bpp=0 Sr=12
  case 192 + 13: //Bpp=0 Sr=13
  case 192 + 14: //Bpp=0 Sr=14
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=8
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (128);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 192 + 15: //Bpp=0 Sr=15 1BPP Stretch=16
  case 192 + 16: //BPP=1 Sr=0  2BPP Stretch=1
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=1
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (8);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 192 + 17: //Bpp=1 Sr=1  2BPP Stretch=2
  case 192 + 18: //Bpp=1 Sr=2
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=2
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 192 + 19: //Bpp=1 Sr=3  2BPP Stretch=4
  case 192 + 20: //Bpp=1 Sr=4
  case 192 + 21: //Bpp=1 Sr=5
  case 192 + 22: //Bpp=1 Sr=6
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=4
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (32);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 192 + 23: //Bpp=1 Sr=7  2BPP Stretch=8
  case 192 + 24: //Bpp=1 Sr=8
  case 192 + 25: //Bpp=1 Sr=9 
  case 192 + 26: //Bpp=1 Sr=10 
  case 192 + 27: //Bpp=1 Sr=11 
  case 192 + 28: //Bpp=1 Sr=12 
  case 192 + 29: //Bpp=1 Sr=13 
  case 192 + 30: //Bpp=1 Sr=14
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=8
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (64);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 192 + 31: //Bpp=1 Sr=15 2BPP Stretch=16 
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=16
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (128);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface8[yStride += 1] = Pallete8Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 192 + 32: //Bpp=2 Sr=0 4BPP Stretch=1 Unsupport with Compat set
  case 192 + 33: //Bpp=2 Sr=1 4BPP Stretch=2 
  case 192 + 34: //Bpp=2 Sr=2
  case 192 + 35: //Bpp=2 Sr=3 4BPP Stretch=4
  case 192 + 36: //Bpp=2 Sr=4 
  case 192 + 37: //Bpp=2 Sr=5 
  case 192 + 38: //Bpp=2 Sr=6 
  case 192 + 39: //Bpp=2 Sr=7 4BPP Stretch=8
  case 192 + 40: //Bpp=2 Sr=8 
  case 192 + 41: //Bpp=2 Sr=9 
  case 192 + 42: //Bpp=2 Sr=10 
  case 192 + 43: //Bpp=2 Sr=11 
  case 192 + 44: //Bpp=2 Sr=12 
  case 192 + 45: //Bpp=2 Sr=13 
  case 192 + 46: //Bpp=2 Sr=14 
  case 192 + 47: //Bpp=2 Sr=15 4BPP Stretch=16
  case 192 + 48: //Bpp=3 Sr=0  Unsupported 
  case 192 + 49: //Bpp=3 Sr=1 
  case 192 + 50: //Bpp=3 Sr=2 
  case 192 + 51: //Bpp=3 Sr=3 
  case 192 + 52: //Bpp=3 Sr=4 
  case 192 + 53: //Bpp=3 Sr=5 
  case 192 + 54: //Bpp=3 Sr=6 
  case 192 + 55: //Bpp=3 Sr=7 
  case 192 + 56: //Bpp=3 Sr=8 
  case 192 + 57: //Bpp=3 Sr=9 
  case 192 + 58: //Bpp=3 Sr=10 
  case 192 + 59: //Bpp=3 Sr=11 
  case 192 + 60: //Bpp=3 Sr=12 
  case 192 + 61: //Bpp=3 Sr=13 
  case 192 + 62: //Bpp=3 Sr=14 
  case 192 + 63: //Bpp=3 Sr=15 
    return;
    break;

  }
  return;
}

void UpdateScreen16(SystemState* systemState)
{
  register unsigned int yStride = 0;
  static unsigned int textColor = 0;
  static unsigned char pixel = 0;
  static unsigned char character = 0, attributes = 0;
  static unsigned short TextPallete[2] = { 0,0 };
  static unsigned short WidePixel = 0;
  static char pix = 0, bit = 0, phase = 0;
  static char carry2 = 0;
  char color = 0;
  char carry1 = 1;

  if ((graphicsState.HorzCenter != 0) && (graphicsState.BoarderChange > 0))
    for (unsigned short x = 0; x < graphicsState.HorzCenter; x++)
    {
      systemState->PTRsurface16[x + (((systemState->LineCounter + graphicsState.VertCenter) * 2) * (systemState->SurfacePitch))] = graphicsState.BoarderColor16;

      if (!systemState->ScanLines)
        systemState->PTRsurface16[x + (((systemState->LineCounter + graphicsState.VertCenter) * 2 + 1) * (systemState->SurfacePitch))] = graphicsState.BoarderColor16;

      systemState->PTRsurface16[x + (graphicsState.PixelsperLine * (graphicsState.Stretch + 1)) + graphicsState.HorzCenter + (((systemState->LineCounter + graphicsState.VertCenter) * 2) * (systemState->SurfacePitch))] = graphicsState.BoarderColor16;

      if (!systemState->ScanLines)
        systemState->PTRsurface16[x + (graphicsState.PixelsperLine * (graphicsState.Stretch + 1)) + graphicsState.HorzCenter + (((systemState->LineCounter + graphicsState.VertCenter) * 2 + 1) * (systemState->SurfacePitch))] = graphicsState.BoarderColor16;
    }

  if (graphicsState.LinesperRow < 13) {
    graphicsState.TagY++;
  }

  if (!systemState->LineCounter)
  {
    graphicsState.StartofVidram = graphicsState.NewStartofVidram;
    graphicsState.TagY = systemState->LineCounter;
  }

  unsigned int start = graphicsState.StartofVidram + (graphicsState.TagY / graphicsState.LinesperRow) * (graphicsState.VPitch * graphicsState.ExtendedText);
  yStride = (((systemState->LineCounter + graphicsState.VertCenter) * 2) * systemState->SurfacePitch) + (graphicsState.HorzCenter * 1) - 1;

  switch (graphicsState.MasterMode) // (GraphicsMode <<7) | (CompatMode<<6)  | ((Bpp & 3)<<4) | (Stretch & 15);
  {
  case 0: //Width 80
    attributes = 0;
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow * graphicsState.ExtendedText; beam += graphicsState.ExtendedText)
    {
      character = systemState->RamBuffer[graphicsState.VidMask & (start + (unsigned char)(beam + graphicsState.Hoffset))];
      pixel = cc3Fontdata8x12[character * 12 + (systemState->LineCounter % graphicsState.LinesperRow)];

      if (graphicsState.ExtendedText == 2)
      {
        attributes = systemState->RamBuffer[graphicsState.VidMask & (start + (unsigned char)(beam + graphicsState.Hoffset) + 1)];

        if ((attributes & 64) && (systemState->LineCounter % graphicsState.LinesperRow == (graphicsState.LinesperRow - 1))) {	//UnderLine
          pixel = 255;
        }

        if (CheckState(attributes)) {
          pixel = 0;
        }
      }

      TextPallete[1] = Pallete16Bit[8 + ((attributes & 56) >> 3)];
      TextPallete[0] = Pallete16Bit[attributes & 7];
      systemState->PTRsurface16[yStride += 1] = TextPallete[pixel >> 7];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 6) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 5) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 4) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 3) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 2) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 1) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[pixel & 1];

      if (!systemState->ScanLines)
      {
        yStride -= (8);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = TextPallete[pixel >> 7];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 6) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 5) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 4) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 3) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 2) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 1) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[pixel & 1];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 1:
  case 2: //Width 40
    attributes = 0;

    for (unsigned short beam = 0; beam < graphicsState.BytesperRow * graphicsState.ExtendedText; beam += graphicsState.ExtendedText)
    {
      character = systemState->RamBuffer[graphicsState.VidMask & (start + (unsigned char)(beam + graphicsState.Hoffset))];
      pixel = cc3Fontdata8x12[character * 12 + (systemState->LineCounter % graphicsState.LinesperRow)];

      if (graphicsState.ExtendedText == 2)
      {
        attributes = systemState->RamBuffer[graphicsState.VidMask & (start + (unsigned char)(beam + graphicsState.Hoffset) + 1)];

        if ((attributes & 64) && (systemState->LineCounter % graphicsState.LinesperRow == (graphicsState.LinesperRow - 1))) { //UnderLine
          pixel = 255;
        }

        if (CheckState(attributes)) {
          pixel = 0;
        }
      }

      TextPallete[1] = Pallete16Bit[8 + ((attributes & 56) >> 3)];
      TextPallete[0] = Pallete16Bit[attributes & 7];
      systemState->PTRsurface16[yStride += 1] = TextPallete[pixel >> 7];
      systemState->PTRsurface16[yStride += 1] = TextPallete[pixel >> 7];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 6) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 6) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 5) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 5) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 4) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 4) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 3) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 3) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 2) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 2) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 1) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 1) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel & 1)];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel & 1)];

      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = TextPallete[pixel >> 7];
        systemState->PTRsurface16[yStride += 1] = TextPallete[pixel >> 7];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 6) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 6) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 5) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 5) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 4) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 4) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 3) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 3) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 2) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 2) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 1) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 1) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel & 1)];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel & 1)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

    //	case 0:		//Hi Res text GraphicsMode=0 CompatMode=0 Ignore Bpp and Stretch
    //	case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
  case 17:
  case 18:
  case 19:
  case 20:
  case 21:
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 27:
  case 28:
  case 29:
  case 30:
  case 31:
  case 32:
  case 33:
  case 34:
  case 35:
  case 36:
  case 37:
  case 38:
  case 39:
  case 40:
  case 41:
  case 42:
  case 43:
  case 44:
  case 45:
  case 46:
  case 47:
  case 48:
  case 49:
  case 50:
  case 51:
  case 52:
  case 53:
  case 54:
  case 55:
  case 56:
  case 57:
  case 58:
  case 59:
  case 60:
  case 61:
  case 62:
  case 63:
    return; //TODO: Why?

    for (unsigned short beam = 0; beam < graphicsState.BytesperRow * graphicsState.ExtendedText; beam += graphicsState.ExtendedText)
    {
      character = systemState->RamBuffer[graphicsState.VidMask & (start + (unsigned char)(beam + graphicsState.Hoffset))];

      if (graphicsState.ExtendedText == 2) {
        attributes = systemState->RamBuffer[graphicsState.VidMask & (start + (unsigned char)(beam + graphicsState.Hoffset) + 1)];
      }
      else {
        attributes = 0;
      }

      pixel = cc3Fontdata8x12[(character & 127) * 8 + (systemState->LineCounter % 8)];

      if ((attributes & 64) && (systemState->LineCounter % 8 == 7)) { //UnderLine
        pixel = 255;
      }

      if (CheckState(attributes)) {
        pixel = 0;
      }

      TextPallete[1] = Pallete16Bit[8 + ((attributes & 56) >> 3)];
      TextPallete[0] = Pallete16Bit[attributes & 7];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel & 128) / 128];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel & 64) / 64];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel & 32) / 32];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel & 16) / 16];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel & 8) / 8];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel & 4) / 4];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel & 2) / 2];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel & 1)];
    }
    break;

    //******************************************************************** Low Res Text
  case 64:		//Low Res text GraphicsMode=0 CompatMode=1 Ignore Bpp and Stretch
  case 65:
  case 66:
  case 67:
  case 68:
  case 69:
  case 70:
  case 71:
  case 72:
  case 73:
  case 74:
  case 75:
  case 76:
  case 77:
  case 78:
  case 79:
  case 80:
  case 81:
  case 82:
  case 83:
  case 84:
  case 85:
  case 86:
  case 87:
  case 88:
  case 89:
  case 90:
  case 91:
  case 92:
  case 93:
  case 94:
  case 95:
  case 96:
  case 97:
  case 98:
  case 99:
  case 100:
  case 101:
  case 102:
  case 103:
  case 104:
  case 105:
  case 106:
  case 107:
  case 108:
  case 109:
  case 110:
  case 111:
  case 112:
  case 113:
  case 114:
  case 115:
  case 116:
  case 117:
  case 118:
  case 119:
  case 120:
  case 121:
  case 122:
  case 123:
  case 124:
  case 125:
  case 126:
  case 127:

    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam++)
    {
      character = systemState->RamBuffer[graphicsState.VidMask & (start + (unsigned char)(beam + graphicsState.Hoffset))];

      switch ((character & 192) >> 6)
      {
      case 0:
        character = character & 63;
        TextPallete[0] = Pallete16Bit[graphicsState.TextBGPallete];
        TextPallete[1] = Pallete16Bit[graphicsState.TextFGPallete];

        if (graphicsState.LowerCase & (character < 32))
          pixel = ntsc_round_fontdata8x12[(character + 80) * 12 + (systemState->LineCounter % 12)];
        else
          pixel = ~ntsc_round_fontdata8x12[(character) * 12 + (systemState->LineCounter % 12)];
        break;

      case 1:
        character = character & 63;
        TextPallete[0] = Pallete16Bit[graphicsState.TextBGPallete];
        TextPallete[1] = Pallete16Bit[graphicsState.TextFGPallete];
        pixel = ntsc_round_fontdata8x12[(character) * 12 + (systemState->LineCounter % 12)];
        break;

      case 2:
      case 3:
        TextPallete[1] = Pallete16Bit[(character & 112) >> 4];
        TextPallete[0] = Pallete16Bit[8];
        character = 64 + (character & 0xF);
        pixel = ntsc_round_fontdata8x12[(character) * 12 + (systemState->LineCounter % 12)];
        break;

      }

      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 7)];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 7)];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 6) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 6) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 5) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 5) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 4) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 4) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 3) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 3) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 2) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 2) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 1) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 1) & 1];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel & 1)];
      systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel & 1)];

      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 7)];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 7)];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 6) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 6) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 5) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 5) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 4) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 4) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 3) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 3) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 2) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 2) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 1) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel >> 1) & 1];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel & 1)];
        systemState->PTRsurface16[yStride += 1] = TextPallete[(pixel & 1)];
        yStride -= systemState->SurfacePitch;
      }
    }

    break;

  case 128 + 0: //Bpp=0 Sr=0 1BPP Stretch=1
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=1
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

    //case 192+1:
  case 128 + 1: //Bpp=0 Sr=1 1BPP Stretch=2
  case 128 + 2:	//Bpp=0 Sr=2 
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=2
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];

      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (32);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 3: //Bpp=0 Sr=3 1BPP Stretch=4
  case 128 + 4: //Bpp=0 Sr=4
  case 128 + 5: //Bpp=0 Sr=5
  case 128 + 6: //Bpp=0 Sr=6
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=4
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];

      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (64);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 7: //Bpp=0 Sr=7 1BPP Stretch=8 
  case 128 + 8: //Bpp=0 Sr=8
  case 128 + 9: //Bpp=0 Sr=9
  case 128 + 10: //Bpp=0 Sr=10
  case 128 + 11: //Bpp=0 Sr=11
  case 128 + 12: //Bpp=0 Sr=12
  case 128 + 13: //Bpp=0 Sr=13
  case 128 + 14: //Bpp=0 Sr=14
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=8
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (128);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 7)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 5)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 3)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 1)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 15)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 13)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 11)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 9)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[1 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 15: //Bpp=0 Sr=15 1BPP Stretch=16
  case 128 + 16: //BPP=1 Sr=0  2BPP Stretch=1
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=1
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (8);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 17: //Bpp=1 Sr=1  2BPP Stretch=2
  case 128 + 18: //Bpp=1 Sr=2
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=2
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 19: //Bpp=1 Sr=3  2BPP Stretch=4
  case 128 + 20: //Bpp=1 Sr=4
  case 128 + 21: //Bpp=1 Sr=5
  case 128 + 22: //Bpp=1 Sr=6
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=4
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (32);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 23: //Bpp=1 Sr=7  2BPP Stretch=8
  case 128 + 24: //Bpp=1 Sr=8
  case 128 + 25: //Bpp=1 Sr=9 
  case 128 + 26: //Bpp=1 Sr=10 
  case 128 + 27: //Bpp=1 Sr=11 
  case 128 + 28: //Bpp=1 Sr=12 
  case 128 + 29: //Bpp=1 Sr=13 
  case 128 + 30: //Bpp=1 Sr=14
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=8
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (64);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 31: //Bpp=1 Sr=15 2BPP Stretch=16 
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=16
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (128);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 6)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 2)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 14)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 10)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[3 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 32: //Bpp=2 Sr=0 4BPP Stretch=1
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //4bbp Stretch=1
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (4);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 33: //Bpp=2 Sr=1 4BPP Stretch=2 
  case 128 + 34: //Bpp=2 Sr=2
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //4bbp Stretch=2
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (8);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 35: //Bpp=2 Sr=3 4BPP Stretch=4
  case 128 + 36: //Bpp=2 Sr=4 
  case 128 + 37: //Bpp=2 Sr=5 
  case 128 + 38: //Bpp=2 Sr=6 
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //4bbp Stretch=4
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 39: //Bpp=2 Sr=7 4BPP Stretch=8
  case 128 + 40: //Bpp=2 Sr=8 
  case 128 + 41: //Bpp=2 Sr=9 
  case 128 + 42: //Bpp=2 Sr=10 
  case 128 + 43: //Bpp=2 Sr=11 
  case 128 + 44: //Bpp=2 Sr=12 
  case 128 + 45: //Bpp=2 Sr=13 
  case 128 + 46: //Bpp=2 Sr=14 
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //4bbp Stretch=8
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (32);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 47: //Bpp=2 Sr=15 4BPP Stretch=16
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //4bbp Stretch=16
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (64);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 4)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & WidePixel];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 12)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[15 & (WidePixel >> 8)];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 128 + 48: //Bpp=3 Sr=0  Unsupported 
  case 128 + 49: //Bpp=3 Sr=1 
  case 128 + 50: //Bpp=3 Sr=2 
  case 128 + 51: //Bpp=3 Sr=3 
  case 128 + 52: //Bpp=3 Sr=4 
  case 128 + 53: //Bpp=3 Sr=5 
  case 128 + 54: //Bpp=3 Sr=6 
  case 128 + 55: //Bpp=3 Sr=7 
  case 128 + 56: //Bpp=3 Sr=8 
  case 128 + 57: //Bpp=3 Sr=9 
  case 128 + 58: //Bpp=3 Sr=10 
  case 128 + 59: //Bpp=3 Sr=11 
  case 128 + 60: //Bpp=3 Sr=12 
  case 128 + 61: //Bpp=3 Sr=13 
  case 128 + 62: //Bpp=3 Sr=14 
  case 128 + 63: //Bpp=3 Sr=15 
    return;
    break;

  case 192 + 0: //Bpp=0 Sr=0 1BPP Stretch=1
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=1
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 192 + 1: //Bpp=0 Sr=1 1BPP Stretch=2
  case 192 + 2:	//Bpp=0 Sr=2 
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=2
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];

      if (!graphicsState.MonType)
      { //Pcolor
        for (bit = 7; bit >= 0; bit--)
        {
          pix = (1 & (WidePixel >> bit));
          phase = (carry2 << 2) | (carry1 << 1) | pix;

          switch (phase)
          {
          case 0:
          case 4:
          case 6:
            color = 0;
            break;

          case 1:
          case 5:
            color = (bit & 1) + 1;
            break;

          case 2:
            break;

          case 3:
            color = 3;
            systemState->PTRsurface16[yStride - 1] = Afacts16[graphicsState.ColorInvert][3];

            if (!systemState->ScanLines) {
              systemState->PTRsurface16[yStride + systemState->SurfacePitch - 1] = Afacts16[graphicsState.ColorInvert][3];
            }

            systemState->PTRsurface16[yStride] = Afacts16[graphicsState.ColorInvert][3];

            if (!systemState->ScanLines) {
              systemState->PTRsurface16[yStride + systemState->SurfacePitch] = Afacts16[graphicsState.ColorInvert][3];
            }

            break;

          case 7:
            color = 3;
            break;
          }

          systemState->PTRsurface16[yStride += 1] = Afacts16[graphicsState.ColorInvert][color];

          if (!systemState->ScanLines) {
            systemState->PTRsurface16[yStride + systemState->SurfacePitch] = Afacts16[graphicsState.ColorInvert][color];
          }

          systemState->PTRsurface16[yStride += 1] = Afacts16[graphicsState.ColorInvert][color];

          if (!systemState->ScanLines) {
            systemState->PTRsurface16[yStride + systemState->SurfacePitch] = Afacts16[graphicsState.ColorInvert][color];
          }

          carry2 = carry1;
          carry1 = pix;
        }

        for (bit = 15; bit >= 8; bit--)
        {
          pix = (1 & (WidePixel >> bit));
          phase = (carry2 << 2) | (carry1 << 1) | pix;

          switch (phase)
          {
          case 0:
          case 4:
          case 6:
            color = 0;
            break;

          case 1:
          case 5:
            color = (bit & 1) + 1;
            break;

          case 2:
            break;

          case 3:
            color = 3;
            systemState->PTRsurface16[yStride - 1] = Afacts16[graphicsState.ColorInvert][3];

            if (!systemState->ScanLines) {
              systemState->PTRsurface16[yStride + systemState->SurfacePitch - 1] = Afacts16[graphicsState.ColorInvert][3];
            }

            systemState->PTRsurface16[yStride] = Afacts16[graphicsState.ColorInvert][3];

            if (!systemState->ScanLines) {
              systemState->PTRsurface16[yStride + systemState->SurfacePitch] = Afacts16[graphicsState.ColorInvert][3];
            }

            break;

          case 7:
            color = 3;
            break;
          }

          systemState->PTRsurface16[yStride += 1] = Afacts16[graphicsState.ColorInvert][color];

          if (!systemState->ScanLines) {
            systemState->PTRsurface16[yStride + systemState->SurfacePitch] = Afacts16[graphicsState.ColorInvert][color];
          }

          systemState->PTRsurface16[yStride += 1] = Afacts16[graphicsState.ColorInvert][color];

          if (!systemState->ScanLines) {
            systemState->PTRsurface16[yStride + systemState->SurfacePitch] = Afacts16[graphicsState.ColorInvert][color];
          }

          carry2 = carry1;
          carry1 = pix;
        }
      }
      else
      {
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];

        if (!systemState->ScanLines)
        {
          yStride -= (32);
          yStride += systemState->SurfacePitch;
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
          systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
          yStride -= systemState->SurfacePitch;
        }
      }
    }
    break;

  case 192 + 3: //Bpp=0 Sr=3 1BPP Stretch=4
  case 192 + 4: //Bpp=0 Sr=4
  case 192 + 5: //Bpp=0 Sr=5
  case 192 + 6: //Bpp=0 Sr=6
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=4
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];

      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (64);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 192 + 7: //Bpp=0 Sr=7 1BPP Stretch=8 
  case 192 + 8: //Bpp=0 Sr=8
  case 192 + 9: //Bpp=0 Sr=9
  case 192 + 10: //Bpp=0 Sr=10
  case 192 + 11: //Bpp=0 Sr=11
  case 192 + 12: //Bpp=0 Sr=12
  case 192 + 13: //Bpp=0 Sr=13
  case 192 + 14: //Bpp=0 Sr=14
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=8
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (128);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 192 + 15: //Bpp=0 Sr=15 1BPP Stretch=16
  case 192 + 16: //BPP=1 Sr=0  2BPP Stretch=1
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=1
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (8);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 192 + 17: //Bpp=1 Sr=1  2BPP Stretch=2
  case 192 + 18: //Bpp=1 Sr=2
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=2
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 192 + 19: //Bpp=1 Sr=3  2BPP Stretch=4
  case 192 + 20: //Bpp=1 Sr=4
  case 192 + 21: //Bpp=1 Sr=5
  case 192 + 22: //Bpp=1 Sr=6
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=4
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (32);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 192 + 23: //Bpp=1 Sr=7  2BPP Stretch=8
  case 192 + 24: //Bpp=1 Sr=8
  case 192 + 25: //Bpp=1 Sr=9 
  case 192 + 26: //Bpp=1 Sr=10 
  case 192 + 27: //Bpp=1 Sr=11 
  case 192 + 28: //Bpp=1 Sr=12 
  case 192 + 29: //Bpp=1 Sr=13 
  case 192 + 30: //Bpp=1 Sr=14
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=8
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (64);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 192 + 31: //Bpp=1 Sr=15 2BPP Stretch=16 
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=16
    {
      WidePixel = systemState->WRamBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (128);
        yStride += systemState->SurfacePitch;
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        systemState->PTRsurface16[yStride += 1] = Pallete16Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        yStride -= systemState->SurfacePitch;
      }
    }
    break;

  case 192 + 32: //Bpp=2 Sr=0 4BPP Stretch=1 Unsupport with Compat set
  case 192 + 33: //Bpp=2 Sr=1 4BPP Stretch=2 
  case 192 + 34: //Bpp=2 Sr=2
  case 192 + 35: //Bpp=2 Sr=3 4BPP Stretch=4
  case 192 + 36: //Bpp=2 Sr=4 
  case 192 + 37: //Bpp=2 Sr=5 
  case 192 + 38: //Bpp=2 Sr=6 
  case 192 + 39: //Bpp=2 Sr=7 4BPP Stretch=8
  case 192 + 40: //Bpp=2 Sr=8 
  case 192 + 41: //Bpp=2 Sr=9 
  case 192 + 42: //Bpp=2 Sr=10 
  case 192 + 43: //Bpp=2 Sr=11 
  case 192 + 44: //Bpp=2 Sr=12 
  case 192 + 45: //Bpp=2 Sr=13 
  case 192 + 46: //Bpp=2 Sr=14 
  case 192 + 47: //Bpp=2 Sr=15 4BPP Stretch=16
  case 192 + 48: //Bpp=3 Sr=0  Unsupported 
  case 192 + 49: //Bpp=3 Sr=1 
  case 192 + 50: //Bpp=3 Sr=2 
  case 192 + 51: //Bpp=3 Sr=3 
  case 192 + 52: //Bpp=3 Sr=4 
  case 192 + 53: //Bpp=3 Sr=5 
  case 192 + 54: //Bpp=3 Sr=6 
  case 192 + 55: //Bpp=3 Sr=7 
  case 192 + 56: //Bpp=3 Sr=8 
  case 192 + 57: //Bpp=3 Sr=9 
  case 192 + 58: //Bpp=3 Sr=10 
  case 192 + 59: //Bpp=3 Sr=11 
  case 192 + 60: //Bpp=3 Sr=12 
  case 192 + 61: //Bpp=3 Sr=13 
  case 192 + 62: //Bpp=3 Sr=14 
  case 192 + 63: //Bpp=3 Sr=15 
    return;
    break;

  }

  return;
}

void UpdateScreen24(SystemState* systemState)
{
  return;
}

void UpdateScreen32(SystemState* systemState)
{
  register unsigned int yStride = 0;
  unsigned char pixel = 0;
  unsigned char character = 0, attributes = 0;
  unsigned int textPallete[2] = { 0,0 };
  unsigned short* WideBuffer = (unsigned short*)systemState->RamBuffer;
  unsigned char* buffer = systemState->RamBuffer;
  unsigned short WidePixel = 0;
  char pix = 0, bit = 0, phase = 0;
  static char carry1 = 0, carry2 = 0;
  static char color = 0;
  unsigned int* szSurface32 = systemState->PTRsurface32;
  unsigned short y = systemState->LineCounter;
  long Xpitch = systemState->SurfacePitch;
  carry1 = 1;
  color = 0;

  if ((graphicsState.HorzCenter != 0) && (graphicsState.BoarderChange > 0)) {
    for (unsigned short x = 0; x < graphicsState.HorzCenter; x++)
    {
      szSurface32[x + (((y + graphicsState.VertCenter) * 2) * Xpitch)] = graphicsState.BoarderColor32;

      if (!systemState->ScanLines) {
        szSurface32[x + (((y + graphicsState.VertCenter) * 2 + 1) * Xpitch)] = graphicsState.BoarderColor32;
      }

      szSurface32[x + (graphicsState.PixelsperLine * (graphicsState.Stretch + 1)) + graphicsState.HorzCenter + (((y + graphicsState.VertCenter) * 2) * Xpitch)] = graphicsState.BoarderColor32;

      if (!systemState->ScanLines) {
        szSurface32[x + (graphicsState.PixelsperLine * (graphicsState.Stretch + 1)) + graphicsState.HorzCenter + (((y + graphicsState.VertCenter) * 2 + 1) * Xpitch)] = graphicsState.BoarderColor32;
      }
    }
  }

  if (graphicsState.LinesperRow < 13) {
    graphicsState.TagY++;
  }

  if (!y)
  {
    graphicsState.StartofVidram = graphicsState.NewStartofVidram;
    graphicsState.TagY = y;
  }

  unsigned int start = graphicsState.StartofVidram + (graphicsState.TagY / graphicsState.LinesperRow) * (graphicsState.VPitch * graphicsState.ExtendedText);
  yStride = (((y + graphicsState.VertCenter) * 2) * Xpitch) + (graphicsState.HorzCenter * 1) - 1;

  switch (graphicsState.MasterMode) // (GraphicsMode <<7) | (CompatMode<<6)  | ((Bpp & 3)<<4) | (Stretch & 15);
  {
  case 0: //Width 80
    attributes = 0;

    if (graphicsState.HorzOffsetReg & 128) {
      start = graphicsState.StartofVidram + (graphicsState.TagY / graphicsState.LinesperRow) * (graphicsState.VPitch); //Fix for Horizontal Offset Register in text mode.
    }

    for (unsigned short beam = 0; beam < graphicsState.BytesperRow * graphicsState.ExtendedText; beam += graphicsState.ExtendedText)
    {
      character = buffer[start + (unsigned char)(beam + graphicsState.Hoffset)];
      pixel = cc3Fontdata8x12[character * 12 + (y % graphicsState.LinesperRow)];

      if (graphicsState.ExtendedText == 2)
      {
        attributes = buffer[start + (unsigned char)(beam + graphicsState.Hoffset) + 1];

        if ((attributes & 64) && (y % graphicsState.LinesperRow == (graphicsState.LinesperRow - 1))) {	//UnderLine
          pixel = 255;
        }

        if (CheckState(attributes)) {
          pixel = 0;
        }
      }

      textPallete[1] = Pallete32Bit[8 + ((attributes & 56) >> 3)];
      textPallete[0] = Pallete32Bit[attributes & 7];
      szSurface32[yStride += 1] = textPallete[pixel >> 7];
      szSurface32[yStride += 1] = textPallete[(pixel >> 6) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 5) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 4) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 3) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 2) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 1) & 1];
      szSurface32[yStride += 1] = textPallete[pixel & 1];

      if (!systemState->ScanLines)
      {
        yStride -= (8);
        yStride += Xpitch;
        szSurface32[yStride += 1] = textPallete[pixel >> 7];
        szSurface32[yStride += 1] = textPallete[(pixel >> 6) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 5) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 4) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 3) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 2) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 1) & 1];
        szSurface32[yStride += 1] = textPallete[pixel & 1];
        yStride -= Xpitch;
      }
    }

    break;

  case 1:
  case 2: //Width 40
    attributes = 0;

    for (unsigned short beam = 0; beam < graphicsState.BytesperRow * graphicsState.ExtendedText; beam += graphicsState.ExtendedText)
    {
      character = buffer[start + (unsigned char)(beam + graphicsState.Hoffset)];
      pixel = cc3Fontdata8x12[character * 12 + (y % graphicsState.LinesperRow)];

      if (graphicsState.ExtendedText == 2)
      {
        attributes = buffer[start + (unsigned char)(beam + graphicsState.Hoffset) + 1];

        if ((attributes & 64) && (y % graphicsState.LinesperRow == (graphicsState.LinesperRow - 1))) {	//UnderLine
          pixel = 255;
        }

        if (CheckState(attributes)) {
          pixel = 0;
        }
      }

      textPallete[1] = Pallete32Bit[8 + ((attributes & 56) >> 3)];
      textPallete[0] = Pallete32Bit[attributes & 7];
      szSurface32[yStride += 1] = textPallete[pixel >> 7];
      szSurface32[yStride += 1] = textPallete[pixel >> 7];
      szSurface32[yStride += 1] = textPallete[(pixel >> 6) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 6) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 5) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 5) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 4) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 4) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 3) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 3) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 2) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 2) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 1) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 1) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel & 1)];
      szSurface32[yStride += 1] = textPallete[(pixel & 1)];

      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += Xpitch;
        szSurface32[yStride += 1] = textPallete[pixel >> 7];
        szSurface32[yStride += 1] = textPallete[pixel >> 7];
        szSurface32[yStride += 1] = textPallete[(pixel >> 6) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 6) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 5) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 5) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 4) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 4) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 3) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 3) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 2) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 2) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 1) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 1) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel & 1)];
        szSurface32[yStride += 1] = textPallete[(pixel & 1)];
        yStride -= Xpitch;
      }
    }
    break;

    //	case 0:		//Hi Res text GraphicsMode=0 CompatMode=0 Ignore Bpp and Stretch
    //	case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
  case 17:
  case 18:
  case 19:
  case 20:
  case 21:
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 27:
  case 28:
  case 29:
  case 30:
  case 31:
  case 32:
  case 33:
  case 34:
  case 35:
  case 36:
  case 37:
  case 38:
  case 39:
  case 40:
  case 41:
  case 42:
  case 43:
  case 44:
  case 45:
  case 46:
  case 47:
  case 48:
  case 49:
  case 50:
  case 51:
  case 52:
  case 53:
  case 54:
  case 55:
  case 56:
  case 57:
  case 58:
  case 59:
  case 60:
  case 61:
  case 62:
  case 63:
    return; //TODO: Why?

    for (unsigned short beam = 0; beam < graphicsState.BytesperRow * graphicsState.ExtendedText; beam += graphicsState.ExtendedText)
    {
      character = buffer[start + (unsigned char)(beam + graphicsState.Hoffset)];

      if (graphicsState.ExtendedText == 2) {
        attributes = buffer[start + (unsigned char)(beam + graphicsState.Hoffset) + 1];
      }
      else {
        attributes = 0;
      }

      pixel = cc3Fontdata8x12[(character & 127) * 8 + (y % 8)];

      if ((attributes & 64) && (y % 8 == 7)) {	//UnderLine
        pixel = 255;
      }

      if (CheckState(attributes)) {
        pixel = 0;
      }

      textPallete[1] = Pallete32Bit[8 + ((attributes & 56) >> 3)];
      textPallete[0] = Pallete32Bit[attributes & 7];
      szSurface32[yStride += 1] = textPallete[(pixel & 128) / 128];
      szSurface32[yStride += 1] = textPallete[(pixel & 64) / 64];
      szSurface32[yStride += 1] = textPallete[(pixel & 32) / 32];
      szSurface32[yStride += 1] = textPallete[(pixel & 16) / 16];
      szSurface32[yStride += 1] = textPallete[(pixel & 8) / 8];
      szSurface32[yStride += 1] = textPallete[(pixel & 4) / 4];
      szSurface32[yStride += 1] = textPallete[(pixel & 2) / 2];
      szSurface32[yStride += 1] = textPallete[(pixel & 1)];
    }
    break;

    //******************************************************************** Low Res Text
  case 64:		//Low Res text GraphicsMode=0 CompatMode=1 Ignore Bpp and Stretch
  case 65:
  case 66:
  case 67:
  case 68:
  case 69:
  case 70:
  case 71:
  case 72:
  case 73:
  case 74:
  case 75:
  case 76:
  case 77:
  case 78:
  case 79:
  case 80:
  case 81:
  case 82:
  case 83:
  case 84:
  case 85:
  case 86:
  case 87:
  case 88:
  case 89:
  case 90:
  case 91:
  case 92:
  case 93:
  case 94:
  case 95:
  case 96:
  case 97:
  case 98:
  case 99:
  case 100:
  case 101:
  case 102:
  case 103:
  case 104:
  case 105:
  case 106:
  case 107:
  case 108:
  case 109:
  case 110:
  case 111:
  case 112:
  case 113:
  case 114:
  case 115:
  case 116:
  case 117:
  case 118:
  case 119:
  case 120:
  case 121:
  case 122:
  case 123:
  case 124:
  case 125:
  case 126:
  case 127:
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam++)
    {
      character = buffer[start + (unsigned char)(beam + graphicsState.Hoffset)];

      switch ((character & 192) >> 6)
      {
      case 0:
        character = character & 63;
        textPallete[0] = Pallete32Bit[graphicsState.TextBGPallete];
        textPallete[1] = Pallete32Bit[graphicsState.TextFGPallete];

        if (graphicsState.LowerCase & (character < 32))
          pixel = ntsc_round_fontdata8x12[(character + 80) * 12 + (y % 12)];
        else
          pixel = ~ntsc_round_fontdata8x12[(character) * 12 + (y % 12)];
        break;

      case 1:
        character = character & 63;
        textPallete[0] = Pallete32Bit[graphicsState.TextBGPallete];
        textPallete[1] = Pallete32Bit[graphicsState.TextFGPallete];
        pixel = ntsc_round_fontdata8x12[(character) * 12 + (y % 12)];
        break;

      case 2:
      case 3:
        textPallete[1] = Pallete32Bit[(character & 112) >> 4];
        textPallete[0] = Pallete32Bit[8];
        character = 64 + (character & 0xF);
        pixel = ntsc_round_fontdata8x12[(character) * 12 + (y % 12)];
        break;
      }

      szSurface32[yStride += 1] = textPallete[(pixel >> 7)];
      szSurface32[yStride += 1] = textPallete[(pixel >> 7)];
      szSurface32[yStride += 1] = textPallete[(pixel >> 6) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 6) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 5) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 5) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 4) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 4) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 3) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 3) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 2) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 2) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 1) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel >> 1) & 1];
      szSurface32[yStride += 1] = textPallete[(pixel & 1)];
      szSurface32[yStride += 1] = textPallete[(pixel & 1)];

      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += Xpitch;
        szSurface32[yStride += 1] = textPallete[(pixel >> 7)];
        szSurface32[yStride += 1] = textPallete[(pixel >> 7)];
        szSurface32[yStride += 1] = textPallete[(pixel >> 6) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 6) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 5) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 5) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 4) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 4) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 3) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 3) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 2) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 2) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 1) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel >> 1) & 1];
        szSurface32[yStride += 1] = textPallete[(pixel & 1)];
        szSurface32[yStride += 1] = textPallete[(pixel & 1)];
        yStride -= Xpitch;
      }
    }

    break;

  case 128 + 0: //Bpp=0 Sr=0 1BPP Stretch=1
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=1
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
        yStride -= Xpitch;
      }
    }
    break;

    //case 192+1:
  case 128 + 1: //Bpp=0 Sr=1 1BPP Stretch=2
  case 128 + 2:	//Bpp=0 Sr=2 
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=2
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];

      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (32);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
        yStride -= Xpitch;
      }
    }
    break;

  case 128 + 3: //Bpp=0 Sr=3 1BPP Stretch=4
  case 128 + 4: //Bpp=0 Sr=4
  case 128 + 5: //Bpp=0 Sr=5
  case 128 + 6: //Bpp=0 Sr=6
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=4
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];

      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (64);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
        yStride -= Xpitch;
      }
    }
    break;

  case 128 + 7: //Bpp=0 Sr=7 1BPP Stretch=8 
  case 128 + 8: //Bpp=0 Sr=8
  case 128 + 9: //Bpp=0 Sr=9
  case 128 + 10: //Bpp=0 Sr=10
  case 128 + 11: //Bpp=0 Sr=11
  case 128 + 12: //Bpp=0 Sr=12
  case 128 + 13: //Bpp=0 Sr=13
  case 128 + 14: //Bpp=0 Sr=14
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=8
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (128);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[1 & (WidePixel >> 8)];
        yStride -= Xpitch;
      }
    }
    break;

  case 128 + 15: //Bpp=0 Sr=15 1BPP Stretch=16
  case 128 + 16: //BPP=1 Sr=0  2BPP Stretch=1
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=1
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (8);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        yStride -= Xpitch;
      }
    }
    break;

  case 128 + 17: //Bpp=1 Sr=1  2BPP Stretch=2
  case 128 + 18: //Bpp=1 Sr=2
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=2
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        yStride -= Xpitch;
      }
    }
    break;

  case 128 + 19: //Bpp=1 Sr=3  2BPP Stretch=4
  case 128 + 20: //Bpp=1 Sr=4
  case 128 + 21: //Bpp=1 Sr=5
  case 128 + 22: //Bpp=1 Sr=6
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=4
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (32);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        yStride -= Xpitch;
      }
    }
    break;

  case 128 + 23: //Bpp=1 Sr=7  2BPP Stretch=8
  case 128 + 24: //Bpp=1 Sr=8
  case 128 + 25: //Bpp=1 Sr=9 
  case 128 + 26: //Bpp=1 Sr=10 
  case 128 + 27: //Bpp=1 Sr=11 
  case 128 + 28: //Bpp=1 Sr=12 
  case 128 + 29: //Bpp=1 Sr=13 
  case 128 + 30: //Bpp=1 Sr=14
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=8
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (64);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        yStride -= Xpitch;
      }
    }
    break;

  case 128 + 31: //Bpp=1 Sr=15 2BPP Stretch=16 
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=16
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (128);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[3 & (WidePixel >> 8)];
        yStride -= Xpitch;
      }
    }
    break;

  case 128 + 32: //Bpp=2 Sr=0 4BPP Stretch=1
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //4bbp Stretch=1
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (4);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        yStride -= Xpitch;
      }
    }
    break;

  case 128 + 33: //Bpp=2 Sr=1 4BPP Stretch=2 
  case 128 + 34: //Bpp=2 Sr=2
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //4bbp Stretch=2
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (8);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        yStride -= Xpitch;
      }
    }
    break;

  case 128 + 35: //Bpp=2 Sr=3 4BPP Stretch=4
  case 128 + 36: //Bpp=2 Sr=4 
  case 128 + 37: //Bpp=2 Sr=5 
  case 128 + 38: //Bpp=2 Sr=6 
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //4bbp Stretch=4
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        yStride -= Xpitch;
      }
    }
    break;

  case 128 + 39: //Bpp=2 Sr=7 4BPP Stretch=8
  case 128 + 40: //Bpp=2 Sr=8 
  case 128 + 41: //Bpp=2 Sr=9 
  case 128 + 42: //Bpp=2 Sr=10 
  case 128 + 43: //Bpp=2 Sr=11 
  case 128 + 44: //Bpp=2 Sr=12 
  case 128 + 45: //Bpp=2 Sr=13 
  case 128 + 46: //Bpp=2 Sr=14 
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //4bbp Stretch=8
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (32);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        yStride -= Xpitch;
      }
    }
    break;

  case 128 + 47: //Bpp=2 Sr=15 4BPP Stretch=16
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //4bbp Stretch=16
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
      szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];

      if (!systemState->ScanLines)
      {
        yStride -= (64);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = Pallete32Bit[15 & (WidePixel >> 8)];
        yStride -= Xpitch;
      }
    }
    break;

  case 128 + 48: //Bpp=3 Sr=0  Unsupported 
  case 128 + 49: //Bpp=3 Sr=1 
  case 128 + 50: //Bpp=3 Sr=2 
  case 128 + 51: //Bpp=3 Sr=3 
  case 128 + 52: //Bpp=3 Sr=4 
  case 128 + 53: //Bpp=3 Sr=5 
  case 128 + 54: //Bpp=3 Sr=6 
  case 128 + 55: //Bpp=3 Sr=7 
  case 128 + 56: //Bpp=3 Sr=8 
  case 128 + 57: //Bpp=3 Sr=9 
  case 128 + 58: //Bpp=3 Sr=10 
  case 128 + 59: //Bpp=3 Sr=11 
  case 128 + 60: //Bpp=3 Sr=12 
  case 128 + 61: //Bpp=3 Sr=13 
  case 128 + 62: //Bpp=3 Sr=14 
  case 128 + 63: //Bpp=3 Sr=15 
    return;
    break;

  case 192 + 0: //Bpp=0 Sr=0 1BPP Stretch=1
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=1
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        yStride -= Xpitch;
      }
    }
    break;

  case 192 + 1: //Bpp=0 Sr=1 1BPP Stretch=2
  case 192 + 2:	//Bpp=0 Sr=2 
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=2
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];

      if (!graphicsState.MonType)
      { //Pcolor
        for (bit = 7; bit >= 0; bit--)
        {
          pix = (1 & (WidePixel >> bit));
          phase = (carry2 << 2) | (carry1 << 1) | pix;

          switch (phase)
          {
          case 0:
          case 4:
          case 6:
            color = 0;
            break;

          case 1:
          case 5:
            color = (bit & 1) + 1;
            break;

          case 2:
            break;

          case 3:
            color = 3;
            szSurface32[yStride - 1] = Afacts32[graphicsState.ColorInvert][3];

            if (!systemState->ScanLines) {
              szSurface32[yStride + Xpitch - 1] = Afacts32[graphicsState.ColorInvert][3];
            }

            szSurface32[yStride] = Afacts32[graphicsState.ColorInvert][3];

            if (!systemState->ScanLines) {
              szSurface32[yStride + Xpitch] = Afacts32[graphicsState.ColorInvert][3];
            }

            break;

          case 7:
            color = 3;
            break;
          }

          szSurface32[yStride += 1] = Afacts32[graphicsState.ColorInvert][color];

          if (!systemState->ScanLines) {
            szSurface32[yStride + Xpitch] = Afacts32[graphicsState.ColorInvert][color];
          }

          szSurface32[yStride += 1] = Afacts32[graphicsState.ColorInvert][color];

          if (!systemState->ScanLines) {
            szSurface32[yStride + Xpitch] = Afacts32[graphicsState.ColorInvert][color];
          }

          carry2 = carry1;
          carry1 = pix;
        }

        for (bit = 15; bit >= 8; bit--)
        {
          pix = (1 & (WidePixel >> bit));
          phase = (carry2 << 2) | (carry1 << 1) | pix;

          switch (phase)
          {
          case 0:
          case 4:
          case 6:
            color = 0;
            break;

          case 1:
          case 5:
            color = (bit & 1) + 1;
            break;

          case 2:
            break;

          case 3:
            color = 3;
            szSurface32[yStride - 1] = Afacts32[graphicsState.ColorInvert][3];

            if (!systemState->ScanLines) {
              szSurface32[yStride + Xpitch - 1] = Afacts32[graphicsState.ColorInvert][3];
            }

            szSurface32[yStride] = Afacts32[graphicsState.ColorInvert][3];

            if (!systemState->ScanLines) {
              szSurface32[yStride + Xpitch] = Afacts32[graphicsState.ColorInvert][3];
            }

            break;

          case 7:
            color = 3;
            break;
          }

          szSurface32[yStride += 1] = Afacts32[graphicsState.ColorInvert][color];

          if (!systemState->ScanLines) {
            szSurface32[yStride + Xpitch] = Afacts32[graphicsState.ColorInvert][color];
          }

          szSurface32[yStride += 1] = Afacts32[graphicsState.ColorInvert][color];

          if (!systemState->ScanLines) {
            szSurface32[yStride + Xpitch] = Afacts32[graphicsState.ColorInvert][color];
          }

          carry2 = carry1;
          carry1 = pix;
        }
      }
      else
      {
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];

        if (!systemState->ScanLines)
        {
          yStride -= (32);
          yStride += Xpitch;
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
          yStride -= Xpitch;
        }
      }
    }
    break;

  case 192 + 3: //Bpp=0 Sr=3 1BPP Stretch=4
  case 192 + 4: //Bpp=0 Sr=4
  case 192 + 5: //Bpp=0 Sr=5
  case 192 + 6: //Bpp=0 Sr=6
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=4
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];

      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (64);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        yStride -= Xpitch;
      }
    }
    break;

  case 192 + 7: //Bpp=0 Sr=7 1BPP Stretch=8 
  case 192 + 8: //Bpp=0 Sr=8
  case 192 + 9: //Bpp=0 Sr=9
  case 192 + 10: //Bpp=0 Sr=10
  case 192 + 11: //Bpp=0 Sr=11
  case 192 + 12: //Bpp=0 Sr=12
  case 192 + 13: //Bpp=0 Sr=13
  case 192 + 14: //Bpp=0 Sr=14
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //1bbp Stretch=8
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (128);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (1 & (WidePixel >> 8))];
        yStride -= Xpitch;
      }
    }
    break;

  case 192 + 15: //Bpp=0 Sr=15 1BPP Stretch=16
  case 192 + 16: //BPP=1 Sr=0  2BPP Stretch=1
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=1
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (8);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        yStride -= Xpitch;
      }
    }
    break;

  case 192 + 17: //Bpp=1 Sr=1  2BPP Stretch=2
  case 192 + 18: //Bpp=1 Sr=2
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=2
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (16);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        yStride -= Xpitch;
      }
    }
    break;

  case 192 + 19: //Bpp=1 Sr=3  2BPP Stretch=4
  case 192 + 20: //Bpp=1 Sr=4
  case 192 + 21: //Bpp=1 Sr=5
  case 192 + 22: //Bpp=1 Sr=6
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=4
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (32);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        yStride -= Xpitch;
      }
    }
    break;

  case 192 + 23: //Bpp=1 Sr=7  2BPP Stretch=8
  case 192 + 24: //Bpp=1 Sr=8
  case 192 + 25: //Bpp=1 Sr=9 
  case 192 + 26: //Bpp=1 Sr=10 
  case 192 + 27: //Bpp=1 Sr=11 
  case 192 + 28: //Bpp=1 Sr=12 
  case 192 + 29: //Bpp=1 Sr=13 
  case 192 + 30: //Bpp=1 Sr=14
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=8
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (64);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        yStride -= Xpitch;
      }
    }
    break;

  case 192 + 31: //Bpp=1 Sr=15 2BPP Stretch=16 
    for (unsigned short beam = 0; beam < graphicsState.BytesperRow; beam += 2) //2bbp Stretch=16
    {
      WidePixel = WideBuffer[(graphicsState.VidMask & (start + (unsigned char)(graphicsState.Hoffset + beam))) >> 1];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
      szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];

      if (!systemState->ScanLines)
      {
        yStride -= (128);
        yStride += Xpitch;
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = Pallete32Bit[graphicsState.PalleteIndex + (3 & (WidePixel >> 8))];
        yStride -= Xpitch;
      }
    }
    break;

  case 192 + 32: //Bpp=2 Sr=0 4BPP Stretch=1 Unsupport with Compat set
  case 192 + 33: //Bpp=2 Sr=1 4BPP Stretch=2 
  case 192 + 34: //Bpp=2 Sr=2
  case 192 + 35: //Bpp=2 Sr=3 4BPP Stretch=4
  case 192 + 36: //Bpp=2 Sr=4 
  case 192 + 37: //Bpp=2 Sr=5 
  case 192 + 38: //Bpp=2 Sr=6 
  case 192 + 39: //Bpp=2 Sr=7 4BPP Stretch=8
  case 192 + 40: //Bpp=2 Sr=8 
  case 192 + 41: //Bpp=2 Sr=9 
  case 192 + 42: //Bpp=2 Sr=10 
  case 192 + 43: //Bpp=2 Sr=11 
  case 192 + 44: //Bpp=2 Sr=12 
  case 192 + 45: //Bpp=2 Sr=13 
  case 192 + 46: //Bpp=2 Sr=14 
  case 192 + 47: //Bpp=2 Sr=15 4BPP Stretch=16
  case 192 + 48: //Bpp=3 Sr=0  Unsupported 
  case 192 + 49: //Bpp=3 Sr=1 
  case 192 + 50: //Bpp=3 Sr=2 
  case 192 + 51: //Bpp=3 Sr=3 
  case 192 + 52: //Bpp=3 Sr=4 
  case 192 + 53: //Bpp=3 Sr=5 
  case 192 + 54: //Bpp=3 Sr=6 
  case 192 + 55: //Bpp=3 Sr=7 
  case 192 + 56: //Bpp=3 Sr=8 
  case 192 + 57: //Bpp=3 Sr=9 
  case 192 + 58: //Bpp=3 Sr=10 
  case 192 + 59: //Bpp=3 Sr=11 
  case 192 + 60: //Bpp=3 Sr=12 
  case 192 + 61: //Bpp=3 Sr=13 
  case 192 + 62: //Bpp=3 Sr=14 
  case 192 + 63: //Bpp=3 Sr=15 
    return;
    break;
  }
}

void DrawTopBoarder8(SystemState* systemState)
{
  if (graphicsState.BoarderChange == 0) {
    return;
  }

  for (unsigned short x = 0; x < systemState->WindowSize.x; x++)
  {
    systemState->PTRsurface8[x + ((systemState->LineCounter * 2) * systemState->SurfacePitch)] = graphicsState.BoarderColor8 | 128;

    if (!systemState->ScanLines) {
      systemState->PTRsurface8[x + ((systemState->LineCounter * 2 + 1) * systemState->SurfacePitch)] = graphicsState.BoarderColor8 | 128;
    }
  }
}

void DrawTopBoarder16(SystemState* systemState)
{
  if (graphicsState.BoarderChange == 0) {
    return;
  }

  for (unsigned short x = 0; x < systemState->WindowSize.x; x++)
  {
    systemState->PTRsurface16[x + ((systemState->LineCounter * 2) * systemState->SurfacePitch)] = graphicsState.BoarderColor16;

    if (!systemState->ScanLines) {
      systemState->PTRsurface16[x + ((systemState->LineCounter * 2 + 1) * systemState->SurfacePitch)] = graphicsState.BoarderColor16;
    }
  }
}

void DrawTopBoarder24(SystemState* systemState)
{
}

void DrawTopBoarder32(SystemState* systemState)
{
  if (graphicsState.BoarderChange == 0) {
    return;
  }

  for (unsigned short x = 0; x < systemState->WindowSize.x; x++)
  {
    systemState->PTRsurface32[x + ((systemState->LineCounter * 2) * systemState->SurfacePitch)] = graphicsState.BoarderColor32;

    if (!systemState->ScanLines) {
      systemState->PTRsurface32[x + ((systemState->LineCounter * 2 + 1) * systemState->SurfacePitch)] = graphicsState.BoarderColor32;
    }
  }
}

void DrawBottomBoarder8(SystemState* systemState)
{
  if (graphicsState.BoarderChange == 0) {
    return;
  }

  for (unsigned short x = 0; x < systemState->WindowSize.x; x++)
  {
    systemState->PTRsurface8[x + (2 * (systemState->LineCounter + graphicsState.LinesperScreen + graphicsState.VertCenter) * systemState->SurfacePitch)] = graphicsState.BoarderColor8 | 128;

    if (!systemState->ScanLines) {
      systemState->PTRsurface8[x + systemState->SurfacePitch + (2 * (systemState->LineCounter + graphicsState.LinesperScreen + graphicsState.VertCenter) * systemState->SurfacePitch)] = graphicsState.BoarderColor8 | 128;
    }
  }
}

void DrawBottomBoarder16(SystemState* systemState)
{
  if (graphicsState.BoarderChange == 0) {
    return;
  }

  for (unsigned short x = 0; x < systemState->WindowSize.x; x++)
  {
    systemState->PTRsurface16[x + (2 * (systemState->LineCounter + graphicsState.LinesperScreen + graphicsState.VertCenter) * systemState->SurfacePitch)] = graphicsState.BoarderColor16;

    if (!systemState->ScanLines) {
      systemState->PTRsurface16[x + systemState->SurfacePitch + (2 * (systemState->LineCounter + graphicsState.LinesperScreen + graphicsState.VertCenter) * systemState->SurfacePitch)] = graphicsState.BoarderColor16;
    }
  }
}

void DrawBottomBoarder24(SystemState* systemState)
{
}

void DrawBottomBoarder32(SystemState* systemState)
{
  if (graphicsState.BoarderChange == 0) {
    return;
  }

  for (unsigned short x = 0; x < systemState->WindowSize.x; x++)
  {
    systemState->PTRsurface32[x + (2 * (systemState->LineCounter + graphicsState.LinesperScreen + graphicsState.VertCenter) * systemState->SurfacePitch)] = graphicsState.BoarderColor32;

    if (!systemState->ScanLines) {
      systemState->PTRsurface32[x + systemState->SurfacePitch + (2 * (systemState->LineCounter + graphicsState.LinesperScreen + graphicsState.VertCenter) * systemState->SurfacePitch)] = graphicsState.BoarderColor32;
    }
  }
}

// These grab the Video info for all COCO 2 modes
void SetGimeVdgOffset(unsigned char offset)
{
  if (graphicsState.CC2Offset != offset)
  {
    graphicsState.CC2Offset = offset;
    SetupDisplay();
  }
}

void SetGimeVdgMode(unsigned char vdgMode) //3 bits from SAM Registers
{
  if (graphicsState.CC2VDGMode != vdgMode)
  {
    graphicsState.CC2VDGMode = vdgMode;
    SetupDisplay();
    graphicsState.BoarderChange = 3;
  }
}

void SetGimeVdgMode2(unsigned char vdgmode2) //5 bits from PIA Register
{
  if (graphicsState.CC2VDGPiaMode != vdgmode2)
  {
    graphicsState.CC2VDGPiaMode = vdgmode2;
    SetupDisplay();
    graphicsState.BoarderChange = 3;
  }
}

//These grab the Video info for all COCO 3 modes

void SetVerticalOffsetRegister(unsigned short voRegister)
{
  if (graphicsState.VerticalOffsetRegister != voRegister)
  {
    graphicsState.VerticalOffsetRegister = voRegister;

    SetupDisplay();
  }
}

void SetCompatMode(unsigned char mode)
{
  if (graphicsState.CompatMode != mode)
  {
    graphicsState.CompatMode = mode;
    SetupDisplay();
    graphicsState.BoarderChange = 3;
  }
}

void SetGimePallet(unsigned char pallete, unsigned char color)
{
  // Convert the 6bit rgbrgb value to rrrrrggggggbbbbb for the Real video hardware.
  //	unsigned char r,g,b;
  Pallete[pallete] = ((color & 63));
  Pallete8Bit[pallete] = PalleteLookup8[graphicsState.MonType][color & 63];
  Pallete16Bit[pallete] = PalleteLookup16[graphicsState.MonType][color & 63];
  Pallete32Bit[pallete] = PalleteLookup32[graphicsState.MonType][color & 63];
}

void SetGimeVmode(unsigned char vmode)
{
  if (graphicsState.CC3Vmode != vmode)
  {
    graphicsState.CC3Vmode = vmode;
    SetupDisplay();
    graphicsState.BoarderChange = 3;
  }
}

void SetGimeVres(unsigned char vres)
{
  if (graphicsState.CC3Vres != vres)
  {
    graphicsState.CC3Vres = vres;
    SetupDisplay();
    graphicsState.BoarderChange = 3;
  }
}

void SetGimeHorzOffset(unsigned char data)
{
  if (graphicsState.HorzOffsetReg != data)
  {
    graphicsState.Hoffset = (data << 1);
    graphicsState.HorzOffsetReg = data;
    SetupDisplay();
  }
}

void SetGimeBoarderColor(unsigned char data)
{
  if (graphicsState.CC3BoarderColor != (data & 63))
  {
    graphicsState.CC3BoarderColor = data & 63;
    SetupDisplay();
    graphicsState.BoarderChange = 3;
  }
}

void SetBoarderChange(unsigned char data)
{
  if (graphicsState.BoarderChange > 0) {
    graphicsState.BoarderChange--;
  }
}

void InvalidateBoarder(void)
{
  graphicsState.BoarderChange = 5;
}

void SetupDisplay(void)
{
  static unsigned char CC2Bpp[8] = { 1,0,1,0,1,0,1,0 };
  static unsigned char CC2LinesperRow[8] = { 12,3,3,2,2,1,1,1 };
  static unsigned char CC3LinesperRow[8] = { 1,1,2,8,9,10,11,200 };
  static unsigned char CC2BytesperRow[8] = { 16,16,32,16,32,16,32,32 };
  static unsigned char CC3BytesperRow[8] = { 16,20,32,40,64,80,128,160 };
  static unsigned char CC3BytesperTextRow[8] = { 32,40,32,40,64,80,64,80 };
  static unsigned char CC2PaletteSet[4] = { 8,0,10,4 };
  static unsigned char CCPixelsperByte[4] = { 8,4,2,2 };
  static unsigned char ColorSet = 0, Temp1;

  graphicsState.ExtendedText = 1;

  switch (graphicsState.CompatMode)
  {
  case 0:		//Color Computer 3 Mode
    graphicsState.NewStartofVidram = graphicsState.VerticalOffsetRegister * 8;
    graphicsState.GraphicsMode = (graphicsState.CC3Vmode & 128) >> 7;
    graphicsState.VresIndex = (graphicsState.CC3Vres & 96) >> 5;
    CC3LinesperRow[7] = graphicsState.LinesperScreen;	// For 1 pixel high modes
    graphicsState.Bpp = graphicsState.CC3Vres & 3;
    graphicsState.LinesperRow = CC3LinesperRow[graphicsState.CC3Vmode & 7];
    graphicsState.BytesperRow = CC3BytesperRow[(graphicsState.CC3Vres & 28) >> 2];
    graphicsState.PalleteIndex = 0;

    if (!graphicsState.GraphicsMode)
    {
      if (graphicsState.CC3Vres & 1) {
        graphicsState.ExtendedText = 2;
      }

      graphicsState.Bpp = 0;
      graphicsState.BytesperRow = CC3BytesperTextRow[(graphicsState.CC3Vres & 28) >> 2];
    }

    break;

  case 1:					//Color Computer 2 Mode
    graphicsState.CC3BoarderColor = 0;	//Black for text modes
    graphicsState.BoarderChange = 3;
    graphicsState.NewStartofVidram = (512 * graphicsState.CC2Offset) + (graphicsState.VerticalOffsetRegister & 0xE0FF) * 8;
    graphicsState.GraphicsMode = (graphicsState.CC2VDGPiaMode & 16) >> 4; //PIA Set on graphics clear on text
    graphicsState.VresIndex = 0;
    graphicsState.LinesperRow = CC2LinesperRow[graphicsState.CC2VDGMode];

    if (graphicsState.GraphicsMode)
    {
      ColorSet = (graphicsState.CC2VDGPiaMode & 1);

      if (ColorSet == 0) {
        graphicsState.CC3BoarderColor = 18; //18 Bright Green
      }
      else {
        graphicsState.CC3BoarderColor = 63; //63 White 
      }

      graphicsState.BoarderChange = 3;
      graphicsState.Bpp = CC2Bpp[(graphicsState.CC2VDGPiaMode & 15) >> 1];
      graphicsState.BytesperRow = CC2BytesperRow[(graphicsState.CC2VDGPiaMode & 15) >> 1];
      Temp1 = (graphicsState.CC2VDGPiaMode & 1) << 1 | (graphicsState.Bpp & 1);
      graphicsState.PalleteIndex = CC2PaletteSet[Temp1];
    }
    else
    {	//Setup for 32x16 text Mode
      graphicsState.Bpp = 0;
      graphicsState.BytesperRow = 32;
      graphicsState.InvertAll = (graphicsState.CC2VDGPiaMode & 4) >> 2;
      graphicsState.LowerCase = (graphicsState.CC2VDGPiaMode & 2) >> 1;
      ColorSet = (graphicsState.CC2VDGPiaMode & 1);
      Temp1 = ((ColorSet << 1) | graphicsState.InvertAll);

      switch (Temp1)
      {
      case 0:
        graphicsState.TextFGPallete = 12;
        graphicsState.TextBGPallete = 13;
        break;

      case 1:
        graphicsState.TextFGPallete = 13;
        graphicsState.TextBGPallete = 12;
        break;

      case 2:
        graphicsState.TextFGPallete = 14;
        graphicsState.TextBGPallete = 15;
        break;

      case 3:
        graphicsState.TextFGPallete = 15;
        graphicsState.TextBGPallete = 14;
        break;
      }
    }

    break;
  }

  graphicsState.ColorInvert = (graphicsState.CC3Vmode & 32) >> 5;
  graphicsState.LinesperScreen = Lpf[graphicsState.VresIndex];
  SetLinesperScreen(graphicsState.VresIndex);
  graphicsState.VertCenter = VcenterTable[graphicsState.VresIndex] - 4; //4 unrendered top lines
  graphicsState.PixelsperLine = graphicsState.BytesperRow * CCPixelsperByte[graphicsState.Bpp];

  if (graphicsState.PixelsperLine % 40)
  {
    graphicsState.Stretch = (512 / graphicsState.PixelsperLine) - 1;
    graphicsState.HorzCenter = 64;
  }
  else
  {
    graphicsState.Stretch = (640 / graphicsState.PixelsperLine) - 1;
    graphicsState.HorzCenter = 0;
  }

  graphicsState.VPitch = graphicsState.BytesperRow;

  if (graphicsState.HorzOffsetReg & 128) {
    graphicsState.VPitch = 256;
  }

  graphicsState.BoarderColor8 = ((graphicsState.CC3BoarderColor & 63) | 128);
  graphicsState.BoarderColor16 = PalleteLookup16[graphicsState.MonType][graphicsState.CC3BoarderColor & 63];
  graphicsState.BoarderColor32 = PalleteLookup32[graphicsState.MonType][graphicsState.CC3BoarderColor & 63];
  graphicsState.NewStartofVidram = (graphicsState.NewStartofVidram & graphicsState.VidMask) + graphicsState.DistoOffset; //DistoOffset for 2M configuration
  graphicsState.MasterMode = (graphicsState.GraphicsMode << 7) | (graphicsState.CompatMode << 6) | ((graphicsState.Bpp & 3) << 4) | (graphicsState.Stretch & 15);
}

void GimeInit(void)
{
}

void GimeReset(void)
{
  graphicsState.CC3Vmode = 0;
  graphicsState.CC3Vres = 0;
  graphicsState.StartofVidram = 0;
  graphicsState.NewStartofVidram = 0;
  graphicsState.GraphicsMode = 0;
  graphicsState.LowerCase = 0;
  graphicsState.InvertAll = 0;
  graphicsState.ExtendedText = 1;
  graphicsState.HorzOffsetReg = 0;
  graphicsState.TagY = 0;
  graphicsState.DistoOffset = 0;
  MakeRGBPalette();
  MakeCMPpalette();
  graphicsState.BoarderChange = 3;
  graphicsState.CC2Offset = 0;
  graphicsState.Hoffset = 0;
  graphicsState.VerticalOffsetRegister = 0;
  MiscReset();
}

void SetVidMask(unsigned int data)
{
  graphicsState.VidMask = data;
}

void SetVideoBank(unsigned char data)
{
  graphicsState.DistoOffset = data * (512 * 1024);
  SetupDisplay();
}

void MakeRGBPalette(void)
{
  unsigned char r, g, b;

  for (unsigned char index = 0; index < 64; index++)
  {
    PalleteLookup8[1][index] = index | 128;

    r = ColorTable16Bit[(index & 32) >> 4 | (index & 4) >> 2];
    g = ColorTable16Bit[(index & 16) >> 3 | (index & 2) >> 1];
    b = ColorTable16Bit[(index & 8) >> 2 | (index & 1)];
    PalleteLookup16[1][index] = (r << 11) | (g << 6) | b;

    //32BIT
    r = ColorTable32Bit[(index & 32) >> 4 | (index & 4) >> 2];
    g = ColorTable32Bit[(index & 16) >> 3 | (index & 2) >> 1];
    b = ColorTable32Bit[(index & 8) >> 2 | (index & 1)];
    PalleteLookup32[1][index] = (r * 65536) + (g * 256) + b;
  }
}

void MakeCMPpalette(void)
{
  double saturation, brightness, contrast;
  int offset;
  double w;
  double r, g, b;

  int PaletteType = GetPaletteType();

  unsigned char rr, gg, bb;
  
  int red[] = {
    0,14,12,21,51,86,108,118,
    113,92,61,21,1,5,12,13,
    50,29,49,86,119,158,179,192,
    186,165,133,94,23,16,23,25,
    116,74,102,142,179,219,243,252,
    251,230,198,155,81,61,52,57,
    253,137,161,189,215,240,253,253,
    251,237,214,183,134,121,116,255
  };
  int green[] = {
    0,78,69,53,33,4,1,1,
    12,24,31,35,37,51,67,77,
    50,149,141,123,103,77,55,39,
    35,43,53,63,100,119,137,148,
    116,212,204,186,164,137,114,97,
    88,89,96,109,156,179,199,211,
    253,230,221,207,192,174,158,148,
    143,144,150,162,196,212,225,255
  };
  int blue[] = {
    0,20,18,14,10,10,12,19,
    76,135,178,196,148,97,29,20,
    50,38,36,32,28,25,24,78,
    143,207,248,249,228,174,99,46,
    116,58,52,48,44,41,68,132,
    202,250,250,250,251,243,163,99,
    254,104,83,77,82,105,142,188,
    237,251,251,251,252,240,183,255
  };

  float gamma = 1.4f;

  if (PaletteType == 1) {
    OutputDebugString("Loading new CMP palette.\n");
  }
  else {
    OutputDebugString("Loading old CMP palette.\n");
  }

  for (unsigned char index = 0; index <= 63; index++)
  {
    if (PaletteType == 1)
    {
      if (index > 39) { gamma = 1.1f; }

      if (index > 55) { gamma = 1; }

      r = red[index] * (double)gamma; if (r > 255) { r = 255; }
      g = green[index] * (double)gamma; if (g > 255) { g = 255; }
      b = blue[index] * (double)gamma; if (b > 255) { b = 255; }
    }
    else {  //Old palette //Stolen from M.E.S.S.
      switch (index)
      {
      case 0:
        r = g = b = 0;
        break;

      case 16:
        r = g = b = 47;
        break;

      case 32:
        r = g = b = 120;
        break;

      case 48:
      case 63:
        r = g = b = 255;
        break;

      default:
        w = .4195456981879 * 1.01;
        contrast = 70;
        saturation = 92;
        brightness = -20;
        brightness += ((index / 16) + (double)1) * contrast;
        offset = (index % 16) - 1 + (index / 16) * 15;
        r = cos(w * (offset + 9.2)) * saturation + brightness;
        g = cos(w * (offset + 14.2)) * saturation + brightness;
        b = cos(w * (offset + 19.2)) * saturation + brightness;

        if (r < 0) {
          r = 0;
        }
        else if (r > 255) {
          r = 255;
        }

        if (g < 0) {
          g = 0;
        }
        else if (g > 255) {
          g = 255;
        }

        if (b < 0) {
          b = 0;
        }
        else if (b > 255) {
          b = 255;
        }

        break;
      }
    }

    rr = (unsigned char)r;
    gg = (unsigned char)g;
    bb = (unsigned char)b;
    PalleteLookup32[0][index] = (rr << 16) | (gg << 8) | bb;

    rr = rr >> 3;
    gg = gg >> 3;
    bb = bb >> 3;
    PalleteLookup16[0][index] = (rr << 11) | (gg << 6) | bb;

    rr = rr >> 3;
    gg = gg >> 3;
    bb = bb >> 3;
    PalleteLookup8[0][index] = 0x80 | ((rr & 2) << 4) | ((gg & 2) << 3) | ((bb & 2) << 2) | ((rr & 1) << 2) | ((gg & 1) << 1) | (bb & 1);
  }
}

unsigned char SetMonitorType(unsigned char type)
{
  int borderColor = graphicsState.CC3BoarderColor;
  SetGimeBoarderColor(0);

  if (type != QUERY)
  {
    graphicsState.MonType = type & 1;

    for (unsigned char palIndex = 0; palIndex < 16; palIndex++)
    {
      Pallete16Bit[palIndex] = PalleteLookup16[graphicsState.MonType][Pallete[palIndex]];
      Pallete32Bit[palIndex] = PalleteLookup32[graphicsState.MonType][Pallete[palIndex]];
      Pallete8Bit[palIndex] = PalleteLookup8[graphicsState.MonType][Pallete[palIndex]];
    }
  }

  SetGimeBoarderColor(borderColor);

  return(graphicsState.MonType);
}

void SetPaletteType() {
  int borderColor = graphicsState.CC3BoarderColor;
  SetGimeBoarderColor(0);
  MakeCMPpalette();
  SetGimeBoarderColor(borderColor);
}

unsigned char SetScanLines(unsigned char lines)
{
  extern SystemState EmuState;

  if (lines != QUERY)
  {
    EmuState.ScanLines = lines;
    Cls(0, &EmuState);
    graphicsState.BoarderChange = 3;
  }

  return(0);
}

int GetBytesPerRow() {
  return graphicsState.BytesperRow;
}

unsigned int GetStartOfVidram() {
  return graphicsState.StartofVidram;
}

int GetGraphicsMode() {
  return(graphicsState.GraphicsMode);
}

void FlipArtifacts() {
  graphicsState.ColorInvert = graphicsState.ColorInvert == 0 ? 1 : 0;
}

unsigned char GetLpf(unsigned char index) {
  return Lpf[index];
}

unsigned char GetVcenterTable(unsigned char index) {
  return VcenterTable[index];
}

void SetBlinkState(unsigned char blinkState)
{
  graphicsState.BlinkState = blinkState;
}

unsigned char CheckState(unsigned char attributes) {
  return (!graphicsState.BlinkState) & !!(attributes & 128);
}
