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