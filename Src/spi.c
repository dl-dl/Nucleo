/**
  ******************************************************************************
  * File Name          : SPI.c
  * Description        : This file provides code for the configuration
  *                      of the SPI instances.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "spi.h"

#include "gpio.h"
#include "dma.h"

/* USER CODE BEGIN 0 */
#include "types.h"

/* USER CODE END 0 */

/* SPI1 init function */
void MX_SPI1_Init(void)
{
  LL_SPI_InitTypeDef SPI_InitStruct;

  LL_GPIO_InitTypeDef GPIO_InitStruct;
  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
  
  /**SPI1 GPIO Configuration  
  PA4   ------> SPI1_NSS
  PA5   ------> SPI1_SCK
  PA6   ------> SPI1_MISO
  PA7   ------> SPI1_MOSI 
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_4|LL_GPIO_PIN_5|LL_GPIO_PIN_6|LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* SPI1 DMA Init */
  
  /* SPI1_TX Init */
  LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_3, LL_DMAMUX_REQ_SPI1_TX);

  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

  LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PRIORITY_LOW);

  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MODE_NORMAL);

  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PDATAALIGN_BYTE);

  LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MDATAALIGN_BYTE);

  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
  SPI_InitStruct.NSS = LL_SPI_NSS_HARD_OUTPUT;
  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV8;
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
  SPI_InitStruct.CRCPoly = 7;
  LL_SPI_Init(SPI1, &SPI_InitStruct);

  LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);

  LL_SPI_EnableNSSPulseMgt(SPI1);

}
/* SPI2 init function */
void MX_SPI2_Init(void)
{
  LL_SPI_InitTypeDef SPI_InitStruct;

  LL_GPIO_InitTypeDef GPIO_InitStruct;
  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
  
  /**SPI2 GPIO Configuration  
  PB13   ------> SPI2_SCK
  PB15   ------> SPI2_MOSI 
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_13|LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  SPI_InitStruct.TransferDirection = LL_SPI_HALF_DUPLEX_TX;
  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_HIGH;
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_2EDGE;
  SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV8;
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
  SPI_InitStruct.CRCPoly = 7;
  LL_SPI_Init(SPI2, &SPI_InitStruct);

  LL_SPI_SetStandard(SPI2, LL_SPI_PROTOCOL_MOTOROLA);

  LL_SPI_DisableNSSPulseMgt(SPI2);

}
/* SPI3 init function */
void MX_SPI3_Init(void)
{
  LL_SPI_InitTypeDef SPI_InitStruct;

  LL_GPIO_InitTypeDef GPIO_InitStruct;
  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI3);
  
  /**SPI3 GPIO Configuration  
  PG9   ------> SPI3_SCK
  PG10   ------> SPI3_MISO
  PG11   ------> SPI3_MOSI 
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_9|LL_GPIO_PIN_10|LL_GPIO_PIN_11;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
  LL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
  SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV8;
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
  SPI_InitStruct.CRCPoly = 7;
  LL_SPI_Init(SPI3, &SPI_InitStruct);

  LL_SPI_SetStandard(SPI3, LL_SPI_PROTOCOL_MOTOROLA);

  LL_SPI_EnableNSSPulseMgt(SPI3);

}

/* USER CODE BEGIN 1 */

//---------------------------------------------------------------------------------------------------
void SPI1Write(uint8_t *pData, uint16_t len)
{
 LL_SPI_DisableDMAReq_TX(SPI1);
 LL_DMA_DisableIT_TC(DMA1,LL_DMA_CHANNEL_3);
 SET_BIT(SPI1->CR1, SPI_CR1_BIDIOE);                  // Tx direction, clock off
 SET_BIT(SPI1->CR1, SPI_CR1_SPE);
 for (uint16_t cnt = 0; cnt < len; cnt++ )
  {
   while ( (SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE)     // Control TX fifo is empty
    {}
   LL_SPI_TransmitData8(SPI1,*pData++);
  }  
 while ( SPI1->SR & SPI_SR_BSY )                      // Control the BSY flag
  {}
 CLEAR_BIT(SPI1->CR1, SPI_CR1_SPE);                   // SPI Off
}
//---------------------------------------------------------------------------------------------------
__IO ui8 WaitSPI1 = 0;
void SPI1WriteDMA(uint8_t *pData, uint16_t len)
{
 while ( WaitSPI1 )
  {}
 WaitSPI1 = 1;
 SET_BIT(SPI1->CR1, SPI_CR1_BIDIOE);                  // Tx direction, clock off
 SET_BIT(SPI1->CR1, SPI_CR1_SPE);
 LL_SPI_EnableDMAReq_TX(SPI1);
 LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_3,(ui32)pData);
 LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t) &(SPI1->DR));
 LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, len);
 LL_DMA_EnableIT_TC(DMA1,LL_DMA_CHANNEL_3);
 LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
}
//---------------------------------------------------------------------------------------------------
void SPI1Read(uint8_t *pData, uint16_t len)
{
 /*
 CLEAR_BIT(SPI1->CR1, SPI_CR1_BIDIOE);                // Rx direction
 SET_BIT(SPI1->CR2, SPI_CR2_FRXTH);                   // 1/4 Fifo = 8-bit
 SET_BIT(SPI1->CR1, SPI_CR1_SPE);                     // SPI On
 for (uint16_t cnt = 0; cnt < len; cnt++ )
  {while ( (SPI1->SR & SPI_SR_RXNE) != SPI_SR_RXNE)
    {}
   *pData++ = SPI1->DR;
  }
 CLEAR_BIT(SPI1->CR1, SPI_CR1_SPE);                   // SPI Off
 while ( SPI1->SR & SPI_SR_BSY )          
  {}
 */ 
}
//---------------------------------------------------------------------------------------------------
void SPI2Write(uint8_t *pData, uint16_t len)
{
 SET_BIT(SPI2->CR1, SPI_CR1_BIDIOE);                  // Tx direction, clock off
 SET_BIT(SPI2->CR1, SPI_CR1_SPE);                     // SPI On
 for (uint16_t cnt = 0; cnt < len; cnt++ )
  {
   while ( (SPI2->SR & SPI_SR_TXE) != SPI_SR_TXE)     // Control TX fifo is empty
    {}
   LL_SPI_TransmitData8(SPI2,*pData++);
  }  
 while ( SPI2->SR & SPI_SR_BSY )                      // Control the BSY flag
  {}
 CLEAR_BIT(SPI2->CR1, SPI_CR1_SPE);                   // SPI Off
}
//---------------------------------------------------------------------------------------------------
void SPI2Read(uint8_t *pData, uint16_t len)
{
 while ( SPI2->SR & SPI_SR_RXNE )                      // Clear FIFO
  {ui32 tmp = SPI2->DR;
  }
 CLEAR_BIT(SPI2->CR1, SPI_CR1_BIDIOE);                // Rx direction
 SET_BIT(SPI2->CR2, SPI_CR2_FRXTH);                   // 1/4 Fifo = 8-bit
 SET_BIT(SPI2->CR1, SPI_CR1_SPE);                     // SPI On
 for (uint16_t cnt = 0; cnt < len; cnt++ )
  {while ( (SPI2->SR & SPI_SR_RXNE) != SPI_SR_RXNE)
    {}
   *pData++ = LL_SPI_ReceiveData8(SPI2);
  }
 CLEAR_BIT(SPI2->CR1, SPI_CR1_SPE);                   // SPI Off
 while ( SPI2->SR & SPI_SR_BSY )          
  {}
}
//---------------------------------------------------------------------------------------------------
/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
