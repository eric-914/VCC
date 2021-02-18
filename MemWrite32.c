#include "MemWrite16.h"

void MemWrite32(unsigned int data, unsigned short addr)
{
  MemWrite16(data >> 16, addr);
  MemWrite16(data & 0xFFFF, addr + 2);
}
