#ifndef __TEXT_H__
#define __TEXT_H__

#include "types.h"

#define FONT_NORMAL 0

typedef unsigned short WIDE_CHAR;

void PrintStr(const char* s, unsigned int x, unsigned int y, unsigned int fontType, ui8 color);
void PrintStrW(const WIDE_CHAR* s, unsigned int x, unsigned int y, unsigned int fontType, ui8 color);

#endif
