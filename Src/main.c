
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "dma.h"
#include "rng.h"
#include "spi.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
//#include "usart1.h"
#include "lsm303c.h"
#include "sx1278.h"

#include "types.h"
#include "sd.h"
#include "text.h"

#include "screen.h"
#include "graph.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
ui8  MagnetoReg;
char TxBuffer[64];
ui8 RxLen;
ui8 RxData[64];
si8 *Fields[16];
ui8 GPSmobile;
ui8 GPSData[64];

ui8 RadioVersion; 

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void LL_Init(void);
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
//  SysTick_Config(SystemCoreClock / 1000);

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_RNG_Init();
  MX_SPI2_Init();
  MX_SPI3_Init();
  /* USER CODE BEGIN 2 */
 
 SET_BIT(SPI3->CR2, SPI_CR2_FRXTH);                   // 1/4 Fifo = 8-bit
 SET_BIT(SPI3->CR1, SPI_CR1_BIDIOE);                  // Tx direction, clock off
 SET_BIT(SPI3->CR1, SPI_CR1_SPE);                     // SPI3 On
 //ScreenInit(0);
 //Usart1RxOn();
 //RadioInit();
 //RadioVersion = RadioRead(REG_LR_VERSION);
  if ( SDInit() )
	{
    LL_GPIO_SetOutputPin(RedLed_GPIO_Port, RedLed_Pin);
	}
  else
	{
//		if( 0 == SDTest() )
			LL_GPIO_SetOutputPin(GreenLed_GPIO_Port, GreenLed_Pin);
	}

 GPSmobile = 0;
 RadioRxOn(); 

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
 while (1)
  {
   if ( (GPIOG->IDR & DIO0_Pin) ) 
    {
     switch ( RadioRegim ) 
      {
       case rgRadioRx :                 //Rx Radio ready
         LL_GPIO_SetOutputPin(BlueLed_GPIO_Port, BlueLed_Pin);
         RxLen = RadioRxLen();
         RadioReadPacket(RadioBuffer, RxLen);
         RadioBuffer[RxLen++] = RadioRSSI();
         RadioBuffer[RxLen++] = RadioStatus;

         if ( !GPSmobile )
          {SPI1Write(RadioBuffer,RxLen);
           RadioRxOn();
          }
         else
          {
          }
         LL_GPIO_ResetOutputPin(BlueLed_GPIO_Port, BlueLed_Pin);
       break;

       case rgRadioTx :                 //Tx Radio done
         LL_GPIO_ResetOutputPin(RedLed_GPIO_Port, RedLed_Pin);
         RadioStatus = RadioRead(LR_RegIrqFlags);

         RadioLoRaClearIrq();                      //Clear irq
         RadioStandby();                           

         if ( GPSmobile )
           RadioTimeOut  = 50;
         else
           RadioRxOn();

       break;
      } 
    }

   if ( Button1_GPIO_Port->IDR & Button1_Pin )
    {
     LL_GPIO_ResetOutputPin(RedLed_GPIO_Port, RedLed_Pin);
     LL_GPIO_SetOutputPin(BlueLed_GPIO_Port, BlueLed_Pin);
  
/* 	$$$magneto off
     sprintf(TxBuffer,"X=%+8d Y=%+8d Z=%+7d",100,200,300); 
     SPI1Write((ui8*)TxBuffer,32);
     if ( SDTest() )
       LL_GPIO_SetOutputPin(RedLed_GPIO_Port, RedLed_Pin);
     else
       LL_GPIO_SetOutputPin(GreenLed_GPIO_Port, GreenLed_Pin);
*/

     static ui8 color = 0;
     ScreenInit(color);
     FillRect(1,1, 130, 130, color+1);
     FillRect(50,50, 130, 130, color+2);
     FillRect(100,100, 130, 130,color+3);
     Line(200,200,100,100,color+4);
		 Circle(120, 200, 50, color);
     PrintStr("Hello World!", 3, 21, FONT_NORMAL, color+4);
     ScreenTransmit();
     color++;
     color &= 7;


     LL_GPIO_ResetOutputPin(BlueLed_GPIO_Port, BlueLed_Pin);
     while (Button1_GPIO_Port->IDR & Button1_Pin )
      {}
     LL_GPIO_ResetOutputPin(GreenLed_GPIO_Port, GreenLed_Pin);
    }


  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

static void LL_Init(void)
{
  

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* System interrupt init*/
  /* MemoryManagement_IRQn interrupt configuration */
  NVIC_SetPriority(MemoryManagement_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  /* BusFault_IRQn interrupt configuration */
  NVIC_SetPriority(BusFault_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  /* UsageFault_IRQn interrupt configuration */
  NVIC_SetPriority(UsageFault_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  /* SVCall_IRQn interrupt configuration */
  NVIC_SetPriority(SVCall_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  /* DebugMonitor_IRQn interrupt configuration */
  NVIC_SetPriority(DebugMonitor_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  /* PendSV_IRQn interrupt configuration */
  NVIC_SetPriority(PendSV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);

  if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_4)
  {
  Error_Handler();  
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);

  LL_PWR_EnableRange1BoostMode();

  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {
    
  }
  LL_RCC_HSI_SetCalibTrimming(64);

  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_2, 25, LL_RCC_PLLR_DIV_2);

  LL_RCC_PLL_ConfigDomain_48M(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_2, 25, LL_RCC_PLLQ_DIV_8);

  LL_RCC_PLL_EnableDomain_48M();

  LL_RCC_PLL_EnableDomain_SYS();

  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {
    
  }

   /* Intermediate AHB prescaler 2 when target frequency clock is higher than 80 MHz */
   LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_2);
  
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  
  }

  /* Insure 1µs transition state at intermediate medium speed clock based on DWT*/
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  

  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  DWT->CYCCNT = 0;
  while(DWT->CYCCNT < 100);
  
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  LL_Init1msTick(100000000);

  LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);

  LL_SetSystemCoreClock(100000000);

  LL_RCC_SetRNGClockSource(LL_RCC_RNG_CLKSOURCE_PLL);

  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
}

/* USER CODE BEGIN 4 */


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
