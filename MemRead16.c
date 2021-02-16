#include "MemRead8.h"

unsigned short MemRead16(unsigned short addr)
{
  return (MemRead8(addr) << 8 | MemRead8(addr + 1));
}

