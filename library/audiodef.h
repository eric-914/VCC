#ifndef __AUDIODEF_H__
#define __AUDIODEF_H__

typedef struct CardList {
  char CardName[64];
  _GUID* Guid;
} SndCardList;

#endif