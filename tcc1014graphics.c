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

#include "library/graphicsstate.h"
#include "library/defines.h"

void SetupDisplay(void); //This routine gets called every time a software video register get updated.
void MakeRGBPalette(void);
void MakeCMPpalette(void);

// These grab the Video info for all COCO 2 modes
void SetGimeVdgOffset(unsigned char offset)
{
  GraphicsState* gs = GetGraphicsState();

  if (gs->CC2Offset != offset)
  {
    gs->CC2Offset = offset;
    SetupDisplay();
  }
}

void SetGimeVdgMode(unsigned char vdgMode) //3 bits from SAM Registers
{
  GraphicsState* gs = GetGraphicsState();

  if (gs->CC2VDGMode != vdgMode)
  {
    gs->CC2VDGMode = vdgMode;
    SetupDisplay();
    gs->BoarderChange = 3;
  }
}

void SetGimeVdgMode2(unsigned char vdgmode2) //5 bits from PIA Register
{
  GraphicsState* gs = GetGraphicsState();

  if (gs->CC2VDGPiaMode != vdgmode2)
  {
    gs->CC2VDGPiaMode = vdgmode2;
    SetupDisplay();
    gs->BoarderChange = 3;
  }
}

//These grab the Video info for all COCO 3 modes

void SetVerticalOffsetRegister(unsigned short voRegister)
{
  GraphicsState* gs = GetGraphicsState();

  if (gs->VerticalOffsetRegister != voRegister)
  {
    gs->VerticalOffsetRegister = voRegister;

    SetupDisplay();
  }
}

void SetCompatMode(unsigned char mode)
{
  GraphicsState* gs = GetGraphicsState();

  if (gs->CompatMode != mode)
  {
    gs->CompatMode = mode;
    SetupDisplay();
    gs->BoarderChange = 3;
  }
}

void SetGimePallet(unsigned char pallete, unsigned char color)
{
  GraphicsState* gs = GetGraphicsState();

  // Convert the 6bit rgbrgb value to rrrrrggggggbbbbb for the Real video hardware.
  //	unsigned char r,g,b;
  gs->Pallete[pallete] = ((color & 63));
  gs->Pallete8Bit[pallete] = gs->PalleteLookup8[gs->MonType][color & 63];
  gs->Pallete16Bit[pallete] = gs->PalleteLookup16[gs->MonType][color & 63];
  gs->Pallete32Bit[pallete] = gs->PalleteLookup32[gs->MonType][color & 63];
}

void SetGimeVmode(unsigned char vmode)
{
  GraphicsState* gs = GetGraphicsState();

  if (gs->CC3Vmode != vmode)
  {
    gs->CC3Vmode = vmode;
    SetupDisplay();
    gs->BoarderChange = 3;
  }
}

void SetGimeVres(unsigned char vres)
{
  GraphicsState* gs = GetGraphicsState();

  if (gs->CC3Vres != vres)
  {
    gs->CC3Vres = vres;
    SetupDisplay();
    gs->BoarderChange = 3;
  }
}

void SetGimeHorzOffset(unsigned char data)
{
  GraphicsState* gs = GetGraphicsState();

  if (gs->HorzOffsetReg != data)
  {
    gs->Hoffset = (data << 1);
    gs->HorzOffsetReg = data;
    SetupDisplay();
  }
}

void SetGimeBoarderColor(unsigned char data)
{
  GraphicsState* gs = GetGraphicsState();

  if (gs->CC3BoarderColor != (data & 63))
  {
    gs->CC3BoarderColor = data & 63;
    SetupDisplay();
    gs->BoarderChange = 3;
  }
}

void SetBoarderChange(unsigned char data)
{
  GraphicsState* gs = GetGraphicsState();

  if (gs->BoarderChange > 0) {
    gs->BoarderChange--;
  }
}

void InvalidateBoarder(void)
{
  GraphicsState* gs = GetGraphicsState();

  gs->BoarderChange = 5;
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

  GraphicsState* gs = GetGraphicsState();

  gs->ExtendedText = 1;

  switch (gs->CompatMode)
  {
  case 0:		//Color Computer 3 Mode
    gs->NewStartofVidram = gs->VerticalOffsetRegister * 8;
    gs->GraphicsMode = (gs->CC3Vmode & 128) >> 7;
    gs->VresIndex = (gs->CC3Vres & 96) >> 5;
    CC3LinesperRow[7] = gs->LinesperScreen;	// For 1 pixel high modes
    gs->Bpp = gs->CC3Vres & 3;
    gs->LinesperRow = CC3LinesperRow[gs->CC3Vmode & 7];
    gs->BytesperRow = CC3BytesperRow[(gs->CC3Vres & 28) >> 2];
    gs->PalleteIndex = 0;

    if (!gs->GraphicsMode)
    {
      if (gs->CC3Vres & 1) {
        gs->ExtendedText = 2;
      }

      gs->Bpp = 0;
      gs->BytesperRow = CC3BytesperTextRow[(gs->CC3Vres & 28) >> 2];
    }

    break;

  case 1:					//Color Computer 2 Mode
    gs->CC3BoarderColor = 0;	//Black for text modes
    gs->BoarderChange = 3;
    gs->NewStartofVidram = (512 * gs->CC2Offset) + (gs->VerticalOffsetRegister & 0xE0FF) * 8;
    gs->GraphicsMode = (gs->CC2VDGPiaMode & 16) >> 4; //PIA Set on graphics clear on text
    gs->VresIndex = 0;
    gs->LinesperRow = CC2LinesperRow[gs->CC2VDGMode];

    if (gs->GraphicsMode)
    {
      ColorSet = (gs->CC2VDGPiaMode & 1);

      if (ColorSet == 0) {
        gs->CC3BoarderColor = 18; //18 Bright Green
      }
      else {
        gs->CC3BoarderColor = 63; //63 White 
      }

      gs->BoarderChange = 3;
      gs->Bpp = CC2Bpp[(gs->CC2VDGPiaMode & 15) >> 1];
      gs->BytesperRow = CC2BytesperRow[(gs->CC2VDGPiaMode & 15) >> 1];
      Temp1 = (gs->CC2VDGPiaMode & 1) << 1 | (gs->Bpp & 1);
      gs->PalleteIndex = CC2PaletteSet[Temp1];
    }
    else
    {	//Setup for 32x16 text Mode
      gs->Bpp = 0;
      gs->BytesperRow = 32;
      gs->InvertAll = (gs->CC2VDGPiaMode & 4) >> 2;
      gs->LowerCase = (gs->CC2VDGPiaMode & 2) >> 1;
      ColorSet = (gs->CC2VDGPiaMode & 1);
      Temp1 = ((ColorSet << 1) | gs->InvertAll);

      switch (Temp1)
      {
      case 0:
        gs->TextFGPallete = 12;
        gs->TextBGPallete = 13;
        break;

      case 1:
        gs->TextFGPallete = 13;
        gs->TextBGPallete = 12;
        break;

      case 2:
        gs->TextFGPallete = 14;
        gs->TextBGPallete = 15;
        break;

      case 3:
        gs->TextFGPallete = 15;
        gs->TextBGPallete = 14;
        break;
      }
    }

    break;
  }

  //gs->ColorInvert = (gs->CC3Vmode & 32) >> 5;
  gs->LinesperScreen = gs->Lpf[gs->VresIndex];
  SetLinesperScreen(gs->VresIndex);
  gs->VertCenter = gs->VcenterTable[gs->VresIndex] - 4; //4 unrendered top lines
  gs->PixelsperLine = gs->BytesperRow * CCPixelsperByte[gs->Bpp];

  if (gs->PixelsperLine % 40)
  {
    gs->Stretch = (512 / gs->PixelsperLine) - 1;
    gs->HorzCenter = 64;
  }
  else
  {
    gs->Stretch = (640 / gs->PixelsperLine) - 1;
    gs->HorzCenter = 0;
  }

  gs->VPitch = gs->BytesperRow;

  if (gs->HorzOffsetReg & 128) {
    gs->VPitch = 256;
  }

  gs->BoarderColor8 = ((gs->CC3BoarderColor & 63) | 128);
  gs->BoarderColor16 = gs->PalleteLookup16[gs->MonType][gs->CC3BoarderColor & 63];
  gs->BoarderColor32 = gs->PalleteLookup32[gs->MonType][gs->CC3BoarderColor & 63];
  gs->NewStartofVidram = (gs->NewStartofVidram & gs->VidMask) + gs->DistoOffset; //DistoOffset for 2M configuration
  gs->MasterMode = (gs->GraphicsMode << 7) | (gs->CompatMode << 6) | ((gs->Bpp & 3) << 4) | (gs->Stretch & 15);
}

void GimeInit(void)
{
}

void GimeReset(void)
{
  GraphicsState* gs = GetGraphicsState();

  gs->CC3Vmode = 0;
  gs->CC3Vres = 0;
  gs->StartofVidram = 0;
  gs->NewStartofVidram = 0;
  gs->GraphicsMode = 0;
  gs->LowerCase = 0;
  gs->InvertAll = 0;
  gs->ExtendedText = 1;
  gs->HorzOffsetReg = 0;
  gs->TagY = 0;
  gs->DistoOffset = 0;
  MakeRGBPalette();
  MakeCMPpalette();
  gs->BoarderChange = 3;
  gs->CC2Offset = 0;
  gs->Hoffset = 0;
  gs->VerticalOffsetRegister = 0;
  MiscReset();
}

void SetVidMask(unsigned int data)
{
  GraphicsState* gs = GetGraphicsState();

  gs->VidMask = data;
}

void SetVideoBank(unsigned char data)
{
  GraphicsState* gs = GetGraphicsState();

  gs->DistoOffset = data * (512 * 1024);
  SetupDisplay();
}

void MakeRGBPalette(void)
{
  unsigned char r, g, b;

  GraphicsState* gs = GetGraphicsState();

  for (unsigned char index = 0; index < 64; index++)
  {
    gs->PalleteLookup8[1][index] = index | 128;

    r = gs->ColorTable16Bit[(index & 32) >> 4 | (index & 4) >> 2];
    g = gs->ColorTable16Bit[(index & 16) >> 3 | (index & 2) >> 1];
    b = gs->ColorTable16Bit[(index & 8) >> 2 | (index & 1)];
    gs->PalleteLookup16[1][index] = (r << 11) | (g << 6) | b;

    //32BIT
    r = gs->ColorTable32Bit[(index & 32) >> 4 | (index & 4) >> 2];
    g = gs->ColorTable32Bit[(index & 16) >> 3 | (index & 2) >> 1];
    b = gs->ColorTable32Bit[(index & 8) >> 2 | (index & 1)];
    gs->PalleteLookup32[1][index] = (r * 65536) + (g * 256) + b;
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

unsigned char SetMonitorType(unsigned char type)
{
  GraphicsState* gs = GetGraphicsState();

  int borderColor = gs->CC3BoarderColor;
  SetGimeBoarderColor(0);

  if (type != QUERY)
  {
    gs->MonType = type & 1;

    for (unsigned char palIndex = 0; palIndex < 16; palIndex++)
    {
      gs->Pallete16Bit[palIndex] = gs->PalleteLookup16[gs->MonType][gs->Pallete[palIndex]];
      gs->Pallete32Bit[palIndex] = gs->PalleteLookup32[gs->MonType][gs->Pallete[palIndex]];
      gs->Pallete8Bit[palIndex] = gs->PalleteLookup8[gs->MonType][gs->Pallete[palIndex]];
    }
  }

  SetGimeBoarderColor(borderColor);

  return(gs->MonType);
}

void SetPaletteType() {
  GraphicsState* gs = GetGraphicsState();

  int borderColor = gs->CC3BoarderColor;
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

    GraphicsState* gs = GetGraphicsState();
    gs->BoarderChange = 3;
  }

  return(0);
}

int GetBytesPerRow() {
  GraphicsState* gs = GetGraphicsState();

  return gs->BytesperRow;
}

unsigned int GetStartOfVidram() {
  GraphicsState* gs = GetGraphicsState();

  return gs->StartofVidram;
}

int GetGraphicsMode() {
  GraphicsState* gs = GetGraphicsState();

  return(gs->GraphicsMode);
}

void FlipArtifacts() {
  GraphicsState* gs = GetGraphicsState();

  gs->ColorInvert = gs->ColorInvert == 0 ? 1 : 0;
}

unsigned char GetLpf(unsigned char index) {
  GraphicsState* gs = GetGraphicsState();

  return gs->Lpf[index];
}

unsigned char GetVcenterTable(unsigned char index) {
  GraphicsState* gs = GetGraphicsState();

  return gs->VcenterTable[index];
}

void SetBlinkState(unsigned char blinkState)
{
  GraphicsState* gs = GetGraphicsState();

  gs->BlinkState = blinkState;
}

unsigned char CheckState(unsigned char attributes) {
  GraphicsState* gs = GetGraphicsState();

  return (!gs->BlinkState) & !!(attributes & 128);
}
