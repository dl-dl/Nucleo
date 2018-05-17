#ifndef __SD_H
#define __SD_H

#include "types.h"

#define SDClearFlag()         (SDMMC1->ICR = SDMMC_STATIC_FLAGS)

ui32 SDInit(void);
ui32 SDWriteBlocks(const ui8 *pData, ui32 BlockAdd, ui32 NumberOfBlocks);
ui32 SDReadBlocks(ui8 *pData, ui32 BlockAdd, ui32 NumberOfBlocks);
ui32 SDErase(ui32 BlockStartAdd, ui32 BlockEndAdd);
ui32 SDTest(void);
#endif
