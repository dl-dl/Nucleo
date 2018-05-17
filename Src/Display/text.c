#include "screen.h"
#include "text.h"
#include "devfont.h"

extern DevFont font21x15;

static const unsigned char* FindChr(const DevFont* f, unsigned int c)
{
	const ui32 numBlocks = sizeof(f->block) / sizeof(*(f->block));
	for (ui32 i = 0; i < numBlocks; ++i)
	{
		if ((c >= f->block[i].base) && (c <= f->block[i].base + f->block[i].sz))
			return &f->block[i].sym[(c - f->block[i].base) * (f->maxW * f->bytesH + 1)];
	}
	return f->block[0].sym;
}

static unsigned int PrintChr(unsigned int chr, unsigned int x, unsigned int y, const DevFont* f, ui8 color)
{
	const unsigned char* c = FindChr(f, chr);
	for (ui32 i = 0; (i < c[0]); ++i)
	{
		for (ui32 j = 0; (j < f->h); ++j)
		{
			if (c[i * f->bytesH + 1 + j / 8] & (1 << (j % 8)))
				SetPixel(x + i, y + j, color);
		}
	}
	return c[0];
}

void PrintStr(const char* s, unsigned int x, unsigned int y, unsigned int fontType, ui8 color)
{
	const DevFont* f = &font21x15; // TODO: select based on fontType
	for (ui32 i = 0; s[i]; ++i)
		x += PrintChr(s[i], x, y, f, color);
}

void PrintStrW(const WIDE_CHAR* s, unsigned int x, unsigned int y, unsigned int fontType, ui8 color)
{
	const DevFont* f = &font21x15; // TODO: select based on fontType
	for (ui32 i = 0; s[i]; ++i)
		x += PrintChr(s[i], x, y, f, color);
}
