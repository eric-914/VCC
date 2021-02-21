#include "library/Graphics.h"
#include "library/Config.h"

void SetPaletteType() {
  int borderColor = GetGraphicsState()->CC3BorderColor;
  SetGimeBorderColor(0);
  MakeCMPpalette(GetPaletteType());
  SetGimeBorderColor(borderColor);
}
