#ifndef __DEVFONT_H__
#define __DEVFONT_H__

typedef struct
{
unsigned int base, sz;
const unsigned char* sym;
} DevFontBlock;

typedef struct
{
unsigned char h, bytesH, maxW;
DevFontBlock block[2];
} DevFont;

#endif
