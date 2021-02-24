#include <windows.h>
#include <math.h>

#include "Graphics.h"
#include "CoCo.h"
#include "VCC.h"
#include "Config.h"
#include "DirectDraw.h"

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

      rr = (unsigned char)r;
      gg = (unsigned char)g;
      bb = (unsigned char)b;
      instance->PalleteLookup32[0][index] = (rr << 16) | (gg << 8) | bb;

      rr = rr >> 3;
      gg = gg >> 3;
      bb = bb >> 3;
      instance->PalleteLookup16[0][index] = (rr << 11) | (gg << 6) | bb;

      rr = rr >> 3;
      gg = gg >> 3;
      bb = bb >> 3;
      instance->PalleteLookup8[0][index] = 0x80 | ((rr & 2) << 4) | ((gg & 2) << 3) | ((bb & 2) << 2) | ((rr & 1) << 2) | ((gg & 1) << 1) | (bb & 1);
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetGimePalette(unsigned char pallete, unsigned char color)
  {
    // Convert the 6bit rgbrgb value to rrrrrggggggbbbbb for the Real video hardware.
    //	unsigned char r,g,b;
    instance->Pallete[pallete] = ((color & 63));
    instance->Pallete8Bit[pallete] = instance->PalleteLookup8[instance->MonType][color & 63];
    instance->Pallete16Bit[pallete] = instance->PalleteLookup16[instance->MonType][color & 63];
    instance->Pallete32Bit[pallete] = instance->PalleteLookup32[instance->MonType][color & 63];
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl InvalidateBorder(void)
  {
    instance->BorderChange = 5;
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetBorderChange(unsigned char data)
  {
    if (instance->BorderChange > 0) {
      instance->BorderChange--;
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

    instance->ExtendedText = 1;

    switch (instance->CompatMode)
    {
    case 0:		//Color Computer 3 Mode
      instance->NewStartofVidram = instance->VerticalOffsetRegister * 8;
      instance->GraphicsMode = (instance->CC3Vmode & 128) >> 7;
      instance->VresIndex = (instance->CC3Vres & 96) >> 5;
      CC3LinesperRow[7] = instance->LinesperScreen;	// For 1 pixel high modes
      instance->Bpp = instance->CC3Vres & 3;
      instance->LinesperRow = CC3LinesperRow[instance->CC3Vmode & 7];
      instance->BytesperRow = CC3BytesperRow[(instance->CC3Vres & 28) >> 2];
      instance->PalleteIndex = 0;

      if (!instance->GraphicsMode)
      {
        if (instance->CC3Vres & 1) {
          instance->ExtendedText = 2;
        }

        instance->Bpp = 0;
        instance->BytesperRow = CC3BytesperTextRow[(instance->CC3Vres & 28) >> 2];
      }

      break;

    case 1:					//Color Computer 2 Mode
      instance->CC3BorderColor = 0;	//Black for text modes
      instance->BorderChange = 3;
      instance->NewStartofVidram = (512 * instance->CC2Offset) + (instance->VerticalOffsetRegister & 0xE0FF) * 8;
      instance->GraphicsMode = (instance->CC2VDGPiaMode & 16) >> 4; //PIA Set on graphics clear on text
      instance->VresIndex = 0;
      instance->LinesperRow = CC2LinesperRow[instance->CC2VDGMode];

      if (instance->GraphicsMode)
      {
        ColorSet = (instance->CC2VDGPiaMode & 1);

        if (ColorSet == 0) {
          instance->CC3BorderColor = 18; //18 Bright Green
        }
        else {
          instance->CC3BorderColor = 63; //63 White 
        }

        instance->BorderChange = 3;
        instance->Bpp = CC2Bpp[(instance->CC2VDGPiaMode & 15) >> 1];
        instance->BytesperRow = CC2BytesperRow[(instance->CC2VDGPiaMode & 15) >> 1];
        Temp1 = (instance->CC2VDGPiaMode & 1) << 1 | (instance->Bpp & 1);
        instance->PalleteIndex = CC2PaletteSet[Temp1];
      }
      else
      {	//Setup for 32x16 text Mode
        instance->Bpp = 0;
        instance->BytesperRow = 32;
        instance->InvertAll = (instance->CC2VDGPiaMode & 4) >> 2;
        instance->LowerCase = (instance->CC2VDGPiaMode & 2) >> 1;
        ColorSet = (instance->CC2VDGPiaMode & 1);
        Temp1 = ((ColorSet << 1) | instance->InvertAll);

        switch (Temp1)
        {
        case 0:
          instance->TextFGPallete = 12;
          instance->TextBGPallete = 13;
          break;

        case 1:
          instance->TextFGPallete = 13;
          instance->TextBGPallete = 12;
          break;

        case 2:
          instance->TextFGPallete = 14;
          instance->TextBGPallete = 15;
          break;

        case 3:
          instance->TextFGPallete = 15;
          instance->TextBGPallete = 14;
          break;
        }
      }

      break;
    }

    //gs->ColorInvert = (gs->CC3Vmode & 32) >> 5;
    instance->LinesperScreen = instance->Lpf[instance->VresIndex];

    SetLinesperScreen(instance->VresIndex);

    instance->VertCenter = instance->VcenterTable[instance->VresIndex] - 4; //4 unrendered top lines
    instance->PixelsperLine = instance->BytesperRow * CCPixelsperByte[instance->Bpp];

    if (instance->PixelsperLine % 40)
    {
      instance->Stretch = (512 / instance->PixelsperLine) - 1;
      instance->HorzCenter = 64;
    }
    else
    {
      instance->Stretch = (640 / instance->PixelsperLine) - 1;
      instance->HorzCenter = 0;
    }

    instance->VPitch = instance->BytesperRow;

    if (instance->HorzOffsetReg & 128) {
      instance->VPitch = 256;
    }

    instance->BorderColor8 = ((instance->CC3BorderColor & 63) | 128);
    instance->BorderColor16 = instance->PalleteLookup16[instance->MonType][instance->CC3BorderColor & 63];
    instance->BorderColor32 = instance->PalleteLookup32[instance->MonType][instance->CC3BorderColor & 63];
    instance->NewStartofVidram = (instance->NewStartofVidram & instance->VidMask) + instance->DistoOffset; //DistoOffset for 2M configuration
    instance->MasterMode = (instance->GraphicsMode << 7) | (instance->CompatMode << 6) | ((instance->Bpp & 3) << 4) | (instance->Stretch & 15);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetCompatMode(unsigned char mode)
  {
    if (instance->CompatMode != mode)
    {
      instance->CompatMode = mode;
      SetupDisplay();
      instance->BorderChange = 3;
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetGimeBorderColor(unsigned char data)
  {
    if (instance->CC3BorderColor != (data & 63))
    {
      instance->CC3BorderColor = data & 63;
      SetupDisplay();
      instance->BorderChange = 3;
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetGimeHorzOffset(unsigned char data)
  {
    if (instance->HorzOffsetReg != data)
    {
      instance->Hoffset = (data << 1);
      instance->HorzOffsetReg = data;
      SetupDisplay();
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetGimeVdgMode(unsigned char vdgMode) //3 bits from SAM Registers
  {
    if (instance->CC2VDGMode != vdgMode)
    {
      instance->CC2VDGMode = vdgMode;
      SetupDisplay();
      instance->BorderChange = 3;
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetGimeVdgMode2(unsigned char vdgmode2) //5 bits from PIA Register
  {
    if (instance->CC2VDGPiaMode != vdgmode2)
    {
      instance->CC2VDGPiaMode = vdgmode2;
      SetupDisplay();
      instance->BorderChange = 3;
    }
  }
}

// These grab the Video info for all COCO 2 modes
extern "C" {
  __declspec(dllexport) void __cdecl SetGimeVdgOffset(unsigned char offset)
  {
    if (instance->CC2Offset != offset)
    {
      instance->CC2Offset = offset;
      SetupDisplay();
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetGimeVmode(unsigned char vmode)
  {
    if (instance->CC3Vmode != vmode)
    {
      instance->CC3Vmode = vmode;
      SetupDisplay();
      instance->BorderChange = 3;
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetGimeVres(unsigned char vres)
  {
    if (instance->CC3Vres != vres)
    {
      instance->CC3Vres = vres;
      SetupDisplay();
      instance->BorderChange = 3;
    }
  }
}

//These grab the Video info for all COCO 3 modes
extern "C" {
  __declspec(dllexport) void __cdecl SetVerticalOffsetRegister(unsigned short voRegister)
  {
    if (instance->VerticalOffsetRegister != voRegister)
    {
      instance->VerticalOffsetRegister = voRegister;

      SetupDisplay();
    }
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetVideoBank(unsigned char data)
  {
    instance->DistoOffset = data * (512 * 1024);

    SetupDisplay();
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetMonitorType(unsigned char type)
  {
    int borderColor = instance->CC3BorderColor;

    SetGimeBorderColor(0);

    if (type != QUERY)
    {
      instance->MonType = type & 1;

      for (unsigned char palIndex = 0; palIndex < 16; palIndex++)
      {
        instance->Pallete16Bit[palIndex] = instance->PalleteLookup16[instance->MonType][instance->Pallete[palIndex]];
        instance->Pallete32Bit[palIndex] = instance->PalleteLookup32[instance->MonType][instance->Pallete[palIndex]];
        instance->Pallete8Bit[palIndex] = instance->PalleteLookup8[instance->MonType][instance->Pallete[palIndex]];
      }
    }

    SetGimeBorderColor(borderColor);

    return(instance->MonType);
  }
}

extern "C" {
  __declspec(dllexport) void __cdecl SetPaletteType() {
    int borderColor = instance->CC3BorderColor;

    SetGimeBorderColor(0);
    MakeCMPpalette(GetPaletteType());
    SetGimeBorderColor(borderColor);
  }
}

extern "C" {
  __declspec(dllexport) unsigned char __cdecl SetScanLines(SystemState* systemState, unsigned char lines)
  {
    if (lines != QUERY)
    {
      systemState->ScanLines = lines;

      Cls(0, systemState);

      instance->BorderChange = 3;
    }

    return(0);
  }
}
