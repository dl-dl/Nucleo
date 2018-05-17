#include "SPI.h"
#include "types.h"
#include "SX1278.h"

#define sxPower 0xFF   
//0xFF  0xFC, 0xF9  0xF6
//20dbm 17dbm 14dbm 11dbm

//#define sxBandWide 0x50
#define sxBandWide 0x70

//0x00   0x10    0x20    0x30    0x40    0x50    0x60    0x70   0x80   0x90
//7.8KHz 10.4KHz 15.6KHz 20.8KHz 31.2KHz 41.7KHz 62.5KHz 125KHz 250KHz 500KHz

#define sxSFactor 0xC0
// 0x60 0x70 0x80 0x90 0xA0 0xB0 0xC0
// 6    7    8    9    10   11   12

#define sxCR 0x08
// 2   4   6   8
// 4/5 4/6 4/7 4/8

#define sxCRC 0x04
//4           0
//CRC Enable  CRC Disable

#define sxExplHeader 0
// #define sxImplHeader 1


ui8 RadioBuffer[128];
ui8 RadioRegim;
ui8 RadioStatus;
ui8 RadioReg;
si8 RadioRxRSSI;
ui32 RadioTimeOut;


//-----------------------------------------------------------------------------------------
#define SPI3DR8 *(ui8*)(&SPI3->DR)
void RadioWrite(ui8 regAddr, ui8 regData)
{
 LL_GPIO_ResetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);
// LL_GPIO_ResetOutputPin(CS_DOP_GPIO_Port, CS_DOP_Pin);

 SPI3DR8 = regAddr | 0x80;

 while ( (SPI3->SR & SPI_SR_TXE) != SPI_SR_TXE)     // Control TX fifo is empty
  {}
 SPI3DR8 = regData;

 while ( SPI3->SR & SPI_SR_BSY )                      // Control the BSY flag
  {}

 LL_GPIO_SetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);  
// LL_GPIO_SetOutputPin(CS_DOP_GPIO_Port, CS_DOP_Pin);
}
//---------------------------------------------------------------------------------------------------
void RadioWrites(ui8 regAddr, ui8 *regData, ui8 len)
{
 LL_GPIO_ResetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);

 SPI3DR8 = regAddr | 0x80;

 for (ui8 cnt = 0; cnt < len; cnt++ )
  {
   while ( (SPI3->SR & SPI_SR_TXE) != SPI_SR_TXE)     // Control TX fifo is empty
    {}   
   SPI3DR8 = *regData++;
  }  

 while ( SPI3->SR & SPI_SR_BSY )                      // Control the BSY flag
  {}

 LL_GPIO_SetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin); 
}
//---------------------------------------------------------------------------------------------------/* USER CODE END 0 */
ui8 RadioRead(ui8 regAddr)
{
 while ( SPI3->SR & SPI_SR_RXNE )                      // Clear FIFO
  {ui32 tmp = SPI3->DR;
  }

 LL_GPIO_ResetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);
// LL_GPIO_ResetOutputPin(CS_DOP_GPIO_Port, CS_DOP_Pin);

 SPI3DR8 = regAddr;
 while ( SPI3->SR & SPI_SR_BSY )                      // Control the BSY flag
  {}
 ui8 regVal = SPI3->DR;

 SPI3DR8 = 0;
 while ( SPI3->SR & SPI_SR_BSY )                      // Control the BSY flag
  {}
 regVal = SPI3->DR;

 LL_GPIO_SetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);  
// LL_GPIO_SetOutputPin(CS_DOP_GPIO_Port, CS_DOP_Pin);
 return(regVal);  
}
//---------------------------------------------------------------------------------------------------
void RadioReads(ui8 regAddr, ui8 *regData, ui8 len)
{
 while ( SPI3->SR & SPI_SR_RXNE )                      // Control the BSY flag
  {ui32 tmp = SPI3->DR;
  }

 LL_GPIO_ResetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);

 SPI3DR8 = regAddr;
 while ( SPI3->SR & SPI_SR_BSY )  
  {}
 ui8 regVal = SPI3->DR;

 for (int cnt = 0 ; cnt < len; cnt++)
  {
   SPI3DR8 = 0;
   while ( SPI3->SR & SPI_SR_BSY )
    {}
   *regData++ = SPI3->DR;
  } 

 LL_GPIO_SetOutputPin(RADIO_CS_GPIO_Port, RADIO_CS_Pin);
}
//---------------------------------------------------------------------------------------------------
void RadioStandby()
{
 RadioWrite(LR_RegOpMode,0x09);    //Standby Low Frequency Mode
}
//-----------------------------------------------------------------------------------------
void RadioSleep()
{
 RadioWrite(LR_RegOpMode,0x08); //Sleep Low Frequency Mode
} 
//-----------------------------------------------------------------------------------------
si8 RadioRSSI()
{
 si16 temp = -154;
 temp += RadioRead(LR_RegPktRssiValue);
 return(temp);
} 
//-----------------------------------------------------------------------------------------
void RadioLoRaClearIrq(void)
{
 RadioWrite(LR_RegIrqFlags,0xFF);
}
//-----------------------------------------------------------------------------------------
ui8 sxFreqTbl[3] = {0x6C, 0x80, 0x00}; //434MHz  

void RadioInit()
{
 RadioRegim = rgRadioIdle;
 
 LL_GPIO_ResetOutputPin(rReset_GPIO_Port, rReset_Pin);
 LL_mDelay(1);
 LL_GPIO_SetOutputPin(rReset_GPIO_Port, rReset_Pin);
 LL_mDelay(1);

 RadioSleep();                                    
 LL_mDelay(15);
 
 RadioWrite(LR_RegOpMode,0x88);                     //->LoRa mode, Low Frequency Mode

 RadioWrites(LR_RegFrMsb,sxFreqTbl,3);              //Set frequency
 RadioWrite(LR_RegPaConfig,sxPower);                //Set output power
 RadioWrite(LR_RegOcp,0x3F);                        //00(x) 1(OCPon) 11111(I) Imax=45+31*5 = 200 ma
 RadioWrite(LR_RegLna,0x23);                        //001(Gain=Max) 000(x) 11(Boost LNA)

 RadioWrite(LR_RegModemConfig1,(sxBandWide+sxCR+sxExplHeader));  //BandWide, Error Coding rate, Explicit header
 RadioWrite(LR_RegModemConfig2,(sxSFactor+sxCRC+3));  //SFactor,Normal mode(Packet),CRC,Rx TimeOut MSB
 RadioWrite(LR_RegSymbTimeoutLsb,0xFF);             //Timeout Lsb = 0xFF(Max)

 RadioWrite(LR_RegPreambleMsb,0x00);                //Preamble length MSB,
 RadioWrite(LR_RegPreambleLsb,12);                  //Preamble length LSB 12+4=16 byte
 RadioWrite(REG_LR_DIOMAPPING2,0x01);               //RegDioMapping2 DIO5=00, DIO4=01
 RadioStandby();                                  //Entry standby mode
} 
//-----------------------------------------------------------------------------------------
unsigned char RadioRxOn(void)
{
 unsigned char addr;
 unsigned char cnt;
 RadioTimeOut = 0;

 RadioWrite(REG_LR_PADAC,0x84);             //Normal and Rx
 RadioWrite(LR_RegHopPeriod,0xFF);          //RegHopPeriod NO FHSS
 
 RadioWrite(REG_LR_DIOMAPPING1,0x01);       //DIO0=00 RxDone, DIO1=00 RxTimeout, DIO2=00, DIO3=01 ValidHeader

 RadioWrite(LR_RegIrqFlagsMask,0x1F);       //Enable: RxTimeOut RxDone PayloadCrcError
 
 RadioLoRaClearIrq();
 RadioWrite(LR_RegPayloadLength,21);        //RegPayloadLength 21byte(this register must difine when the data long of one byte in SF is 6)
 addr = RadioRead(LR_RegFifoRxBaseAddr);    //Read RxBaseAddr
 RadioWrite(LR_RegFifoAddrPtr,addr);        //RxBaseAddr ->FiFoAddrPtr
 RadioWrite(LR_RegOpMode,0x8d);             //LoRa Mode, Receive continuous, Low Frequency
 
 for (cnt = 0; cnt < 10; cnt++)
  {
   if ( ( RadioRead(LR_RegModemStat) & 0x04 ) == 0x04 ) //Rx-on going RegModemStat
    {RadioRegim = rgRadioRx;
     return(1);
    } 
   LL_mDelay(10);   
  }

 return(0); 
}
//-----------------------------------------------------------------------------------------
ui8 RadioRxLen()
{return(RadioRead(LR_RegRxNbBytes));    //Number for received bytes
}
//-----------------------------------------------------------------------------------------
ui8 RadioReadPacket(unsigned char *rxData, unsigned char maxLen)
{
 if ( !(GPIOG->IDR & DIO0_Pin) ) return(0);

 RadioTimeOut = 0;

 ui8 addr = RadioRead(LR_RegFifoRxCurrentaddr);     //last packet addr
 RadioWrite(LR_RegFifoAddrPtr,addr);            //RxBaseAddr ->FiFoAddrPtr
   
 ui8 rxLen = RadioRead(LR_RegRxNbBytes);    //Number for received bytes
 if ( rxLen > maxLen )
   rxLen = maxLen;
 
 RadioReads(0x00, rxData, rxLen);
 
 RadioStatus = RadioRead(LR_RegIrqFlags);
 RadioLoRaClearIrq();
 return(rxLen);
} 
//-----------------------------------------------------------------------------------------
void RadioTxPacket(ui8 *txData, ui8 len)
{
 RadioTimeOut = 0;

 RadioWrite(REG_LR_PADAC,0x84);               // Tx for 0x87 = 20dBm, 0x84 = 17dBm
 RadioWrite(LR_RegHopPeriod,0x00);            // RegHopPeriod NO FHSS
 RadioWrite(REG_LR_DIOMAPPING1,0x41);         // DIO0=01 TxDone, DIO1=00,DIO2=00, DIO3=01
 
 RadioLoRaClearIrq();
 RadioWrite(LR_RegIrqFlagsMask,0xF7);         // Only TxDone interrupt
 
 RadioWrite(LR_RegPayloadLength,len);         // RegPayloadLength21byte
 
 ui8 addr = RadioRead(LR_RegFifoTxBaseAddr);      // RegFiFoTxBaseAddr
 
 RadioWrite(LR_RegFifoAddrPtr,addr);          // RegFifoAddrPtr

 RadioWrites(0x00, txData, len);

 RadioWrite(LR_RegOpMode,0x8b);               // Tx Mode On

 RadioRegim = rgRadioTx;
}
//-----------------------------------------------------------------------------------------
