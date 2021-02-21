#pragma once

#define SIZEOF(_)  (sizeof(_) / sizeof((_)[0]))

//--Requires source variable to be "p" of course

#define STRARRAYCOPY(_) for (int i = 0; i < SIZEOF(_); i++) { strcpy(p->_[i], _[i]); }

#define ARRAYCOPY(_) for (int i = 0; i < SIZEOF(_); i++) { p->_[i] = _[i]; }

#define ZEROARRAY(_) for (int i = 0; i < SIZEOF(_); i++) { p->_[i] = 0; }
