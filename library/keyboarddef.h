#ifndef __KEYBOARDDEF_H__
#define __KEYBOARDDEF_H__

typedef enum keyevent_e
{
  kEventKeyUp = 0,
  kEventKeyDown = 1
} keyevent_e;

/**
  Keyboard layouts
*/
typedef enum keyboardlayout_e
{
  kKBLayoutCoCo = 0,
  kKBLayoutNatural,
  kKBLayoutCompact,
  kKBLayoutCustom,

  kKBLayoutCount
} keyboardlayout_e;

/**
  Keyboard layout names used to populate the
  layout selection pull-down in the config dialog

  This of course must match keyboardlayout_e above
*/
const char* const k_keyboardLayoutNames[] =
{
  "CoCo (DECB)",
  "Natural (OS-9)",
  "Compact (OS-9)",
  "Custom"
};

typedef struct keytranslationentry_t
{
  unsigned char ScanCode1;
  unsigned char ScanCode2;
  unsigned char Row1;
  unsigned char Col1;
  unsigned char Row2;
  unsigned char Col2;
} keytranslationentry_t;

typedef struct {
  unsigned char UseMouse;
  unsigned char Up;
  unsigned char Down;
  unsigned char Left;
  unsigned char Right;
  unsigned char Fire1;
  unsigned char Fire2;
  unsigned char DiDevice;
  unsigned char HiRes;
} JoyStick;

#endif