#include "library/graphicsstate.h"

#include "SetLinesperScreen.h"

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
    gs->CC3BorderColor = 0;	//Black for text modes
    gs->BorderChange = 3;
    gs->NewStartofVidram = (512 * gs->CC2Offset) + (gs->VerticalOffsetRegister & 0xE0FF) * 8;
    gs->GraphicsMode = (gs->CC2VDGPiaMode & 16) >> 4; //PIA Set on graphics clear on text
    gs->VresIndex = 0;
    gs->LinesperRow = CC2LinesperRow[gs->CC2VDGMode];

    if (gs->GraphicsMode)
    {
      ColorSet = (gs->CC2VDGPiaMode & 1);

      if (ColorSet == 0) {
        gs->CC3BorderColor = 18; //18 Bright Green
      }
      else {
        gs->CC3BorderColor = 63; //63 White 
      }

      gs->BorderChange = 3;
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

  gs->BorderColor8 = ((gs->CC3BorderColor & 63) | 128);
  gs->BorderColor16 = gs->PalleteLookup16[gs->MonType][gs->CC3BorderColor & 63];
  gs->BorderColor32 = gs->PalleteLookup32[gs->MonType][gs->CC3BorderColor & 63];
  gs->NewStartofVidram = (gs->NewStartofVidram & gs->VidMask) + gs->DistoOffset; //DistoOffset for 2M configuration
  gs->MasterMode = (gs->GraphicsMode << 7) | (gs->CompatMode << 6) | ((gs->Bpp & 3) << 4) | (gs->Stretch & 15);
}
