/*****************************************************************************/
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
#include <math.h>

#include "graphicsstate.h"

static const unsigned char BlinkState = 1;
static const unsigned char BoarderChange = 3;
static const unsigned char Bpp = 0;
static const unsigned char BytesperRow = 32;
static const unsigned char CC2Offset = 0;
static const unsigned char CC2VDGMode = 0;
static const unsigned char CC2VDGPiaMode = 0;
static const unsigned char CC3BoarderColor = 0;
static const unsigned char CC3Vmode = 0;
static const unsigned char CC3Vres = 0;
static const unsigned char ColorInvert = 1;
static const unsigned char CompatMode = 0;
static const unsigned char ExtendedText = 1;
static const unsigned char GraphicsMode = 0;
static const unsigned char Hoffset = 0;
static const unsigned char HorzCenter = 0;
static const unsigned char HorzOffsetReg = 0;
static const unsigned char InvertAll = 0;
static const unsigned char LinesperRow = 1;
static const unsigned char LinesperScreen = 0;
static const unsigned char LowerCase = 0;
static const unsigned char MasterMode = 0;
static const unsigned char MonType = 1;
static const unsigned char PalleteIndex = 0;
static const unsigned char Stretch = 0;
static const unsigned char TextBGColor = 0;
static const unsigned char TextBGPallete = 0;
static const unsigned char TextFGColor = 0;
static const unsigned char TextFGPallete = 0;
static const unsigned char VertCenter = 0;
static const unsigned char VresIndex = 0;

static const unsigned short PixelsperLine = 0;
static const unsigned short TagY = 0;
static const unsigned short VerticalOffsetRegister = 0;
static const unsigned short VPitch = 32;

static const unsigned int DistoOffset = 0;
static const unsigned int NewStartofVidram = 0;
static const unsigned int StartofVidram = 0;
static const unsigned int VidMask = 0x1FFFF;

static const unsigned char  BoarderColor8 = 0;
static const unsigned short BoarderColor16 = 0;
static const unsigned int   BoarderColor32 = 0;

static const unsigned char  ColorTable16Bit[4] = { 0, 10, 21, 31 };	//Color brightness at 0 1 2 and 3 (2 bits)
static const unsigned char  ColorTable32Bit[4] = { 0, 85, 170, 255 };

static const unsigned char  Lpf[4] = { 192, 199, 225, 225 }; // #2 is really undefined but I gotta put something here.
static const unsigned char  VcenterTable[4] = { 29, 23, 12, 12 };

static const unsigned char  Afacts8[2][4] = { 0,     0xA4,     0x89,     0xBF, 0,      137,      164,      191 };
static const unsigned short Afacts16[2][4] = { 0,   0xF800,   0x001F,   0xFFFF, 0,   0x001F,   0xF800,   0xFFFF };
static const unsigned int   Afacts32[2][4] = { 0, 0xFF8D1F, 0x0667FF, 0xFFFFFF, 0, 0x0667FF, 0xFF8D1F, 0xFFFFFF };

GraphicsState* InitializeInstance(GraphicsState* graphicsState);

static GraphicsState* instance = InitializeInstance(new GraphicsState());

extern "C" {
  __declspec(dllexport) GraphicsState* __cdecl GetGraphicsState() {
    return instance;
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl CheckState(unsigned char attributes) {
    return (!instance->BlinkState) & !!(attributes & 128);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl FlipArtifacts() {
    instance->ColorInvert = instance->ColorInvert == 0 ? 1 : 0;
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl GetLpf(unsigned char index) {
    return instance->Lpf[index];
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl GetVcenterTable(unsigned char index) {
    return instance->VcenterTable[index];
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl ResetGraphicsState() {
    instance->CC3Vmode = 0;
    instance->CC3Vres = 0;
    instance->StartofVidram = 0;
    instance->NewStartofVidram = 0;
    instance->GraphicsMode = 0;
    instance->LowerCase = 0;
    instance->InvertAll = 0;
    instance->ExtendedText = 1;
    instance->HorzOffsetReg = 0;
    instance->TagY = 0;
    instance->DistoOffset = 0;
    instance->BoarderChange = 3;
    instance->CC2Offset = 0;
    instance->Hoffset = 0;
    instance->VerticalOffsetRegister = 0;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MakeRGBPalette(void)
  {
    unsigned char r, g, b;

    for (unsigned char index = 0; index < 64; index++)
    {
      instance->PalleteLookup8[1][index] = index | 128;

      r = instance->ColorTable16Bit[(index & 32) >> 4 | (index & 4) >> 2];
      g = instance->ColorTable16Bit[(index & 16) >> 3 | (index & 2) >> 1];
      b = instance->ColorTable16Bit[(index & 8) >> 2 | (index & 1)];
      instance->PalleteLookup16[1][index] = (r << 11) | (g << 6) | b;

      //32BIT
      r = instance->ColorTable32Bit[(index & 32) >> 4 | (index & 4) >> 2];
      g = instance->ColorTable32Bit[(index & 16) >> 3 | (index & 2) >> 1];
      b = instance->ColorTable32Bit[(index & 8) >> 2 | (index & 1)];
      instance->PalleteLookup32[1][index] = (r * 65536) + (g * 256) + b;
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl MakeCMPpalette(int paletteType)
  {
    double saturation, brightness, contrast;
    int offset;
    double w;
    double r, g, b;

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

    if (paletteType == 1) {
      OutputDebugString("Loading new CMP palette.\n");
    }
    else {
      OutputDebugString("Loading old CMP palette.\n");
    }

    for (unsigned char index = 0; index <= 63; index++)
    {
      if (paletteType == 1)
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

      GraphicsState* gs = GetGraphicsState();

      rr = (unsigned char)r;
      gg = (unsigned char)g;
      bb = (unsigned char)b;
      gs->PalleteLookup32[0][index] = (rr << 16) | (gg << 8) | bb;

      rr = rr >> 3;
      gg = gg >> 3;
      bb = bb >> 3;
      gs->PalleteLookup16[0][index] = (rr << 11) | (gg << 6) | bb;

      rr = rr >> 3;
      gg = gg >> 3;
      bb = bb >> 3;
      gs->PalleteLookup8[0][index] = 0x80 | ((rr & 2) << 4) | ((gg & 2) << 3) | ((bb & 2) << 2) | ((rr & 1) << 2) | ((gg & 1) << 1) | (bb & 1);
    }
  }
}

GraphicsState* InitializeInstance(GraphicsState* g) {
  g->BlinkState = BlinkState;
  g->BoarderChange = BoarderChange;
  g->Bpp = Bpp;
  g->BytesperRow = BytesperRow;
  g->CC2Offset = CC2Offset;
  g->CC2VDGMode = CC2VDGMode;
  g->CC2VDGPiaMode = CC2VDGPiaMode;
  g->CC3BoarderColor = CC3BoarderColor;
  g->CC3Vmode = CC3Vmode;
  g->CC3Vres = CC3Vres;
  g->ColorInvert = ColorInvert;
  g->CompatMode = CompatMode;
  g->ExtendedText = ExtendedText;
  g->GraphicsMode = GraphicsMode;
  g->Hoffset = Hoffset;
  g->HorzCenter = HorzCenter;
  g->HorzOffsetReg = HorzOffsetReg;
  g->InvertAll = InvertAll;
  g->LinesperRow = LinesperRow;
  g->LinesperScreen = LinesperScreen;
  g->LowerCase = LowerCase;
  g->MasterMode = MasterMode;
  g->MonType = MonType;
  g->PalleteIndex = PalleteIndex;
  g->Stretch = Stretch;
  g->TextBGColor = TextBGColor;
  g->TextBGPallete = TextBGPallete;
  g->TextFGColor = TextFGColor;
  g->TextFGPallete = TextFGPallete;
  g->VertCenter = VertCenter;
  g->VresIndex = VresIndex;

  g->PixelsperLine = PixelsperLine;
  g->TagY = TagY;
  g->VerticalOffsetRegister = VerticalOffsetRegister;
  g->VPitch = VPitch;

  g->DistoOffset = DistoOffset;
  g->NewStartofVidram = NewStartofVidram;
  g->StartofVidram = StartofVidram;
  g->VidMask = VidMask;

  g->BoarderColor8 = BoarderColor8;
  g->BoarderColor16 = BoarderColor16;
  g->BoarderColor32 = BoarderColor32;

  for (int i = 0; i < 4; i++) {
    g->ColorTable16Bit[i] = ColorTable16Bit[i];
    g->ColorTable32Bit[i] = ColorTable32Bit[i];

    g->Lpf[i] = Lpf[i];
    g->VcenterTable[i] = VcenterTable[i];
  }

  for (int i = 0; i < 16; i++) {
    g->Pallete[i] = 0;
    g->Pallete8Bit[i] = 0;
    g->Pallete16Bit[i] = 0;
    g->Pallete32Bit[i] = 0;
  }

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 4; j++) {
      g->Afacts8[i][j] = Afacts8[i][j];
      g->Afacts16[i][j] = Afacts16[i][j];
      g->Afacts32[i][j] = Afacts32[i][j];
    }
  }

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 64; j++) {
      g->PalleteLookup8[i][j] = 0;
      g->PalleteLookup16[i][j] = 0;
      g->PalleteLookup32[i][j] = 0;
    }
  }

  return g;
}