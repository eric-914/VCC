extern unsigned char pia0_read(unsigned char port);
extern unsigned char pia1_read(unsigned char port);
extern unsigned char sam_read(unsigned char);
extern unsigned char GimeRead(unsigned char);
extern unsigned char PackPortRead(unsigned char);

unsigned char port_read(unsigned short addr)
{
  unsigned char port = 0, value = 0;

  port = (addr & 0xFF);

  switch (port)
  {
  case 0:
  case 1:
  case 2:
  case 3:
    value = pia0_read(port);	//MC6821 P.I.A  Keyboard access $FF00-$FF03
    break;

  case 0x20:
  case 0x21:
  case 0x22:
  case 0x23:
    value = pia1_read(port);	//MC6821 P.I.A	Sound and VDG Control 
    break;

  case 0xC0:
  case 0xC1:
  case 0xC2:
  case 0xC3:
  case 0xC4:
  case 0xC5:
  case 0xC6:
  case 0xC7:
  case 0xC8:
  case 0xC9:
  case 0xCA:
  case 0xCB:
  case 0xCC:
  case 0xCD:
  case 0xCE:
  case 0xCF:
  case 0xD0:
  case 0xD1:
  case 0xD2:
  case 0xD3:
  case 0xD4:
  case 0xD5:
  case 0xD6:
  case 0xD7:
  case 0xD8:
  case 0xD9:
  case 0xDA:
  case 0xDB:
  case 0xDC:
  case 0xDD:
  case 0xDE:
  case 0xDF:
    value = sam_read(port);	//MC6883 S.A.M. address range $FFC0-$FFDF
    break;

  case 0xF0:
  case 0xF1:
  case 0xF2:
  case 0xF3:
  case 0xF4:
  case 0xF5:
  case 0xF6:
  case 0xF7:
  case 0xF8:
  case 0xF9:
  case 0xFA:
  case 0xFB:
  case 0xFC:
  case 0xFD:
  case 0xFE:
  case 0xFF:
    value = sam_read(port);	// SAM controls IRQ Vectors at $FFF0 - $FFFF	
    break;

  case 0x90:					//TCC1014 G.I.M.E.
  case 0x91:
  case 0x92:
  case 0x93:
  case 0x94:
  case 0x95:
  case 0x96:
  case 0x97:
  case 0x98:
  case 0x99:
  case 0x9A:
  case 0x9B:
  case 0x9C:
  case 0x9D:
  case 0x9E:
  case 0x9F:
  case 0xA0:
  case 0xA1:
  case 0xA2:
  case 0xA3:
  case 0xA4:
  case 0xA5:
  case 0xA6:
  case 0xA7:
  case 0xA8:
  case 0xA9:
  case 0xAA:
  case 0xAB:
  case 0xAC:
  case 0xAD:
  case 0xAE:
  case 0xAF:
  case 0xB0:
  case 0xB1:
  case 0xB2:
  case 0xB3:
  case 0xB4:
  case 0xB5:
  case 0xB6:
  case 0xB7:
  case 0xB8:
  case 0xB9:
  case 0xBA:
  case 0xBB:
  case 0xBC:
  case 0xBD:
  case 0xBE:
  case 0xBF:
    value = GimeRead(port);
    break;

  default:
    value = PackPortRead(port);
  }

  return(value);
}
