#include "screen.h"
#include "spi.h"
#include "gpio.h"

#define SPI_DATA_NUM_BIT 4
#define SCREEN_CX 240
#define SCREEN_CY 400

#define DATA_SZ (SCREEN_CY * SPI_DATA_NUM_BIT / 8)

#define SCREEN_TRANSMIT_MODE 0x90

#pragma pack(push,1)
typedef struct
{
 ui8  Mode;
 ui8  NumLine;
 ui8  Pixels[DATA_SZ];
 ui16 Terminator;
} tLines;
#pragma pack(pop)

#pragma pack(push,1)
struct
{
 tLines Lines[SCREEN_CX];
} Screen;
#pragma pack(pop)


static const ui8 colorLo[8] = { 0x00, 0x20, 0x40, 0x80, 0x60, 0xC0, 0xA0, 0xE0 };
static const ui8 colorHi[8] = { 0x00, 0x02, 0x04, 0x08, 0x06, 0x0C, 0x0A, 0x0E };
static const ui8 colorHL[8] = { 0x00, 0x22, 0x44, 0x88, 0x66, 0xCC, 0xAA, 0xEE };

void SetPixel(ui16 x, ui16 y, ui8 color)
{
 if (x >= SCREEN_CX)
	 return;
 if (y >= SCREEN_CY)
	 return;

 color %= sizeof(colorLo)/sizeof(*colorLo);

 ui8 *ptr = &Screen.Lines[x].Pixels[y / 2];

 if (y % 2)
   *ptr = (*ptr & 0xF0) | colorHi[color];
 else
	 *ptr = (*ptr & 0x0F) | colorLo[color];
}

static void VLine(ui16 x, ui16 y, ui16 height, ui8 color)
{
 ui8 *ptr = &Screen.Lines[x].Pixels[y / 2];

 if (y % 2)
	{
	 *ptr = ((*ptr & 0xF0) | colorHi[color]);
	 ptr++;
	 height--;
	}

 for (ui16 cnt = height / 2; cnt--; )
		*ptr++ = colorHL[color];

 if (height % 2)
	*ptr = ((*ptr & 0x0F) | colorLo[color]);
}

void FillRect(ui16 left, ui16 top, ui16 width, ui16 height, ui8 color)
{
 if (left + width > SCREEN_CX)
	return;
 if (top + height > SCREEN_CY)
	return;

 color %= sizeof(colorLo) / sizeof(*colorLo);

 while (width--)
	VLine(left + width, top, height, color);
}

void ScreenInit(ui8 color)
{
 for (ui8 x = 0; x < SCREEN_CX; ++x)
	{
	 Screen.Lines[x].Mode = SCREEN_TRANSMIT_MODE;
	 Screen.Lines[x].NumLine = x;
	 Screen.Lines[x].Terminator = 0; // 16 stop bits
	}
	FillRect(0, 0, SCREEN_CX, SCREEN_CY, color);
}

void ScreenRainbow()
{
 for ( ui16 x = 0; x < SCREEN_CX; x++)
  {for ( ui16 y = 0; y < SCREEN_CY / 2; y++)
    {
     Screen.Lines[x].Pixels[y]  = colorHL[y / 25];
    } 
  }
}

void ScreenTransmit()
{
 ui16 toggle = DISPLAY_BUSY_GPIO_Port->IDR & DISPLAY_BUSY_Pin;
 ui16 cntNotAnswer = 0;  

 for ( ui16 x = 0; x < SCREEN_CX; )
  {
   
   if ( cntNotAnswer > 500 )
    {
     break;
    } 
   SPI1WriteDMA((uint8_t*)(&Screen.Lines[x]), sizeof(tLines));
   
   __IO uint32_t  tmp = SysTick->CTRL;            // Clear the COUNTFLAG

   while ( 1 ) 
    {
     if ( toggle != (DISPLAY_BUSY_GPIO_Port->IDR & DISPLAY_BUSY_Pin) ) 
      {x++;
       cntNotAnswer = 0;
       break;
      }
     if ( (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) != 0U)
      {cntNotAnswer++;
       break;
      }
    }
   toggle = DISPLAY_BUSY_GPIO_Port->IDR & DISPLAY_BUSY_Pin;
	}
}
