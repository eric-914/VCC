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

GraphicsState InitializeInstance(GraphicsState graphicsState);

static GraphicsState instance = { 0 };

GraphicsState* GetGraphicsState() {
  if (!instance.Initialized) {
    instance = InitializeInstance(instance);
  }

  return &instance;
}

GraphicsState InitializeInstance(GraphicsState graphicsState) {
  graphicsState.BlinkState = BlinkState;
  graphicsState.BoarderChange = BoarderChange;
  graphicsState.Bpp = Bpp;
  graphicsState.BytesperRow = BytesperRow;
  graphicsState.CC2Offset = CC2Offset;
  graphicsState.CC2VDGMode = CC2VDGMode;
  graphicsState.CC2VDGPiaMode = CC2VDGPiaMode;
  graphicsState.CC3BoarderColor = CC3BoarderColor;
  graphicsState.CC3Vmode = CC3Vmode;
  graphicsState.CC3Vres = CC3Vres;
  graphicsState.ColorInvert = ColorInvert;
  graphicsState.CompatMode = CompatMode;
  graphicsState.ExtendedText = ExtendedText;
  graphicsState.GraphicsMode = GraphicsMode;
  graphicsState.Hoffset = Hoffset;
  graphicsState.HorzCenter = HorzCenter;
  graphicsState.HorzOffsetReg = HorzOffsetReg;
  graphicsState.InvertAll = InvertAll;
  graphicsState.LinesperRow = LinesperRow;
  graphicsState.LinesperScreen = LinesperScreen;
  graphicsState.LowerCase = LowerCase;
  graphicsState.MasterMode = MasterMode;
  graphicsState.MonType = MonType;
  graphicsState.PalleteIndex = PalleteIndex;
  graphicsState.Stretch = Stretch;
  graphicsState.TextBGColor = TextBGColor;
  graphicsState.TextBGPallete = TextBGPallete;
  graphicsState.TextFGColor = TextFGColor;
  graphicsState.TextFGPallete = TextFGPallete;
  graphicsState.VertCenter = VertCenter;
  graphicsState.VresIndex = VresIndex;

  graphicsState.PixelsperLine = PixelsperLine;
  graphicsState.TagY = TagY;
  graphicsState.VerticalOffsetRegister = VerticalOffsetRegister;
  graphicsState.VPitch = VPitch;

  graphicsState.DistoOffset = DistoOffset;
  graphicsState.NewStartofVidram = NewStartofVidram;
  graphicsState.StartofVidram = StartofVidram;
  graphicsState.VidMask = VidMask;

  graphicsState.BoarderColor8 = BoarderColor8;
  graphicsState.BoarderColor16 = BoarderColor16;
  graphicsState.BoarderColor32 = BoarderColor32;

  for (int i = 0; i < 4; i++) {
    graphicsState.ColorTable16Bit[i] = ColorTable16Bit[i];
    graphicsState.ColorTable32Bit[i] = ColorTable32Bit[i];

    graphicsState.Lpf[i] = Lpf[i];
    graphicsState.VcenterTable[i] = VcenterTable[i];
  }

  for (int i = 0; i < 16; i++) {
    graphicsState.Pallete[i] = 0;
    graphicsState.Pallete8Bit[i] = 0;
    graphicsState.Pallete16Bit[i] = 0;
    graphicsState.Pallete32Bit[i] = 0;
  }

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 4; j++) {
      graphicsState.Afacts8[i][j] = Afacts8[i][j];
      graphicsState.Afacts16[i][j] = Afacts16[i][j];
      graphicsState.Afacts32[i][j] = Afacts32[i][j];
    }
  }

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 64; j++) {
      graphicsState.PalleteLookup8[i][j] = 0;
      graphicsState.PalleteLookup16[i][j] = 0;
      graphicsState.PalleteLookup32[i][j] = 0;
    }
  }

  graphicsState.Initialized = 1;

  return graphicsState;
}