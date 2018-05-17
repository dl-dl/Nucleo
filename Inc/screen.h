#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "types.h"

void ScreenInit(ui8 color);
void ScreenRainbow(void);
void ScreenTransmit(void);

void SetPixel(ui16 x, ui16 y, ui8 color);
void FillRect(ui16 left, ui16 top, ui16 width, ui16 height, ui8 color);

#endif
