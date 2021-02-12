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
#include "config.h"
#include "DirectDrawInterface.h"

#include "library/graphicsstate.h"
#include "library/defines.h"

void SetupDisplay(void); //This routine gets called every time a software video register get updated.

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
  GetGraphicsState()->BoarderChange = 5;
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
  ResetGraphicsState();

  MakeRGBPalette();
  MakeCMPpalette(GetPaletteType());
  MiscReset();
}

void SetVideoBank(unsigned char data)
{
  GetGraphicsState()->DistoOffset = data * (512 * 1024);

  SetupDisplay();
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
  int borderColor = GetGraphicsState()->CC3BoarderColor;
  SetGimeBoarderColor(0);
  MakeCMPpalette(GetPaletteType());
  SetGimeBoarderColor(borderColor);
}

unsigned char SetScanLines(SystemState* systemState, unsigned char lines)
{
  if (lines != QUERY)
  {
    systemState->ScanLines = lines;

    Cls(0, systemState);

    GetGraphicsState()->BoarderChange = 3;
  }

  return(0);
}
