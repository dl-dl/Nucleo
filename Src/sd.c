#include "types.h"
#include "gpio.h"
#include "stm32l4xx_ll_gpio.h"
#include "sd\sdmmc.h"
#include "sd.h"



#define RESP_NO           0
#define RESP_R1           1
#define RESP_R1b          2  
#define RESP_R2           3
#define RESP_R3           4
#define RESP_R6           5
#define RESP_R7           6

#define CARD_SDSC         0
#define CARD_SDHC_SDXC    1

#define BLOCKSIZE       512U

#define SDMMC_CLKCR_CLKEN   0x00000100      //SDMMC_CLKCR_CLKEN_Msk                  // Clock enable bit

#define SD_STATE_MASK  ((ui32)0x00001E00U)

#define SD_STATE_IDLE  ((ui8)0U)
#define SD_STATE_READY ((ui8)1U)
#define SD_STATE_IDENT ((ui8)2U)
#define SD_STATE_STBY  ((ui8)3U)
#define SD_STATE_TRAN  ((ui8)4U)
#define SD_STATE_DATA  ((ui8)5U)
#define SD_STATE_RCV   ((ui8)6U)
#define SD_STATE_PRG   ((ui8)7U)
#define SD_STATE_DIS   ((ui8)8U)


typedef struct
{
 ui8  CSDStruct;            /*!< CSD structure                         */
 ui8  SysSpecVersion;       /*!< System specification version          */
 ui8  Reserved1;            /*!< Reserved                              */
 ui8  TAAC;                 /*!< Data read access time 1               */
 ui8  NSAC;                 /*!< Data read access time 2 in CLK cycles */
 ui8  MaxBusClkFrec;        /*!< Max. bus clock frequency              */
 ui16 CardComdClasses;      /*!< Card command classes                  */
 ui8  RdBlockLen;           /*!< Max. read data block length           */
 ui8  PartBlockRead;        /*!< Partial blocks for read allowed       */
 ui8  WrBlockMisalign;      /*!< Write block misalignment              */
 ui8  RdBlockMisalign;      /*!< Read block misalignment               */
 ui8  DSRImpl;              /*!< DSR implemented                       */
 ui8  Reserved2;            /*!< Reserved                              */
 ui32 DeviceSize;           /*!< Device Size                           */
 ui8  MaxRdCurrentVDDMin;   /*!< Max. read current @ VDD min           */
 ui8  MaxRdCurrentVDDMax;   /*!< Max. read current @ VDD max           */
 ui8  MaxWrCurrentVDDMin;   /*!< Max. write current @ VDD min          */
 ui8  MaxWrCurrentVDDMax;   /*!< Max. write current @ VDD max          */
 ui8  DeviceSizeMul;        /*!< Device size multiplier                */
 ui8  EraseGrSize;          /*!< Erase group size                      */
 ui8  EraseGrMul;           /*!< Erase group size multiplier           */
 ui8  WrProtectGrSize;      /*!< Write protect group size              */
 ui8  WrProtectGrEnable;    /*!< Write protect group enable            */
 ui8  ManDeflECC;           /*!< Manufacturer default ECC              */
 ui8  WrSpeedFact;          /*!< Write speed factor                    */
 ui8  MaxWrBlockLen;        /*!< Max. write data block length          */
 ui8  WriteBlockPaPartial;  /*!< Partial blocks for write allowed      */
 ui8  Reserved3;            /*!< Reserved                              */
 ui8  ContentProtectAppli;  /*!< Content protection application        */
 ui8  FileFormatGrouop;     /*!< File format group                     */
 ui8  CopyFlag;             /*!< Copy flag (OTP)                       */
 ui8  PermWrProtect;        /*!< Permanent write protection            */
 ui8  TempWrProtect;        /*!< Temporary write protection            */
 ui8  FileFormat;           /*!< File format                           */
 ui8  ECC;                  /*!< ECC code                              */
 ui8  CSD_CRC;              /*!< CSD CRC                               */
 ui8  Reserved4;            /*!< Always 1                              */
} sdsCSD;


struct
{
 ui32 CardType;
 ui32 CardVersion;
 ui32 Class;
 ui32 RelCardAdd;
 ui32 BlockNbr;
 ui32 BlockSize;
 ui32 LogBlockNbr;
 ui32 LogBlockSize;
 ui32 CardSpeed;
 ui8  Enable;
 ui8  State;


 ui8  ManufacturerID;  // Manufacturer ID     
 ui16 OEM_AppliID;     // OEM/Application ID  
 ui32 ProdName1;       // Product Name part1  
 ui8  ProdName2;       // Product Name part2  
 ui8  ProdRev;         // Product Revision    
 ui32 ProdSN;          // Product Serial Number
 ui8  Reserved1;       // Reserved1           
 ui16 ManufactDate;    // Manufacturing Date  
 ui8  CID_CRC;         // CID CRC               
 ui8  Reserved2;       // Always 1              
 
 sdsCSD CSD;
} SDCard;


ui32 SDResp[4];
extern ui32 MainTick;


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
ui32 SDSendCommand(ui32 command, ui32 argument, ui32 respType)
{
 ui32 response;
 ui32 flags;
 __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS);
 
 switch ( respType )
  {
   case  RESP_NO :                 //No response
     response = 0x00000000;
     flags = SDMMC_FLAG_CMDSENT;
   break;
   case  RESP_R1 :
   case  RESP_R6 :
   case  RESP_R7 :
     response = 0x00000100;       //Short response
     flags = SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CMDREND;
   break;
   case  RESP_R1b :
   case  RESP_R3 :
     response = 0x00000200;       //Short response no CRC
     flags = SDMMC_FLAG_CMDREND;
   break;
   case  RESP_R2 :
     response = 0x00000300;       //Long response
     flags = SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CMDREND;
   break;
   //assert
  }
 
 SDMMC1->ARG = argument;
 MODIFY_REG(SDMMC1->CMD, CMD_CLEAR_MASK, command | response | SDMMC_WAIT_NO | SDMMC_CPSM_ENABLE); 
 
 ui32 cnt = 100;
 ui32 resFlags = 0;

 while ( 1 )
  {
   if (cnt-- == 0)
    {__SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS);        //Clear flags
     return(SDMMC_ERROR_TIMEOUT);
    }
   resFlags = SDMMC1->STA & flags;
   
   if ( resFlags & SDMMC_FLAG_CCRCFAIL )
    {__SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS);        //Clear flags
     return(SDMMC_ERROR_CMD_CRC_FAIL);
    } 
   if ( resFlags ) 
     break;
   LL_mDelay(10);  
  } 

 __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS);        //Clear flags
 
 SDResp[0] = SDMMC1->RESP1;
 SDResp[1] = SDMMC1->RESP2;
 SDResp[2] = SDMMC1->RESP3;
 SDResp[3] = SDMMC1->RESP4;

 if ( respType == RESP_R1 )
   SDCard.State = ( SDResp[0] & SD_STATE_MASK ) >> 9; 

 return(SDMMC_ERROR_NONE);
}
//-----------------------------------------------------------------------------------------------------
void SDWriteFIFO(uint32_t *pWriteData)
{ 
 SDMMC1->FIFO = *pWriteData;
}
//-----------------------------------------------------------------------------------------------------
ui32 SDReadFIFO()
{
 return (SDMMC1->FIFO);
}
//-----------------------------------------------------------------------------------------------------
void SDGetCardCID(ui32 cid[4])
{
 ui32 tmp;
  
 tmp = (ui8)((cid[0] & 0xFF000000U) >> 24);
 SDCard.ManufacturerID = tmp;
  
 tmp = (uint8_t)((cid[0] & 0x00FF0000) >> 16);
 SDCard.OEM_AppliID = tmp << 8;
  
 tmp = (uint8_t)((cid[0] & 0x0000FF00) >> 8);
 SDCard.OEM_AppliID |= tmp;
  
 tmp = (uint8_t)(cid[0] & 0x000000FF);
 SDCard.ProdName1 = tmp << 24;
  
 tmp = (uint8_t)((cid[1] & 0xFF000000U) >> 24);
 SDCard.ProdName1 |= tmp << 16;
  
 tmp = (uint8_t)((cid[1] & 0x00FF0000) >> 16);
 SDCard.ProdName1 |= tmp << 8;
  
 tmp = (uint8_t)((cid[1] & 0x0000FF00) >> 8);
 SDCard.ProdName1 |= tmp;
  
 tmp = (uint8_t)(cid[1] & 0x000000FF);
 SDCard.ProdName2 = tmp;
  
 tmp = (uint8_t)((cid[2] & 0xFF000000U) >> 24);
 SDCard.ProdRev = tmp;
  
 tmp = (uint8_t)((cid[2] & 0x00FF0000) >> 16);
 SDCard.ProdSN = tmp << 24;
  
 tmp = (uint8_t)((cid[2] & 0x0000FF00) >> 8);
 SDCard.ProdSN |= tmp << 16;
  
 tmp = (uint8_t)(cid[2] & 0x000000FF);
 SDCard.ProdSN |= tmp << 8;
  
 tmp = (uint8_t)((cid[3] & 0xFF000000U) >> 24);
 SDCard.ProdSN |= tmp;
  
 tmp = (uint8_t)((cid[3] & 0x00FF0000) >> 16);
 SDCard.Reserved1   |= (tmp & 0xF0) >> 4;
 SDCard.ManufactDate = (tmp & 0x0F) << 8;
  
 tmp = (uint8_t)((cid[3] & 0x0000FF00) >> 8);
 SDCard.ManufactDate |= tmp;
  
 tmp = (uint8_t)(cid[3] & 0x000000FF);
 SDCard.CID_CRC   = (tmp & 0xFE) >> 1;
 SDCard.Reserved2 = 1;
}
//-----------------------------------------------------------------------------------------------------
ui32 SDSetCardCSD(ui32 csd[4])
{
 ui32 tmp;
  
 /* Byte 0 */
 tmp = (csd[0] & 0xFF000000U) >> 24;
 SDCard.CSD.CSDStruct = (uint8_t)((tmp & 0xC0) >> 6);
 SDCard.CSD.SysSpecVersion = (uint8_t)((tmp & 0x3C) >> 2);
 SDCard.CSD.Reserved1      = tmp & 0x03;
  
  /* Byte 1 */
 tmp = (csd[0] & 0x00FF0000) >> 16;
 SDCard.CSD.TAAC = (uint8_t)tmp;
  
  /* Byte 2 */
 tmp = (csd[0] & 0x0000FF00) >> 8;
 SDCard.CSD.NSAC = (uint8_t)tmp;
  
  /* Byte 3 */
 tmp = csd[0] & 0x000000FF;
 SDCard.CSD.MaxBusClkFrec = (uint8_t)tmp;
  
  /* Byte 4 */
 tmp = (csd[1] & 0xFF000000U) >> 24;
 SDCard.CSD.CardComdClasses = (uint16_t)(tmp << 4);
  
  /* Byte 5 */
 tmp = (csd[1] & 0x00FF0000U) >> 16;
 SDCard.CSD.CardComdClasses |= (uint16_t)((tmp & 0xF0) >> 4);
 SDCard.CSD.RdBlockLen       = (uint8_t)(tmp & 0x0F);
  
  /* Byte 6 */
 tmp = (csd[1] & 0x0000FF00U) >> 8;
 SDCard.CSD.PartBlockRead   = (uint8_t)((tmp & 0x80) >> 7);
 SDCard.CSD.WrBlockMisalign = (uint8_t)((tmp & 0x40) >> 6);
 SDCard.CSD.RdBlockMisalign = (uint8_t)((tmp & 0x20) >> 5);
 SDCard.CSD.DSRImpl         = (uint8_t)((tmp & 0x10) >> 4);
 SDCard.CSD.Reserved2       = 0; /*!< Reserved */
  
 if (SDCard.CardType == CARD_SDSC)
  {
   SDCard.CSD.DeviceSize = (tmp & 0x03) << 10;
    
   /* Byte 7 */
   tmp = (uint8_t)(csd[1] & 0x000000FFU);
   SDCard.CSD.DeviceSize |= (tmp) << 2;
    
   /* Byte 8 */
   tmp = (uint8_t)((csd[2] & 0xFF000000U) >> 24);
   SDCard.CSD.DeviceSize |= (tmp & 0xC0) >> 6;
    
   SDCard.CSD.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
   SDCard.CSD.MaxRdCurrentVDDMax = (tmp & 0x07);
    
   /* Byte 9 */
   tmp = (uint8_t)((csd[2] & 0x00FF0000U) >> 16);
   SDCard.CSD.MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
   SDCard.CSD.MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
   SDCard.CSD.DeviceSizeMul      = (tmp & 0x03) << 1;
    /* Byte 10 */
   tmp = (uint8_t)((csd[2] & 0x0000FF00U) >> 8);
   SDCard.CSD.DeviceSizeMul |= (tmp & 0x80) >> 7;
    
   SDCard.BlockNbr  = (SDCard.CSD.DeviceSize + 1) ;
   SDCard.BlockNbr *= (1 << (SDCard.CSD.DeviceSizeMul + 2));
   SDCard.BlockSize = 1 << (SDCard.CSD.RdBlockLen);

   SDCard.LogBlockNbr =  (SDCard.BlockNbr) * ((SDCard.BlockSize) / BLOCKSIZE); 
   SDCard.LogBlockSize = BLOCKSIZE;
  }
 else if ( SDCard.CardType == CARD_SDHC_SDXC )
  {
   /* Byte 7 */
   tmp = (uint8_t)(csd[1] & 0x000000FFU);
   SDCard.CSD.DeviceSize = (tmp & 0x3F) << 16;
    
   /* Byte 8 */
   tmp = (uint8_t)((csd[2] & 0xFF000000U) >> 24);
    
   SDCard.CSD.DeviceSize |= (tmp << 8);
    
   /* Byte 9 */
   tmp = (uint8_t)((csd[2] & 0x00FF0000U) >> 16);
   SDCard.CSD.DeviceSize |= (tmp);
    
   /* Byte 10 */
   tmp = (uint8_t)((csd[2] & 0x0000FF00U) >> 8);
    
   SDCard.LogBlockNbr  = SDCard.BlockNbr = (((uint64_t)SDCard.CSD.DeviceSize + 1) * 1024);
   SDCard.LogBlockSize = SDCard.BlockSize = BLOCKSIZE;
  }
 else
  {
   __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS);   
   return(1);
  }
  
  SDCard.CSD.EraseGrSize = (tmp & 0x40) >> 6;
  SDCard.CSD.EraseGrMul  = (tmp & 0x3F) << 1;
  
  /* Byte 11 */
  tmp = (uint8_t)(csd[2] & 0x000000FF);
  SDCard.CSD.EraseGrMul     |= (tmp & 0x80) >> 7;
  SDCard.CSD.WrProtectGrSize = (tmp & 0x7F);
  
  /* Byte 12 */
  tmp = (uint8_t)((csd[3] & 0xFF000000U) >> 24);
  SDCard.CSD.WrProtectGrEnable = (tmp & 0x80) >> 7;
  SDCard.CSD.ManDeflECC        = (tmp & 0x60) >> 5;
  SDCard.CSD.WrSpeedFact       = (tmp & 0x1C) >> 2;
  SDCard.CSD.MaxWrBlockLen     = (tmp & 0x03) << 2;
  
  /* Byte 13 */
  tmp = (uint8_t)((csd[3] & 0x00FF0000) >> 16);
  SDCard.CSD.MaxWrBlockLen      |= (tmp & 0xC0) >> 6;
  SDCard.CSD.WriteBlockPaPartial = (tmp & 0x20) >> 5;
  SDCard.CSD.Reserved3           = 0;
  SDCard.CSD.ContentProtectAppli = (tmp & 0x01);
  
  /* Byte 14 */
  tmp = (uint8_t)((csd[3] & 0x0000FF00) >> 8);
  SDCard.CSD.FileFormatGrouop = (tmp & 0x80) >> 7;
  SDCard.CSD.CopyFlag         = (tmp & 0x40) >> 6;
  SDCard.CSD.PermWrProtect    = (tmp & 0x20) >> 5;
  SDCard.CSD.TempWrProtect    = (tmp & 0x10) >> 4;
  SDCard.CSD.FileFormat       = (tmp & 0x0C) >> 2;
  SDCard.CSD.ECC              = (tmp & 0x03);
  
  /* Byte 15 */
  tmp = (uint8_t)(csd[3] & 0x000000FF);
  SDCard.CSD.CSD_CRC   = (tmp & 0xFE) >> 1;
  SDCard.CSD.Reserved4 = 1;
  return(SDMMC_ERROR_NONE);
}
//-----------------------------------------------------------------------------------------------------
ui32 SDWaitState(ui8 sdState)
{
 ui32 Timeout = SDMMC_CMDTIMEOUT;
 ui32 errorstate;
 ui32 tickstart = MainTick;
 
 while ( 1 )
  {
   if ( SDSendCommand(SDMMC_CMD_SEND_STATUS, SDCard.RelCardAdd, RESP_R1) )
    {errorstate = SDMMC_ERROR_CMD_RSP_TIMEOUT;
     break;
    } 
   if ( (MainTick - tickstart) >=  Timeout ) 
    {errorstate = SDMMC_ERROR_CMD_RSP_TIMEOUT;
     break;
    }
   if ( SDCard.State == sdState )
    {errorstate = SDMMC_ERROR_NONE;
     break;
    }
   LL_mDelay(10); 
  }
 __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS);
 return SDMMC_ERROR_NONE;
} 
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
ui32 SDInit()
{
 SDCard.Enable = false;
 LL_GPIO_InitTypeDef GPIO_InitStruct;

 SET_BIT(RCC->PLLCFGR,  RCC_PLLCFGR_PLLPEN);                                      //Enable PLLP;
 MODIFY_REG(RCC->CCIPR2,RCC_CCIPR2_SDMMCSEL, LL_RCC_SDMMC1_KERNELCLKSOURCE_PLLP); //Select PLLP as Clk for SDMMC

 __IO uint32_t tmpreg;
 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_SDMMC1EN);             // SD_CLK_ENABLE();
 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_SDMMC1EN);   // Delay after an RCC peripheral clock enabling
 //UNUSED(tmpreg);

 /**SDMMC1 GPIO Configuration    
  PC8     ------> SDMMC1_D0
  PC9     ------> SDMMC1_D1
  PC10     ------> SDMMC1_D2
  PC11     ------> SDMMC1_D3
  PC12     ------> SDMMC1_CK
  PD2     ------> SDMMC1_CMD 
 */
 GPIO_InitStruct.Pin       = LL_GPIO_PIN_8|LL_GPIO_PIN_9|LL_GPIO_PIN_10|LL_GPIO_PIN_11|LL_GPIO_PIN_12;
 GPIO_InitStruct.Mode      = LL_GPIO_MODE_ALTERNATE;
 GPIO_InitStruct.Pull      = LL_GPIO_PULL_NO;
 GPIO_InitStruct.Speed     = LL_GPIO_SPEED_FREQ_VERY_HIGH;
 GPIO_InitStruct.Alternate = LL_GPIO_AF_12;
 LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

 GPIO_InitStruct.Pin       = LL_GPIO_PIN_2;
 GPIO_InitStruct.Mode      = LL_GPIO_MODE_ALTERNATE;
 GPIO_InitStruct.Pull      = LL_GPIO_PULL_NO;
 GPIO_InitStruct.Speed     = LL_GPIO_SPEED_FREQ_VERY_HIGH;
 GPIO_InitStruct.Alternate = LL_GPIO_AF_12;
 LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  
 // Initialize SDMMC peripheral interface with default configuration
 MODIFY_REG(SDMMC1->CLKCR, CLKCR_CLEAR_MASK, SDMMC_CLOCK_EDGE_RISING              |\
                                             SDMMC_CLOCK_POWER_SAVE_DISABLE       |\
                                             SDMMC_BUS_WIDE_1B                    |\
                                             SDMMC_HARDWARE_FLOW_CONTROL_DISABLE  |\
                                             SDMMC_INIT_CLK_DIV);  
 

 SDMMC1->CLKCR &= ~SDMMC_CLKCR_CLKEN;     // Clk SD_Disable
 SDMMC1->POWER |= SDMMC_POWER_PWRCTRL;    // PowerState ON
 SDMMC1->CLKCR |= SDMMC_CLKCR_CLKEN;      // Clk SD_ENABLE

 LL_mDelay(2);

 __IO uint32_t cnt = 0;
 
 if ( SDSendCommand(SDMMC_CMD_GO_IDLE_STATE, 0, RESP_NO) )            // CMD0: SDMMC_CMD_GO_IDLE_STATE
   return(1);

 if ( SDSendCommand(SDMMC_CMD_HS_SEND_EXT_CSD, 0x1AA, RESP_R7) )        // Check type: Command available only on V2.0 cards
  {
   SDCard.CardVersion = 1;                    //SDMMC_ERROR_TIMEOUT or error
   while ( 1 )
    {
     if ( cnt++ > 1000)
       return(1);
 
     if ( SDSendCommand(SDMMC_CMD_APP_CMD, 0, RESP_R1) ) // SDMMC_CMD_APP_CMD with RCA as 0
       return(1);
      
     if ( SDSendCommand(SDMMC_CMD_SD_APP_OP_COND, SDMMC_STD_CAPACITY | 0x00020000, RESP_R3) )
       return(1);

     if ( SDResp[0] & 0x80000000 )
      {SDCard.CardType = CARD_SDSC;
       break;
      } 

     LL_mDelay(1);        
      
    }
  }
 else  
  {
   SDCard.CardVersion = 2;
   while( 1 )
    {
     if ( cnt++ > 1000 )                      // wait 1 sec for answer
       return(1);
      
     if ( SDSendCommand(SDMMC_CMD_APP_CMD, 0, RESP_R1) ) // SEND SDMMC_CMD_APP_CMD APP_CMD with RCA as 0
       return(1);
      
     if ( SDSendCommand(SDMMC_CMD_SD_APP_OP_COND, SDMMC_HIGH_CAPACITY | 0x00020000, RESP_R3) )
       return(1);     
  
     // Get operating voltage
     if ( SDResp[0] & 0x80000000 )
      {if ( (SDResp[0] & 0x40000000U ) == SDMMC_HIGH_CAPACITY ) // (response &= SD_HIGH_CAPACITY) 
         SDCard.CardType = CARD_SDHC_SDXC;
       else
         SDCard.CardType = CARD_SDSC;
       break;
      }
     LL_mDelay(50);        
    }
  } 

 // Get Card identification number data
 if ( SDSendCommand(SDMMC_CMD_ALL_SEND_CID, 0, RESP_R2) )
   return(1);
 SDGetCardCID(SDResp);
  
 // Get card adress - RCA 
 if ( SDSendCommand(SDMMC_CMD_SET_REL_ADDR, 0, RESP_R6) )
   return(1);
 SDCard.RelCardAdd = ( SDResp[0] & (0xFFFF0000) );      //RCA

 if ( SDSendCommand(SDMMC_CMD_SEND_CSD, SDCard.RelCardAdd, RESP_R2) )
   return(1);
 SDSetCardCSD(SDResp);

 if ( SDSendCommand(SDMMC_CMD_SEL_DESEL_CARD, SDCard.RelCardAdd, RESP_R1b) )
   return(1);

 // Configure the bus wide
 if ( SDSendCommand(SDMMC_CMD_APP_CMD, SDCard.RelCardAdd, RESP_R1) ) // SDMMC_CMD_APP_CMD with RCA
   return(1);
 if ( SDSendCommand(SDMMC_CMD_APP_SD_SET_BUSWIDTH, 2, RESP_R1) )
   return(1);
 
 if ( SDResp[0] != 0x920)
   return(1);

 // Initialize SDMMC peripheral interface with default configuration
 MODIFY_REG(SDMMC1->CLKCR, CLKCR_CLEAR_MASK, SDMMC_CLOCK_EDGE_RISING              |\
                                             SDMMC_CLOCK_POWER_SAVE_DISABLE       |\
                                             SDMMC_BUS_WIDE_4B                    |\
                                             SDMMC_HARDWARE_FLOW_CONTROL_DISABLE  |\
                                             1);  //SDMMC_INIT_CLK_DIV
 
 SDCard.Enable = true;
 return(SDMMC_ERROR_NONE); 
}
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
ui32 SDWriteBlocks(const ui8 *pData, ui32 BlockAdd, ui32 NumberOfBlocks)
{
 if ( !SDCard.Enable )
  return(1);

 ui32 Timeout = SDMMC_CMDTIMEOUT;
 //SDMMC_DataInitTypeDef config;
 uint32_t errorstate;
 uint32_t tickstart = MainTick;
 uint32_t count = 0;
 uint32_t *tempbuff = (uint32_t *)pData;
 
 if((BlockAdd + NumberOfBlocks) > (SDCard.LogBlockNbr))
   return(1);
    
 // Initialize data control register
 SDMMC1->DCTRL = 0;
     
 if ( SDCard.CardType != CARD_SDHC_SDXC)
   BlockAdd *= BLOCKSIZE;
    
 errorstate = SDSendCommand(SDMMC_CMD_SET_BLOCKLEN, BLOCKSIZE, RESP_R1);
 if ( errorstate != SDMMC_ERROR_NONE )
  {
   __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS);  
   return(1);
  }
    
 // Configure the SD DPSM (Data Path State Machine)
 SDMMC1->DTIMER = SDMMC_DATATIMEOUT; // Set the SDMMC Data TimeOut value
 SDMMC1->DLEN = NumberOfBlocks * BLOCKSIZE; // Set the SDMMC DataLength value
 // Set the SDMMC data configuration parameters
 MODIFY_REG(SDMMC1->DCTRL, DCTRL_CLEAR_MASK,SDMMC_DATABLOCK_SIZE_512B  |\
                                            SDMMC_TRANSFER_DIR_TO_CARD |\
                                            SDMMC_TRANSFER_MODE_BLOCK  |\
                                            SDMMC_DPSM_DISABLE);

 SDMMC1->CMD |= SDMMC_CMD_CMDTRANS; //CMDTRANS_ENABLE

 // Write Blocks in Polling mode
 if ( NumberOfBlocks > 1U )
   errorstate = SDMMC_CmdWriteMultiBlock(SDMMC1, BlockAdd);
 else
   errorstate = SDMMC_CmdWriteSingleBlock(SDMMC1, BlockAdd);

 if ( errorstate != SDMMC_ERROR_NONE )
  {
   __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS);  
   return(1);
  }
    
 // Write block(s) in polling mode
  while(!__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_TXUNDERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DATAEND))
   {
    if(__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_TXFIFOHE))
     {
      for(count = 0U; count < 8U; count++)
        SDWriteFIFO(tempbuff + count);

      tempbuff += 8U;
     }
      
    if ( (MainTick - tickstart) >=  Timeout  )
     {
      __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS);  
      return(1);
     }
   }
// __SDMMC_CMDTRANS_DISABLE(SDMMC1);
 SDMMC1->CMD &= ~SDMMC_CMD_CMDTRANS;


 // Send stop transmission command in case of multiblock write
 if(__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_DATAEND) && (NumberOfBlocks > 1U))
  { 
   // Send stop transmission command
   errorstate = SDMMC_CmdStopTransfer(SDMMC1);
   if ( errorstate != SDMMC_ERROR_NONE )
    {
     __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS);  
     return(1);
    }
  }

  // Get error state
 if( __SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_TXUNDERR) )
  {
   __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS);
   return(1);
  }
 __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS);
 
 errorstate = SDWaitState(SD_STATE_TRAN);
 return(errorstate);
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
ui32 SDReadBlocks(ui8 *pData, ui32 BlockAdd, ui32 NumberOfBlocks)
{
 if ( !SDCard.Enable )
  return(1);

 ui32 Timeout = SDMMC_CMDTIMEOUT;

 uint32_t errorstate;
 uint32_t tickstart = MainTick;
 uint32_t count = 0, *tempbuff = (uint32_t *)pData;
  
 if((BlockAdd + NumberOfBlocks) > SDCard.LogBlockNbr)
   return(1);

  /* Initialize data control register */
 SDMMC1->DCTRL = 0;
    
 if ( SDCard.CardType != CARD_SDHC_SDXC )
   BlockAdd *= BLOCKSIZE;
      
 /* Set Block Size for Card */
 errorstate = SDSendCommand(SDMMC_CMD_SET_BLOCKLEN, BLOCKSIZE, RESP_R1);      //
 if ( errorstate != SDMMC_ERROR_NONE )
  {
   __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS);
   return(1);
  }
    
 // Configure the SD DPSM (Data Path State Machine)
 SDMMC1->DTIMER = SDMMC_DATATIMEOUT;          // Set the SDMMC Data TimeOut value
 SDMMC1->DLEN = NumberOfBlocks * BLOCKSIZE;   // Set the SDMMC DataLength value
                                              // Set the SDMMC data configuration parameters
 MODIFY_REG(SDMMC1->DCTRL, DCTRL_CLEAR_MASK,SDMMC_DATABLOCK_SIZE_512B   |\
                                            SDMMC_TRANSFER_DIR_TO_SDMMC |\
                                            SDMMC_TRANSFER_MODE_BLOCK   |\
                                            SDMMC_DPSM_DISABLE);

 
 SDMMC1->CMD |= SDMMC_CMD_CMDTRANS;       //CMDTRANS_ENABLE
 
 if(NumberOfBlocks > 1)
   SDMMC_CmdReadMultiBlock(SDMMC1, BlockAdd); // Read Multi Block command
 else
   errorstate = SDMMC_CmdReadSingleBlock(SDMMC1, BlockAdd); // Read Single Block command

 if ( errorstate != SDMMC_ERROR_NONE )
  {
   __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS);
   return(1);
  }
      
 /* Poll on SDMMC flags */
 while(!__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DATAEND))
  {
   if( __SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_RXFIFOHF))
    {
     /* Read data from SDMMC Rx FIFO */
     for(count = 0U; count < 8U; count++)
      {     
       *(tempbuff + count) = SDReadFIFO();
      }
     tempbuff += 8U;
    }
   if ( (MainTick - tickstart) >=  Timeout)
    {
     __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS);
     return(SDMMC_ERROR_DATA_TIMEOUT);
    }
  }
 SDMMC1->CMD &= ~SDMMC_CMD_CMDTRANS;   //CMDTRANS_DISABLE
   
 /* Send stop transmission command in case of multiblock read */
 if (__SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_DATAEND) && (NumberOfBlocks > 1U) )
  {    
   errorstate = SDMMC_CmdStopTransfer(SDMMC1);
   if ( errorstate != SDMMC_ERROR_NONE )
    {
     __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS);
     return(1);
    }
  }

 /* Get error state */
 if ( __SDMMC_GET_FLAG(SDMMC1, SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_RXOVERR) )
  {
   __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS);
   return(1);
  }
 __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_DATA_FLAGS);
 
 errorstate = SDWaitState(SD_STATE_TRAN);
 return(errorstate);
}
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
ui32 SDErase(ui32 BlockStartAdd, ui32 BlockEndAdd)
{
 ui32 Timeout = SDMMC_CMDTIMEOUT;

 uint32_t tickstart = MainTick;
 uint32_t errorstate;
 
 if ( !SDCard.Enable )
  return(1); 

 if ( BlockEndAdd < BlockStartAdd )
   return(1);
    
 if ( BlockEndAdd > SDCard.LogBlockNbr )
   return (1);
    
   
 /*
 // Check if the card command class supports erase command
 if (  (SDCard.Class & SDMMC_CCCC_ERASE) == 0U)
  {
   __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS);
   return (1);
  }
 */   
 if ( (SDMMC_GetResponse(SDMMC1, SDMMC_RESP1) & SDMMC_CARD_LOCKED) == SDMMC_CARD_LOCKED )
  {
   __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS);  
   return (1);
  }
    
 // Get start and end block for high capacity cards
 if(SDCard.CardType != CARD_SDHC_SDXC)
  {
   BlockStartAdd *= BLOCKSIZE;
   BlockEndAdd   *= BLOCKSIZE;
  }
    
 errorstate = SDSendCommand(SDMMC_CMD_SD_ERASE_GRP_START, BlockStartAdd, RESP_R1);
 if ( errorstate != SDMMC_ERROR_NONE )
  {
   __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS); 
   return (1);
  }
      
 errorstate = SDSendCommand(SDMMC_CMD_SD_ERASE_GRP_END, BlockEndAdd, RESP_R1);
 if ( errorstate != SDMMC_ERROR_NONE)
  {
   __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS); 
   return (1);
  }

 errorstate = SDSendCommand(SDMMC_CMD_ERASE, 0, RESP_R1b);
 if(errorstate != SDMMC_ERROR_NONE)
  {
   __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS); 
   return (1);
  }
 __SDMMC_CLEAR_FLAG(SDMMC1, SDMMC_STATIC_FLAGS); 
 
 
 errorstate = SDWaitState(SD_STATE_TRAN);
 return(errorstate);
}
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
