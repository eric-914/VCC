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

#include "cc2font.h"
#include "cc3font.h"

#include "defines.h"
#include "Graphics.h"
#include "systemstate.h"

extern "C" {
  __declspec(dllexport)
    void __cdecl UpdateScreen32(SystemState* systemState)
  {
    register unsigned int yStride = 0;
    unsigned char pixel = 0;
    unsigned char character = 0, attributes = 0;
    unsigned int textPallete[2] = { 0,0 };
    unsigned short WidePixel = 0;
    char pix = 0, bit = 0, phase = 0;
    char carry1 = 1, carry2 = 0;
    char color = 0;
    unsigned int* szSurface32 = systemState->PTRsurface32;
    unsigned short y = systemState->LineCounter;
    long Xpitch = systemState->SurfacePitch;
    unsigned short* WideBuffer = (unsigned short*)systemState->RamBuffer;
    unsigned char* buffer = systemState->RamBuffer;

    GraphicsState* gs = GetGraphicsState();

    if ((gs->HorzCenter != 0) && (gs->BorderChange > 0)) {
      for (unsigned short x = 0; x < gs->HorzCenter; x++)
      {
        szSurface32[x + (((y + gs->VertCenter) * 2) * Xpitch)] = gs->BorderColor32;

        if (!systemState->ScanLines) {
          szSurface32[x + (((y + gs->VertCenter) * 2 + 1) * Xpitch)] = gs->BorderColor32;
        }

        szSurface32[x + (gs->PixelsperLine * (gs->Stretch + 1)) + gs->HorzCenter + (((y + gs->VertCenter) * 2) * Xpitch)] = gs->BorderColor32;

        if (!systemState->ScanLines) {
          szSurface32[x + (gs->PixelsperLine * (gs->Stretch + 1)) + gs->HorzCenter + (((y + gs->VertCenter) * 2 + 1) * Xpitch)] = gs->BorderColor32;
        }
      }
    }

    if (gs->LinesperRow < 13) {
      gs->TagY++;
    }

    if (!y)
    {
      gs->StartofVidram = gs->NewStartofVidram;
      gs->TagY = y;
    }

    unsigned int start = gs->StartofVidram + (gs->TagY / gs->LinesperRow) * (gs->VPitch * gs->ExtendedText);
    yStride = (((y + gs->VertCenter) * 2) * Xpitch) + (gs->HorzCenter * 1) - 1;

    switch (gs->MasterMode) // (GraphicsMode <<7) | (CompatMode<<6)  | ((Bpp & 3)<<4) | (Stretch & 15);
    {
    case 0: //Width 80
      attributes = 0;

      if (gs->HorzOffsetReg & 128) {
        start = gs->StartofVidram + (gs->TagY / gs->LinesperRow) * (gs->VPitch); //Fix for Horizontal Offset Register in text mode.
      }

      for (unsigned short beam = 0; beam < gs->BytesperRow * gs->ExtendedText; beam += gs->ExtendedText)
      {
        character = buffer[start + (unsigned char)(beam + gs->Hoffset)];
        pixel = cc3Fontdata8x12[character * 12 + (y % gs->LinesperRow)];

        if (gs->ExtendedText == 2)
        {
          attributes = buffer[start + (unsigned char)(beam + gs->Hoffset) + 1];

          if ((attributes & 64) && (y % gs->LinesperRow == (gs->LinesperRow - 1))) {	//UnderLine
            pixel = 255;
          }

          if (CheckState(attributes)) {
            pixel = 0;
          }
        }

        textPallete[1] = gs->Pallete32Bit[8 + ((attributes & 56) >> 3)];
        textPallete[0] = gs->Pallete32Bit[attributes & 7];
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

      for (unsigned short beam = 0; beam < gs->BytesperRow * gs->ExtendedText; beam += gs->ExtendedText)
      {
        character = buffer[start + (unsigned char)(beam + gs->Hoffset)];
        pixel = cc3Fontdata8x12[character * 12 + (y % gs->LinesperRow)];

        if (gs->ExtendedText == 2)
        {
          attributes = buffer[start + (unsigned char)(beam + gs->Hoffset) + 1];

          if ((attributes & 64) && (y % gs->LinesperRow == (gs->LinesperRow - 1))) {	//UnderLine
            pixel = 255;
          }

          if (CheckState(attributes)) {
            pixel = 0;
          }
        }

        textPallete[1] = gs->Pallete32Bit[8 + ((attributes & 56) >> 3)];
        textPallete[0] = gs->Pallete32Bit[attributes & 7];
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

      for (unsigned short beam = 0; beam < gs->BytesperRow * gs->ExtendedText; beam += gs->ExtendedText)
      {
        character = buffer[start + (unsigned char)(beam + gs->Hoffset)];

        if (gs->ExtendedText == 2) {
          attributes = buffer[start + (unsigned char)(beam + gs->Hoffset) + 1];
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

        textPallete[1] = gs->Pallete32Bit[8 + ((attributes & 56) >> 3)];
        textPallete[0] = gs->Pallete32Bit[attributes & 7];
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
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam++)
      {
        character = buffer[start + (unsigned char)(beam + gs->Hoffset)];

        switch ((character & 192) >> 6)
        {
        case 0:
          character = character & 63;
          textPallete[0] = gs->Pallete32Bit[gs->TextBGPallete];
          textPallete[1] = gs->Pallete32Bit[gs->TextFGPallete];

          if (gs->LowerCase & (character < 32))
            pixel = ntsc_round_fontdata8x12[(character + 80) * 12 + (y % 12)];
          else
            pixel = ~ntsc_round_fontdata8x12[(character) * 12 + (y % 12)];
          break;

        case 1:
          character = character & 63;
          textPallete[0] = gs->Pallete32Bit[gs->TextBGPallete];
          textPallete[1] = gs->Pallete32Bit[gs->TextFGPallete];
          pixel = ntsc_round_fontdata8x12[(character) * 12 + (y % 12)];
          break;

        case 2:
        case 3:
          textPallete[1] = gs->Pallete32Bit[(character & 112) >> 4];
          textPallete[0] = gs->Pallete32Bit[8];
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
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //1bbp Stretch=1
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];

        if (!systemState->ScanLines)
        {
          yStride -= (16);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
          yStride -= Xpitch;
        }
      }
      break;

      //case 192+1:
    case 128 + 1: //Bpp=0 Sr=1 1BPP Stretch=2
    case 128 + 2:	//Bpp=0 Sr=2 
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //1bbp Stretch=2
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];

        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];

        if (!systemState->ScanLines)
        {
          yStride -= (32);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
          yStride -= Xpitch;
        }
      }
      break;

    case 128 + 3: //Bpp=0 Sr=3 1BPP Stretch=4
    case 128 + 4: //Bpp=0 Sr=4
    case 128 + 5: //Bpp=0 Sr=5
    case 128 + 6: //Bpp=0 Sr=6
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //1bbp Stretch=4
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];

        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];

        if (!systemState->ScanLines)
        {
          yStride -= (64);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
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
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //1bbp Stretch=8
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];

        if (!systemState->ScanLines)
        {
          yStride -= (128);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 7)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 5)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 3)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 1)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 15)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 13)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 11)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 9)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[1 & (WidePixel >> 8)];
          yStride -= Xpitch;
        }
      }
      break;

    case 128 + 15: //Bpp=0 Sr=15 1BPP Stretch=16
    case 128 + 16: //BPP=1 Sr=0  2BPP Stretch=1
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //2bbp Stretch=1
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];

        if (!systemState->ScanLines)
        {
          yStride -= (8);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          yStride -= Xpitch;
        }
      }
      break;

    case 128 + 17: //Bpp=1 Sr=1  2BPP Stretch=2
    case 128 + 18: //Bpp=1 Sr=2
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //2bbp Stretch=2
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];

        if (!systemState->ScanLines)
        {
          yStride -= (16);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          yStride -= Xpitch;
        }
      }
      break;

    case 128 + 19: //Bpp=1 Sr=3  2BPP Stretch=4
    case 128 + 20: //Bpp=1 Sr=4
    case 128 + 21: //Bpp=1 Sr=5
    case 128 + 22: //Bpp=1 Sr=6
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //2bbp Stretch=4
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];

        if (!systemState->ScanLines)
        {
          yStride -= (32);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
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
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //2bbp Stretch=8
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];

        if (!systemState->ScanLines)
        {
          yStride -= (64);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          yStride -= Xpitch;
        }
      }
      break;

    case 128 + 31: //Bpp=1 Sr=15 2BPP Stretch=16 
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //2bbp Stretch=16
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];

        if (!systemState->ScanLines)
        {
          yStride -= (128);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 6)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 2)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 14)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 10)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[3 & (WidePixel >> 8)];
          yStride -= Xpitch;
        }
      }
      break;

    case 128 + 32: //Bpp=2 Sr=0 4BPP Stretch=1
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //4bbp Stretch=1
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];

        if (!systemState->ScanLines)
        {
          yStride -= (4);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          yStride -= Xpitch;
        }
      }
      break;

    case 128 + 33: //Bpp=2 Sr=1 4BPP Stretch=2 
    case 128 + 34: //Bpp=2 Sr=2
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //4bbp Stretch=2
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];

        if (!systemState->ScanLines)
        {
          yStride -= (8);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          yStride -= Xpitch;
        }
      }
      break;

    case 128 + 35: //Bpp=2 Sr=3 4BPP Stretch=4
    case 128 + 36: //Bpp=2 Sr=4 
    case 128 + 37: //Bpp=2 Sr=5 
    case 128 + 38: //Bpp=2 Sr=6 
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //4bbp Stretch=4
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];

        if (!systemState->ScanLines)
        {
          yStride -= (16);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
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
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //4bbp Stretch=8
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];

        if (!systemState->ScanLines)
        {
          yStride -= (32);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          yStride -= Xpitch;
        }
      }
      break;

    case 128 + 47: //Bpp=2 Sr=15 4BPP Stretch=16
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //4bbp Stretch=16
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];

        if (!systemState->ScanLines)
        {
          yStride -= (64);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 4)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & WidePixel];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 12)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[15 & (WidePixel >> 8)];
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
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //1bbp Stretch=1
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];

        if (!systemState->ScanLines)
        {
          yStride -= (16);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
          yStride -= Xpitch;
        }
      }
      break;

    case 192 + 1: //Bpp=0 Sr=1 1BPP Stretch=2
    case 192 + 2:	//Bpp=0 Sr=2 
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //1bbp Stretch=2
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];

        if (!gs->MonType)
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
              szSurface32[yStride - 1] = gs->Afacts32[gs->ColorInvert][3];

              if (!systemState->ScanLines) {
                szSurface32[yStride + Xpitch - 1] = gs->Afacts32[gs->ColorInvert][3];
              }

              szSurface32[yStride] = gs->Afacts32[gs->ColorInvert][3];

              if (!systemState->ScanLines) {
                szSurface32[yStride + Xpitch] = gs->Afacts32[gs->ColorInvert][3];
              }

              break;

            case 7:
              color = 3;
              break;
            }

            szSurface32[yStride += 1] = gs->Afacts32[gs->ColorInvert][color];

            if (!systemState->ScanLines) {
              szSurface32[yStride + Xpitch] = gs->Afacts32[gs->ColorInvert][color];
            }

            szSurface32[yStride += 1] = gs->Afacts32[gs->ColorInvert][color];

            if (!systemState->ScanLines) {
              szSurface32[yStride + Xpitch] = gs->Afacts32[gs->ColorInvert][color];
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
              szSurface32[yStride - 1] = gs->Afacts32[gs->ColorInvert][3];

              if (!systemState->ScanLines) {
                szSurface32[yStride + Xpitch - 1] = gs->Afacts32[gs->ColorInvert][3];
              }

              szSurface32[yStride] = gs->Afacts32[gs->ColorInvert][3];

              if (!systemState->ScanLines) {
                szSurface32[yStride + Xpitch] = gs->Afacts32[gs->ColorInvert][3];
              }

              break;

            case 7:
              color = 3;
              break;
            }

            szSurface32[yStride += 1] = gs->Afacts32[gs->ColorInvert][color];

            if (!systemState->ScanLines) {
              szSurface32[yStride + Xpitch] = gs->Afacts32[gs->ColorInvert][color];
            }

            szSurface32[yStride += 1] = gs->Afacts32[gs->ColorInvert][color];

            if (!systemState->ScanLines) {
              szSurface32[yStride + Xpitch] = gs->Afacts32[gs->ColorInvert][color];
            }

            carry2 = carry1;
            carry1 = pix;
          }
        }
        else
        {
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];

          if (!systemState->ScanLines)
          {
            yStride -= (32);
            yStride += Xpitch;
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
            szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
            yStride -= Xpitch;
          }
        }
      }
      break;

    case 192 + 3: //Bpp=0 Sr=3 1BPP Stretch=4
    case 192 + 4: //Bpp=0 Sr=4
    case 192 + 5: //Bpp=0 Sr=5
    case 192 + 6: //Bpp=0 Sr=6
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //1bbp Stretch=4
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];

        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];

        if (!systemState->ScanLines)
        {
          yStride -= (64);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
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
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //1bbp Stretch=8
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];

        if (!systemState->ScanLines)
        {
          yStride -= (128);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 7))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 5))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 3))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 1))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 15))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 13))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 11))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 9))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (1 & (WidePixel >> 8))];
          yStride -= Xpitch;
        }
      }
      break;

    case 192 + 15: //Bpp=0 Sr=15 1BPP Stretch=16
    case 192 + 16: //BPP=1 Sr=0  2BPP Stretch=1
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //2bbp Stretch=1
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];

        if (!systemState->ScanLines)
        {
          yStride -= (8);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          yStride -= Xpitch;
        }
      }
      break;

    case 192 + 17: //Bpp=1 Sr=1  2BPP Stretch=2
    case 192 + 18: //Bpp=1 Sr=2
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //2bbp Stretch=2
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];

        if (!systemState->ScanLines)
        {
          yStride -= (16);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          yStride -= Xpitch;
        }
      }
      break;

    case 192 + 19: //Bpp=1 Sr=3  2BPP Stretch=4
    case 192 + 20: //Bpp=1 Sr=4
    case 192 + 21: //Bpp=1 Sr=5
    case 192 + 22: //Bpp=1 Sr=6
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //2bbp Stretch=4
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];

        if (!systemState->ScanLines)
        {
          yStride -= (32);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
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
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //2bbp Stretch=8
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];

        if (!systemState->ScanLines)
        {
          yStride -= (64);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          yStride -= Xpitch;
        }
      }
      break;

    case 192 + 31: //Bpp=1 Sr=15 2BPP Stretch=16 
      for (unsigned short beam = 0; beam < gs->BytesperRow; beam += 2) //2bbp Stretch=16
      {
        WidePixel = WideBuffer[(gs->VidMask & (start + (unsigned char)(gs->Hoffset + beam))) >> 1];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
        szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];

        if (!systemState->ScanLines)
        {
          yStride -= (128);
          yStride += Xpitch;
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 6))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 4))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 2))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & WidePixel)];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 14))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 12))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 10))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
          szSurface32[yStride += 1] = gs->Pallete32Bit[gs->PalleteIndex + (3 & (WidePixel >> 8))];
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
}

extern "C" {
  __declspec(dllexport)
    void __cdecl DrawTopBorder32(SystemState* systemState)
  {
    GraphicsState* gs = GetGraphicsState();

    if (gs->BorderChange == 0) {
      return;
    }

    for (unsigned short x = 0; x < systemState->WindowSize.x; x++)
    {
      systemState->PTRsurface32[x + ((systemState->LineCounter * 2) * systemState->SurfacePitch)] = gs->BorderColor32;

      if (!systemState->ScanLines) {
        systemState->PTRsurface32[x + ((systemState->LineCounter * 2 + 1) * systemState->SurfacePitch)] = gs->BorderColor32;
      }
    }
  }
}

extern "C" {
  __declspec(dllexport)
    void __cdecl DrawBottomBorder32(SystemState* systemState)
  {
    GraphicsState* gs = GetGraphicsState();

    if (gs->BorderChange == 0) {
      return;
    }

    for (unsigned short x = 0; x < systemState->WindowSize.x; x++)
    {
      systemState->PTRsurface32[x + (2 * (systemState->LineCounter + gs->LinesperScreen + gs->VertCenter) * systemState->SurfacePitch)] = gs->BorderColor32;

      if (!systemState->ScanLines) {
        systemState->PTRsurface32[x + systemState->SurfacePitch + (2 * (systemState->LineCounter + gs->LinesperScreen + gs->VertCenter) * systemState->SurfacePitch)] = gs->BorderColor32;
      }
    }
  }
}
