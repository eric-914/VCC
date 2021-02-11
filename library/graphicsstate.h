#ifndef __GRAPHICS_STATE_H__
#define __GRAPHICS_STATE_H__

typedef struct
{
  unsigned char BlinkState;
  unsigned char BoarderChange;
  unsigned char Bpp;
  unsigned char BytesperRow;
  unsigned char CC2Offset;
  unsigned char CC2VDGMode;
  unsigned char CC2VDGPiaMode;
  unsigned char CC3BoarderColor;
  unsigned char CC3Vmode;
  unsigned char CC3Vres;
  unsigned char ColorInvert;
  unsigned char CompatMode;
  unsigned char ExtendedText;
  unsigned char GraphicsMode;
  unsigned char Hoffset;
  unsigned char HorzCenter;
  unsigned char HorzOffsetReg;
  unsigned char InvertAll;
  unsigned char LinesperRow;
  unsigned char LinesperScreen;
  unsigned char LowerCase;
  unsigned char MasterMode;
  unsigned char MonType;
  unsigned char PalleteIndex;
  unsigned char Stretch;
  unsigned char TextBGColor;
  unsigned char TextBGPallete;
  unsigned char TextFGColor;
  unsigned char TextFGPallete;
  unsigned char VertCenter;
  unsigned char VresIndex;

  unsigned short PixelsperLine;
  unsigned short TagY;
  unsigned short VerticalOffsetRegister;
  unsigned short VPitch;

  unsigned int DistoOffset;
  unsigned int NewStartofVidram;
  unsigned int StartofVidram;
  unsigned int VidMask;

  unsigned char  BoarderColor8;
  unsigned short BoarderColor16;
  unsigned int   BoarderColor32;

  unsigned char  Afacts8[2][4];
  unsigned short Afacts16[2][4];
  unsigned int   Afacts32[2][4];

  unsigned char  ColorTable16Bit[4];
  unsigned char  ColorTable32Bit[4];

  unsigned char  Pallete[16];       //Coco 3 6 bit colors
  unsigned char  Pallete8Bit[16];
  unsigned short Pallete16Bit[16];  //Color values translated to 16bit 32BIT
  unsigned int   Pallete32Bit[16];  //Color values translated to 24/32 bits

  unsigned char  PalleteLookup8[2][64];	  //0 = RGB 1=comp 8BIT
  unsigned short PalleteLookup16[2][64];	//0 = RGB 1=comp 16BIT
  unsigned int   PalleteLookup32[2][64];	//0 = RGB 1=comp 32BIT

  unsigned char  Lpf[4];
  unsigned char  VcenterTable[4];
} GraphicsState;

extern "C" __declspec(dllexport) GraphicsState* __cdecl GetGraphicsState();

extern "C" __declspec(dllexport) unsigned char __cdecl CheckState(unsigned char attributes);
extern "C" __declspec(dllexport) void __cdecl FlipArtifacts();
extern "C" __declspec(dllexport) unsigned char __cdecl GetLpf(unsigned char index);
extern "C" __declspec(dllexport) unsigned char __cdecl GetVcenterTable(unsigned char index);

#endif
