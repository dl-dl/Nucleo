#include "screen.h"
#include "graph.h"

static inline int intAbs(int i)
{
	return i >= 0 ? i : -i;
}

void Line(int x0, int y0, int x1, int y1, ui8 color)
{
	const int dx = intAbs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	const int dy = -intAbs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2;
	for (;;)
	{
		SetPixel(x0, y0, color);
		if (x0 == x1 && y0 == y1)
			break;
		e2 = 2 * err;
		if (e2 >= dy)
		{
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx)
		{
			err += dx;
			y0 += sy;
		}
	}
}

void Circle(int xm, int ym, int r, ui8 color)
{
	int x = -r, y = 0, err = 2 - 2 * r;
	do
	{
		SetPixel(xm - x, ym + y, color);
		SetPixel(xm - y, ym - x, color);
		SetPixel(xm + x, ym - y, color);
		SetPixel(xm + y, ym + x, color);
		r = err;
		if (r > x)
			err += ++x * 2 + 1;
		if (r <= y)
			err += ++y * 2 + 1;
	}
	while (x < 0);
}
