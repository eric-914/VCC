#include <windows.h>
#include <math.h>

#include "Graphics.h"
#include "CoCo.h"

#include "macros.h"

static const unsigned char  ColorTable16Bit[4] = { 0, 10, 21, 31 };	//Color brightness at 0 1 2 and 3 (2 bits)
static const unsigned char  ColorTable32Bit[4] = { 0, 85, 170, 255 };

static const unsigned char  Lpf[4] = { 192, 199, 225, 225 }; // #2 is really undefined but I gotta put something here.
static const unsigned char  VcenterTable[4] = { 29, 23, 12, 12 };

static const unsigned char  Afacts8[2][4] = { 0,     0xA4,     0x89,     0xBF, 0,      137,      164,      191 };
static const unsigned short Afacts16[2][4] = { 0,   0xF800,   0x001F,   0xFFFF, 0,   0x001F,   0xF800,   0xFFFF };
static const unsigned int   Afacts32[2][4] = { 0, 0xFF8D1F, 0x0667FF, 0xFFFFFF, 0, 0x0667FF, 0xFF8D1F, 0xFFFFFF };

GraphicsState* InitializeInstance(GraphicsState*);

static GraphicsState* instance = InitializeInstance(new GraphicsState());

GraphicsState* InitializeInstance(GraphicsState* p) {
  p->BlinkState = 1;
  p->BorderChange = 3;
  p->Bpp = 0;
  p->BytesperRow = 32;
  p->CC2Offset = 0;
  p->CC2VDGMode = 0;
  p->CC2VDGPiaMode = 0;
  p->CC3BorderColor = 0;
  p->CC3Vmode = 0;
  p->CC3Vres = 0;
  p->ColorInvert = 1;
  p->CompatMode = 0;
  p->ExtendedText = 1;
  p->GraphicsMode = 0;
  p->Hoffset = 0;
  p->HorzCenter = 0;
  p->HorzOffsetReg = 0;
  p->InvertAll = 0;
  p->LinesperRow = 1;
  p->LinesperScreen = 0;
  p->LowerCase = 0;
  p->MasterMode = 0;
  p->MonType = 1;
  p->PalleteIndex = 0;
  p->Stretch = 0;
  p->TextBGColor = 0;
  p->TextBGPallete = 0;
  p->TextFGColor = 0;
  p->TextFGPallete = 0;
  p->VertCenter = 0;
  p->VresIndex = 0;

  p->PixelsperLine = 0;
  p->TagY = 0;
  p->VerticalOffsetRegister = 0;
  p->VPitch = 32;

  p->DistoOffset = 0;
  p->NewStartofVidram = 0;
  p->StartofVidram = 0;
  p->VidMask = 0x1FFFF;

  p->BorderColor8 = 0;
  p->BorderColor16 = 0;
  p->BorderColor32 = 0;

  ARRAYCOPY(ColorTable16Bit);
  ARRAYCOPY(ColorTable32Bit);
  ARRAYCOPY(Lpf);
  ARRAYCOPY(VcenterTable);

  //ZEROARRAY(Pallete);

  for (int i = 0; i < 16; i++) {
    p->Pallete[i] = 0;
    p->Pallete8Bit[i] = 0;
    p->Pallete16Bit[i] = 0;
    p->Pallete32Bit[i] = 0;
  }

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 4; j++) {
      p->Afacts8[i][j] = Afacts8[i][j];
      p->Afacts16[i][j] = Afacts16[i][j];
      p->Afacts32[i][j] = Afacts32[i][j];
    }
  }

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 64; j++) {
      p->PalleteLookup8[i][j] = 0;
      p->PalleteLookup16[i][j] = 0;
      p->PalleteLookup32[i][j] = 0;
    }
  }

  return p;
}

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
    instance->BorderChange = 3;
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
          brightness += (((double)index / 16) + (double)1) * contrast;
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

extern "C" {
  __declspec(dllexport) void __cdecl SetGimePalette(unsigned char pallete, unsigned char color)
  {
    GraphicsState* gs = GetGraphicsState();

    // Convert the 6bit rgbrgb value to rrrrrggggggbbbbb for the Real video hardware.
    //	unsigned char r,g,b;
    gs->Pallete[pallete] = ((color & 63));
    gs->Pallete8Bit[pallete] = gs->PalleteLookup8[gs->MonType][color & 63];
    gs->Pallete16Bit[pallete] = gs->PalleteLookup16[gs->MonType][color & 63];
    gs->Pallete32Bit[pallete] = gs->PalleteLookup32[gs->MonType][color & 63];
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl InvalidateBorder(void)
  {
    GetGraphicsState()->BorderChange = 5;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetBorderChange(unsigned char data)
  {
    GraphicsState* gs = GetGraphicsState();

    if (gs->BorderChange > 0) {
      gs->BorderChange--;
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetupDisplay(void)
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
}

extern "C" {
  __declspec(dllexport) void __cdecl SetCompatMode(unsigned char mode)
  {
    GraphicsState* gs = GetGraphicsState();

    if (gs->CompatMode != mode)
    {
      gs->CompatMode = mode;
      SetupDisplay();
      gs->BorderChange = 3;
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetGimeBorderColor(unsigned char data)
  {
    GraphicsState* gs = GetGraphicsState();

    if (gs->CC3BorderColor != (data & 63))
    {
      gs->CC3BorderColor = data & 63;
      SetupDisplay();
      gs->BorderChange = 3;
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetGimeHorzOffset(unsigned char data)
  {
    GraphicsState* gs = GetGraphicsState();

    if (gs->HorzOffsetReg != data)
    {
      gs->Hoffset = (data << 1);
      gs->HorzOffsetReg = data;
      SetupDisplay();
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetGimeVdgMode(unsigned char vdgMode) //3 bits from SAM Registers
  {
    GraphicsState* gs = GetGraphicsState();

    if (gs->CC2VDGMode != vdgMode)
    {
      gs->CC2VDGMode = vdgMode;
      SetupDisplay();
      gs->BorderChange = 3;
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetGimeVdgMode2(unsigned char vdgmode2) //5 bits from PIA Register
  {
    GraphicsState* gs = GetGraphicsState();

    if (gs->CC2VDGPiaMode != vdgmode2)
    {
      gs->CC2VDGPiaMode = vdgmode2;
      SetupDisplay();
      gs->BorderChange = 3;
    }
  }
}

// These grab the Video info for all COCO 2 modes
extern "C" {
  __declspec(dllexport) void __cdecl SetGimeVdgOffset(unsigned char offset)
  {
    GraphicsState* gs = GetGraphicsState();

    if (gs->CC2Offset != offset)
    {
      gs->CC2Offset = offset;
      SetupDisplay();
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetGimeVmode(unsigned char vmode)
  {
    GraphicsState* gs = GetGraphicsState();

    if (gs->CC3Vmode != vmode)
    {
      gs->CC3Vmode = vmode;
      SetupDisplay();
      gs->BorderChange = 3;
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetGimeVres(unsigned char vres)
  {
    GraphicsState* gs = GetGraphicsState();

    if (gs->CC3Vres != vres)
    {
      gs->CC3Vres = vres;
      SetupDisplay();
      gs->BorderChange = 3;
    }
  }
}

//These grab the Video info for all COCO 3 modes
extern "C" {
  __declspec(dllexport) void __cdecl SetVerticalOffsetRegister(unsigned short voRegister)
  {
    GraphicsState* gs = GetGraphicsState();

    if (gs->VerticalOffsetRegister != voRegister)
    {
      gs->VerticalOffsetRegister = voRegister;

      SetupDisplay();
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetVideoBank(unsigned char data)
  {
    GetGraphicsState()->DistoOffset = data * (512 * 1024);

    SetupDisplay();
  }
}
