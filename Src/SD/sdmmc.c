//----------------------------------------------------------------------------------------------------
#include "stm32l4xx_ll_gpio.h"
#include "sdmmc.h"

static uint32_t SDMMC_GetCmdError(SDMMC_TypeDef *SDMMCx);
static uint32_t SDMMC_GetCmdResp1(SDMMC_TypeDef *SDMMCx, uint8_t SD_CMD, uint32_t Timeout);
static uint32_t SDMMC_GetCmdResp2(SDMMC_TypeDef *SDMMCx);
static uint32_t SDMMC_GetCmdResp3(SDMMC_TypeDef *SDMMCx);
static uint32_t SDMMC_GetCmdResp7(SDMMC_TypeDef *SDMMCx);
static uint32_t SDMMC_GetCmdResp6(SDMMC_TypeDef *SDMMCx, uint8_t SD_CMD, uint16_t *pRCA);


//----------------------------------------------------------------------------------------------------
uint32_t  SDMMC_Init(SDMMC_TypeDef *SDMMCx, SDMMC_InitTypeDef Init)
{
 MODIFY_REG(SDMMCx->CLKCR, CLKCR_CLEAR_MASK, Init.ClockEdge           |\
                                              Init.ClockPowerSave      |\
                                              Init.BusWide             |\
                                              Init.HardwareFlowControl |\
                                              Init.ClockDiv);  
 return SDMMC_ERROR_NONE;
}
//----------------------------------------------------------------------------------------------------
uint32_t SDMMC_ReadFIFO(SDMMC_TypeDef *SDMMCx)
{
  return (SDMMCx->FIFO);
}
//----------------------------------------------------------------------------------------------------
uint32_t SDMMC_WriteFIFO(SDMMC_TypeDef *SDMMCx, uint32_t *pWriteData)
{ 
  /* Write data to FIFO */ 
  SDMMCx->FIFO = *pWriteData;

  return SDMMC_ERROR_NONE;
}


// Set SDMMC Power state to ON. 
uint32_t SDMMC_PowerState_ON(SDMMC_TypeDef *SDMMCx)
{  
  /* Set power state to ON */ 
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  SDMMCx->POWER |= SDMMC_POWER_PWRCTRL;

#else
  SDMMCx->POWER = SDMMC_POWER_PWRCTRL;

#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
  
  return SDMMC_ERROR_NONE; 
}

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
// Set SDMMC Power state to Power-Cycle. 
uint32_t SDMMC_PowerState_Cycle(SDMMC_TypeDef *SDMMCx)
{  
 SDMMCx->POWER |= SDMMC_POWER_PWRCTRL_1;
 return SDMMC_ERROR_NONE; 
}
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

// Set SDMMC Power state to OFF. 
uint32_t SDMMC_PowerState_OFF(SDMMC_TypeDef *SDMMCx)
{
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  SDMMCx->POWER &= ~(SDMMC_POWER_PWRCTRL);

#else
  SDMMCx->POWER = (uint32_t)0x00000000;

#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
 return SDMMC_ERROR_NONE;
}

// Get SDMMC Power state. 
// - 0x00: Power OFF
// - 0x02: Power UP
// - 0x03: Power ON 
uint32_t SDMMC_GetPowerState(SDMMC_TypeDef *SDMMCx)
{
 return (SDMMCx->POWER & SDMMC_POWER_PWRCTRL);
}

// Configure the SDMMC command path according to the specified parameters in
//  *         SDMMC_CmdInitTypeDef structure and send the command 
uint32_t SDMMC_SendCommand(SDMMC_TypeDef *SDMMCx, SDMMC_CmdInitTypeDef *Command)
{
  /* Set the SDMMC Argument value */
  SDMMCx->ARG = Command->Argument;

  /* Set SDMMC command parameters */
  /* Write to SDMMC CMD register */
  MODIFY_REG(SDMMCx->CMD, CMD_CLEAR_MASK, Command->CmdIndex         |\
                                          Command->Response         |\
                                          Command->WaitForInterrupt |\
                                          Command->CPSM); 
  
  return SDMMC_ERROR_NONE;
}

// Return the command index of last command for which response received
//retval - Command index of the last command response received
uint8_t SDMMC_GetCommandResponse(SDMMC_TypeDef *SDMMCx)
{
  return (uint8_t)(SDMMCx->RESPCMD);
}


// Return the response received from the card for the last command
// parameter can be one of the following values:
// SDMMC_RESP1: Response Register 1
// SDMMC_RESP2: Response Register 2
// SDMMC_RESP3: Response Register 3
// SDMMC_RESP4: Response Register 4  
// retval - The Corresponding response register value
uint32_t SDMMC_GetResponse(SDMMC_TypeDef *SDMMCx, uint32_t Response)
{
  __IO uint32_t tmp = 0;

  tmp = (uint32_t)&(SDMMCx->RESP1) + Response;
  return (*(__IO uint32_t *) tmp);
}  

/*
Configure the SDMMC data path according to the specified 
parameters in the SDMMC_DataInitTypeDef.
SDMMCx: Pointer to SDMMC register base  
Data : pointer to a SDMMC_DataInitTypeDef structure 
that contains the configuration information for the SDMMC data.
@retval HAL status
*/
uint32_t SDMMC_ConfigData(SDMMC_TypeDef *SDMMCx, SDMMC_DataInitTypeDef* Data)
{
 // Set the SDMMC Data TimeOut value
 SDMMCx->DTIMER = Data->DataTimeOut;

 // Set the SDMMC DataLength value
 SDMMCx->DLEN = Data->DataLength;

 // Set the SDMMC data configuration parameters
 MODIFY_REG(SDMMCx->DCTRL, DCTRL_CLEAR_MASK, Data->DataBlockSize |\
                                             Data->TransferDir   |\
                                             Data->TransferMode  |\
                                             Data->DPSM);

 return SDMMC_ERROR_NONE;
}

// Returns number of remaining data bytes to be transferred.
uint32_t SDMMC_GetDataCounter(SDMMC_TypeDef *SDMMCx)
{
  return (SDMMCx->DCOUNT);
}


// Get the FIFO data
uint32_t SDMMC_GetFIFOCount(SDMMC_TypeDef *SDMMCx)
{
 return (SDMMCx->FIFO);
}

/*
Sets one of the two options of inserting read wait interval.
SDMMCx: Pointer to SDMMC register base   
SDMMC_ReadWaitMode: SDMMC Read Wait operation mode.
This parameter can be:
SDMMC_READ_WAIT_MODE_CLK: Read Wait control by stopping SDMMCCLK
SDMMC_READ_WAIT_MODE_DATA2: Read Wait control using SDMMC_DATA2
*/
uint32_t SDMMC_SetSDMMCReadWaitMode(SDMMC_TypeDef *SDMMCx, uint32_t SDMMC_ReadWaitMode)
{
 // Set SDMMC read wait mode
 MODIFY_REG(SDMMCx->DCTRL, SDMMC_DCTRL_RWMOD, SDMMC_ReadWaitMode);
 return SDMMC_ERROR_NONE;  
}


// Send the Data Block Lenght command and check the response
uint32_t SDMMC_CmdBlockLength(SDMMC_TypeDef *SDMMCx, uint32_t BlockSize)
{
 SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
 uint32_t errorstate = SDMMC_ERROR_NONE;
  
 // Set Block Size for Card
 sdmmc_cmdinit.Argument         = (uint32_t)BlockSize;
 sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_SET_BLOCKLEN;
 sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
 sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
 sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
 SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
 errorstate = SDMMC_GetCmdResp1(SDMMCx, SDMMC_CMD_SET_BLOCKLEN, SDMMC_CMDTIMEOUT);
 return errorstate;
}

// Send the Read Single Block command and check the response
uint32_t SDMMC_CmdReadSingleBlock(SDMMC_TypeDef *SDMMCx, uint32_t ReadAdd)
{
 SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
 uint32_t errorstate = SDMMC_ERROR_NONE;
  
 // Set Block Size for Card
 sdmmc_cmdinit.Argument         = (uint32_t)ReadAdd;
 sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_READ_SINGLE_BLOCK;
 sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
 sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
 sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
 SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
 // Check for error conditions
 errorstate = SDMMC_GetCmdResp1(SDMMCx, SDMMC_CMD_READ_SINGLE_BLOCK, SDMMC_CMDTIMEOUT);

 return errorstate;
}

// Send the Read Multi Block command and check the response
uint32_t SDMMC_CmdReadMultiBlock(SDMMC_TypeDef *SDMMCx, uint32_t ReadAdd)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  // Set Block Size for Card
  sdmmc_cmdinit.Argument         = (uint32_t)ReadAdd;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_READ_MULT_BLOCK;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  // Check for error conditions
  errorstate = SDMMC_GetCmdResp1(SDMMCx, SDMMC_CMD_READ_MULT_BLOCK, SDMMC_CMDTIMEOUT);

  return errorstate;
}

// Send the Write Single Block command and check the response
uint32_t SDMMC_CmdWriteSingleBlock(SDMMC_TypeDef *SDMMCx, uint32_t WriteAdd)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  /* Set Block Size for Card */ 
  sdmmc_cmdinit.Argument         = (uint32_t)WriteAdd;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_WRITE_SINGLE_BLOCK;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  errorstate = SDMMC_GetCmdResp1(SDMMCx, SDMMC_CMD_WRITE_SINGLE_BLOCK, SDMMC_CMDTIMEOUT);
  return errorstate;
}

// Send the Write Multi Block command and check the response
uint32_t SDMMC_CmdWriteMultiBlock(SDMMC_TypeDef *SDMMCx, uint32_t WriteAdd)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  /* Set Block Size for Card */ 
  sdmmc_cmdinit.Argument         = (uint32_t)WriteAdd;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_WRITE_MULT_BLOCK;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  errorstate = SDMMC_GetCmdResp1(SDMMCx, SDMMC_CMD_WRITE_MULT_BLOCK, SDMMC_CMDTIMEOUT);
  return errorstate;
}

// Send the Start Address Erase command for SD and check the response
uint32_t SDMMC_CmdSDEraseStartAdd(SDMMC_TypeDef *SDMMCx, uint32_t StartAdd)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  /* Set Block Size for Card */ 
  sdmmc_cmdinit.Argument         = (uint32_t)StartAdd;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_SD_ERASE_GRP_START;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  /* Check for error conditions */
  errorstate = SDMMC_GetCmdResp1(SDMMCx, SDMMC_CMD_SD_ERASE_GRP_START, SDMMC_CMDTIMEOUT);

  return errorstate;
}

// Send the End Address Erase command for SD and check the response
uint32_t SDMMC_CmdSDEraseEndAdd(SDMMC_TypeDef *SDMMCx, uint32_t EndAdd)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  /* Set Block Size for Card */ 
  sdmmc_cmdinit.Argument         = (uint32_t)EndAdd;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_SD_ERASE_GRP_END;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  errorstate = SDMMC_GetCmdResp1(SDMMCx, SDMMC_CMD_SD_ERASE_GRP_END, SDMMC_CMDTIMEOUT);
  return errorstate;
}

// Send the Start Address Erase command and check the response
uint32_t SDMMC_CmdEraseStartAdd(SDMMC_TypeDef *SDMMCx, uint32_t StartAdd)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  /* Set Block Size for Card */ 
  sdmmc_cmdinit.Argument         = (uint32_t)StartAdd;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_ERASE_GRP_START;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  errorstate = SDMMC_GetCmdResp1(SDMMCx, SDMMC_CMD_ERASE_GRP_START, SDMMC_CMDTIMEOUT);
  return errorstate;
}


// Send the End Address Erase command and check the response
uint32_t SDMMC_CmdEraseEndAdd(SDMMC_TypeDef *SDMMCx, uint32_t EndAdd)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  /* Set Block Size for Card */ 
  sdmmc_cmdinit.Argument         = (uint32_t)EndAdd;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_ERASE_GRP_END;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  errorstate = SDMMC_GetCmdResp1(SDMMCx, SDMMC_CMD_ERASE_GRP_END, SDMMC_CMDTIMEOUT);
  return errorstate;
}

// Send the Erase command and check the response
uint32_t SDMMC_CmdErase(SDMMC_TypeDef *SDMMCx)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  sdmmc_cmdinit.Argument         = 0;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_ERASE;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  errorstate = SDMMC_GetCmdResp1(SDMMCx, SDMMC_CMD_ERASE, SDMMC_MAXERASETIMEOUT);
  return errorstate;
}

// Send the Stop Transfer command and check the response.
uint32_t SDMMC_CmdStopTransfer(SDMMC_TypeDef *SDMMCx)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  /* Send CMD12 STOP_TRANSMISSION  */
  sdmmc_cmdinit.Argument         = 0;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_STOP_TRANSMISSION;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  errorstate = SDMMC_GetCmdResp1(SDMMCx, SDMMC_CMD_STOP_TRANSMISSION, SDMMC_STOPTRANSFERTIMEOUT);
  return errorstate;
}

// Send the Select Deselect command and check the response.
uint32_t SDMMC_CmdSelDesel(SDMMC_TypeDef *SDMMCx, uint64_t Addr)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  /* Send CMD7 SDMMC_SEL_DESEL_CARD */
  sdmmc_cmdinit.Argument         = (uint32_t)Addr;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_SEL_DESEL_CARD;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  errorstate = SDMMC_GetCmdResp1(SDMMCx, SDMMC_CMD_SEL_DESEL_CARD, SDMMC_CMDTIMEOUT);
  return errorstate;
}

// Send the Go Idle State command and check the response.
uint32_t SDMMC_CmdGoIdleState(SDMMC_TypeDef *SDMMCx)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  sdmmc_cmdinit.Argument         = 0;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_GO_IDLE_STATE;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_NO;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  errorstate = SDMMC_GetCmdError(SDMMCx);
  return errorstate;
}

// Send the Operating Condition command and check the response.
uint32_t SDMMC_CmdOperCond(SDMMC_TypeDef *SDMMCx)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  /* Send CMD8 to verify SD card interface operating condition */
  /* Argument: - [31:12]: Reserved (shall be set to '0')
  - [11:8]: Supply Voltage (VHS) 0x1 (Range: 2.7-3.6 V)
  - [7:0]: Check Pattern (recommended 0xAA) */
  /* CMD Response: R7 */
  sdmmc_cmdinit.Argument         = SDMMC_CHECK_PATTERN;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_HS_SEND_EXT_CSD;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  errorstate = SDMMC_GetCmdResp7(SDMMCx);
  return errorstate;
}

// Send the Application command to verify that that the next command 
uint32_t SDMMC_CmdAppCommand(SDMMC_TypeDef *SDMMCx, uint32_t Argument)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  sdmmc_cmdinit.Argument         = (uint32_t)Argument;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_APP_CMD;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  /* Check for error conditions */
  /* If there is a HAL_ERROR, it is a MMC card, else
  it is a SD card: SD card 2.0 (voltage range mismatch)
     or SD card 1.x */
  errorstate = SDMMC_GetCmdResp1(SDMMCx, SDMMC_CMD_APP_CMD, SDMMC_CMDTIMEOUT);
  return errorstate;
}

// Send the command asking the accessed card to send its operating 
// condition register (OCR)
uint32_t SDMMC_CmdAppOperCommand(SDMMC_TypeDef *SDMMCx, uint32_t Argument)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  sdmmc_cmdinit.Argument         = Argument;
#else
  sdmmc_cmdinit.Argument         = SDMMC_VOLTAGE_WINDOW_SD | Argument;
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_SD_APP_OP_COND;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  /* Check for error conditions */
  errorstate = SDMMC_GetCmdResp3(SDMMCx);

  return errorstate;
}

// Send the Bus Width command and check the response.
uint32_t SDMMC_CmdBusWidth(SDMMC_TypeDef *SDMMCx, uint32_t BusWidth)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  sdmmc_cmdinit.Argument         = (uint32_t)BusWidth;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_APP_SD_SET_BUSWIDTH;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  errorstate = SDMMC_GetCmdResp1(SDMMCx, SDMMC_CMD_APP_SD_SET_BUSWIDTH, SDMMC_CMDTIMEOUT);
  return errorstate;
}

// Send the Send SCR command and check the response.
uint32_t SDMMC_CmdSendSCR(SDMMC_TypeDef *SDMMCx)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  /* Send CMD51 SD_APP_SEND_SCR */
  sdmmc_cmdinit.Argument         = 0;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_SD_APP_SEND_SCR;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  errorstate = SDMMC_GetCmdResp1(SDMMCx, SDMMC_CMD_SD_APP_SEND_SCR, SDMMC_CMDTIMEOUT);
  return errorstate;
}

// Send the Send CID command and check the response.
uint32_t SDMMC_CmdSendCID(SDMMC_TypeDef *SDMMCx)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  /* Send CMD2 ALL_SEND_CID */
  sdmmc_cmdinit.Argument         = 0;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_ALL_SEND_CID;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_LONG;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  errorstate = SDMMC_GetCmdResp2(SDMMCx);
  return errorstate;
}

// Send the Send CSD command and check the response.
uint32_t SDMMC_CmdSendCSD(SDMMC_TypeDef *SDMMCx, uint32_t Argument)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  /* Send CMD9 SEND_CSD */
  sdmmc_cmdinit.Argument         = Argument;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_SEND_CSD;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_LONG;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  errorstate = SDMMC_GetCmdResp2(SDMMCx);
  return errorstate;
}

// Send the Send CSD command and check the response.
uint32_t SDMMC_CmdSetRelAdd(SDMMC_TypeDef *SDMMCx, uint16_t *pRCA)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  /* Send CMD3 SD_CMD_SET_REL_ADDR */
  sdmmc_cmdinit.Argument         = 0;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_SET_REL_ADDR;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  errorstate = SDMMC_GetCmdResp6(SDMMCx, SDMMC_CMD_SET_REL_ADDR, pRCA);
  return errorstate;
}

// Send the Status command and check the response.
uint32_t SDMMC_CmdSendStatus(SDMMC_TypeDef *SDMMCx, uint32_t Argument)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  sdmmc_cmdinit.Argument         = Argument;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_SEND_STATUS;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  errorstate = SDMMC_GetCmdResp1(SDMMCx, SDMMC_CMD_SEND_STATUS, SDMMC_CMDTIMEOUT);
  return errorstate;
}

// Send the Status register command and check the response.
uint32_t SDMMC_CmdStatusRegister(SDMMC_TypeDef *SDMMCx)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  sdmmc_cmdinit.Argument         = 0;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_SD_APP_STATUS;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  errorstate = SDMMC_GetCmdResp1(SDMMCx, SDMMC_CMD_SD_APP_STATUS, SDMMC_CMDTIMEOUT);
  return errorstate;
}

// Sends host capacity support information and activates the card's 
// initialization process. Send SDMMC_CMD_SEND_OP_COND command
uint32_t SDMMC_CmdOpCondition(SDMMC_TypeDef *SDMMCx, uint32_t Argument)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  sdmmc_cmdinit.Argument         = Argument;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_SEND_OP_COND;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  errorstate = SDMMC_GetCmdResp3(SDMMCx);
  return errorstate;
}

// Checks switchable function and switch card function. SDMMC_CMD_HS_SWITCH comand
uint32_t SDMMC_CmdSwitch(SDMMC_TypeDef *SDMMCx, uint32_t Argument)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  sdmmc_cmdinit.Argument         = Argument;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_HS_SWITCH;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  errorstate = SDMMC_GetCmdResp1(SDMMCx, SDMMC_CMD_HS_SWITCH, SDMMC_CMDTIMEOUT);
  return errorstate;
}

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)

// Send the command asking the accessed card to send its operating 
// condition register (OCR)
uint32_t SDMMC_CmdVoltageSwitch(SDMMC_TypeDef *SDMMCx)
{
  SDMMC_CmdInitTypeDef  sdmmc_cmdinit;
  uint32_t errorstate = SDMMC_ERROR_NONE;
  
  sdmmc_cmdinit.Argument         = 0x00000000;
  sdmmc_cmdinit.CmdIndex         = SDMMC_CMD_VOLTAGE_SWITCH;
  sdmmc_cmdinit.Response         = SDMMC_RESPONSE_SHORT;
  sdmmc_cmdinit.WaitForInterrupt = SDMMC_WAIT_NO;
  sdmmc_cmdinit.CPSM             = SDMMC_CPSM_ENABLE;
  SDMMC_SendCommand(SDMMCx, &sdmmc_cmdinit);
  
  errorstate = SDMMC_GetCmdResp1(SDMMCx, SDMMC_CMD_VOLTAGE_SWITCH, SDMMC_CMDTIMEOUT);
  return errorstate;
}
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */


// Checks for error conditions for CMD0.
static uint32_t SDMMC_GetCmdError(SDMMC_TypeDef *SDMMCx)
{
  /* 8 is the number of required instructions cycles for the below loop statement.
  The SDMMC_CMDTIMEOUT is expressed in ms */
  register uint32_t count = SDMMC_CMDTIMEOUT * (SystemCoreClock / 8 /1000);
  
  do
  {
    if (count-- == 0)
    {
      return SDMMC_ERROR_TIMEOUT;
    }
    
  }while(!__SDMMC_GET_FLAG(SDMMCx, SDMMC_FLAG_CMDSENT));
  
  /* Clear all the static flags */
  __SDMMC_CLEAR_FLAG(SDMMCx, SDMMC_STATIC_CMD_FLAGS);
  
  return SDMMC_ERROR_NONE;
}

// Checks for error conditions for R1 response.
static uint32_t SDMMC_GetCmdResp1(SDMMC_TypeDef *SDMMCx, uint8_t SD_CMD, uint32_t Timeout)
{
  uint32_t response_r1;
  uint32_t flags;

  flags = SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CMDREND | SDMMC_FLAG_CTIMEOUT;

  /* 8 is the number of required instructions cycles for the below loop statement.
  The Timeout is expressed in ms */
  register uint32_t count = Timeout * (SystemCoreClock / 8 /1000);
  
  do
  {
    if (count-- == 0)
    {
      return SDMMC_ERROR_TIMEOUT;
    }
    
  }while(!__SDMMC_GET_FLAG(SDMMCx, flags));
  
  if(__SDMMC_GET_FLAG(SDMMCx, SDMMC_FLAG_CTIMEOUT))
  {
    __SDMMC_CLEAR_FLAG(SDMMCx, SDMMC_FLAG_CTIMEOUT);
    
    return SDMMC_ERROR_CMD_RSP_TIMEOUT;
  }
  else if(__SDMMC_GET_FLAG(SDMMCx, SDMMC_FLAG_CCRCFAIL))
  {
    __SDMMC_CLEAR_FLAG(SDMMCx, SDMMC_FLAG_CCRCFAIL);
    
    return SDMMC_ERROR_CMD_CRC_FAIL;
  }
  
  /* Check response received is of desired command */
  if(SDMMC_GetCommandResponse(SDMMCx) != SD_CMD)
  {
    return SDMMC_ERROR_CMD_CRC_FAIL;
  }
  
  /* Clear all the static flags */
  __SDMMC_CLEAR_FLAG(SDMMCx, SDMMC_STATIC_CMD_FLAGS);
  
  /* We have received response, retrieve it for analysis  */
  response_r1 = SDMMC_GetResponse(SDMMCx, SDMMC_RESP1);
  
  if((response_r1 & SDMMC_OCR_ERRORBITS) == SDMMC_ALLZERO)
  {
    return SDMMC_ERROR_NONE;
  }
  else if((response_r1 & SDMMC_OCR_ADDR_OUT_OF_RANGE) == SDMMC_OCR_ADDR_OUT_OF_RANGE)
  {
    return SDMMC_ERROR_ADDR_OUT_OF_RANGE;
  }
  else if((response_r1 & SDMMC_OCR_ADDR_MISALIGNED) == SDMMC_OCR_ADDR_MISALIGNED)
  {
    return SDMMC_ERROR_ADDR_MISALIGNED;
  }
  else if((response_r1 & SDMMC_OCR_BLOCK_LEN_ERR) == SDMMC_OCR_BLOCK_LEN_ERR)
  {
    return SDMMC_ERROR_BLOCK_LEN_ERR;
  }
  else if((response_r1 & SDMMC_OCR_ERASE_SEQ_ERR) == SDMMC_OCR_ERASE_SEQ_ERR)
  {
    return SDMMC_ERROR_ERASE_SEQ_ERR;
  }
  else if((response_r1 & SDMMC_OCR_BAD_ERASE_PARAM) == SDMMC_OCR_BAD_ERASE_PARAM)
  {
    return SDMMC_ERROR_BAD_ERASE_PARAM;
  }
  else if((response_r1 & SDMMC_OCR_WRITE_PROT_VIOLATION) == SDMMC_OCR_WRITE_PROT_VIOLATION)
  {
    return SDMMC_ERROR_WRITE_PROT_VIOLATION;
  }
  else if((response_r1 & SDMMC_OCR_LOCK_UNLOCK_FAILED) == SDMMC_OCR_LOCK_UNLOCK_FAILED)
  {
    return SDMMC_ERROR_LOCK_UNLOCK_FAILED;
  }
  else if((response_r1 & SDMMC_OCR_COM_CRC_FAILED) == SDMMC_OCR_COM_CRC_FAILED)
  {
    return SDMMC_ERROR_COM_CRC_FAILED;
  }
  else if((response_r1 & SDMMC_OCR_ILLEGAL_CMD) == SDMMC_OCR_ILLEGAL_CMD)
  {
    return SDMMC_ERROR_ILLEGAL_CMD;
  }
  else if((response_r1 & SDMMC_OCR_CARD_ECC_FAILED) == SDMMC_OCR_CARD_ECC_FAILED)
  {
    return SDMMC_ERROR_CARD_ECC_FAILED;
  }
  else if((response_r1 & SDMMC_OCR_CC_ERROR) == SDMMC_OCR_CC_ERROR)
  {
    return SDMMC_ERROR_CC_ERR;
  }
  else if((response_r1 & SDMMC_OCR_STREAM_READ_UNDERRUN) == SDMMC_OCR_STREAM_READ_UNDERRUN)
  {
    return SDMMC_ERROR_STREAM_READ_UNDERRUN;
  }
  else if((response_r1 & SDMMC_OCR_STREAM_WRITE_OVERRUN) == SDMMC_OCR_STREAM_WRITE_OVERRUN)
  {
    return SDMMC_ERROR_STREAM_WRITE_OVERRUN;
  }
  else if((response_r1 & SDMMC_OCR_CID_CSD_OVERWRITE) == SDMMC_OCR_CID_CSD_OVERWRITE)
  {
    return SDMMC_ERROR_CID_CSD_OVERWRITE;
  }
  else if((response_r1 & SDMMC_OCR_WP_ERASE_SKIP) == SDMMC_OCR_WP_ERASE_SKIP)
  {
    return SDMMC_ERROR_WP_ERASE_SKIP;
  }
  else if((response_r1 & SDMMC_OCR_CARD_ECC_DISABLED) == SDMMC_OCR_CARD_ECC_DISABLED)
  {
    return SDMMC_ERROR_CARD_ECC_DISABLED;
  }
  else if((response_r1 & SDMMC_OCR_ERASE_RESET) == SDMMC_OCR_ERASE_RESET)
  {
    return SDMMC_ERROR_ERASE_RESET;
  }
  else if((response_r1 & SDMMC_OCR_AKE_SEQ_ERROR) == SDMMC_OCR_AKE_SEQ_ERROR)
  {
    return SDMMC_ERROR_AKE_SEQ_ERR;
  }
  else
  {
    return SDMMC_ERROR_GENERAL_UNKNOWN_ERR;
  }
}

// Checks for error conditions for R2 (CID or CSD) response.
static uint32_t SDMMC_GetCmdResp2(SDMMC_TypeDef *SDMMCx)
{
  /* 8 is the number of required instructions cycles for the below loop statement.
  The SDMMC_CMDTIMEOUT is expressed in ms */
  register uint32_t count = SDMMC_CMDTIMEOUT * (SystemCoreClock / 8 /1000);
  
  do
  {
    if (count-- == 0)
    {
      return SDMMC_ERROR_TIMEOUT;
    }
    
  }while(!__SDMMC_GET_FLAG(SDMMCx, SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CMDREND | SDMMC_FLAG_CTIMEOUT));
    
  if (__SDMMC_GET_FLAG(SDMMCx, SDMMC_FLAG_CTIMEOUT))
  {
    __SDMMC_CLEAR_FLAG(SDMMCx, SDMMC_FLAG_CTIMEOUT);
    
    return SDMMC_ERROR_CMD_RSP_TIMEOUT;
  }
  else if (__SDMMC_GET_FLAG(SDMMCx, SDMMC_FLAG_CCRCFAIL))
  {
    __SDMMC_CLEAR_FLAG(SDMMCx, SDMMC_FLAG_CCRCFAIL);
    
    return SDMMC_ERROR_CMD_CRC_FAIL;
  }
  else
  {
    /* No error flag set */
    /* Clear all the static flags */
    __SDMMC_CLEAR_FLAG(SDMMCx, SDMMC_STATIC_CMD_FLAGS);
  }

  return SDMMC_ERROR_NONE;
}

// Checks for error conditions for R3 (OCR) response.
static uint32_t SDMMC_GetCmdResp3(SDMMC_TypeDef *SDMMCx)
{
  /* 8 is the number of required instructions cycles for the below loop statement.
  The SDMMC_CMDTIMEOUT is expressed in ms */
  register uint32_t count = SDMMC_CMDTIMEOUT * (SystemCoreClock / 8 /1000);
  
  do
  {
    if (count-- == 0)
    {
      return SDMMC_ERROR_TIMEOUT;
    }
    
  }while(!__SDMMC_GET_FLAG(SDMMCx, SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CMDREND | SDMMC_FLAG_CTIMEOUT));
  
  if(__SDMMC_GET_FLAG(SDMMCx, SDMMC_FLAG_CTIMEOUT))
  {
    __SDMMC_CLEAR_FLAG(SDMMCx, SDMMC_FLAG_CTIMEOUT);
    
    return SDMMC_ERROR_CMD_RSP_TIMEOUT;
  }
  else
  {  
    /* Clear all the static flags */
    __SDMMC_CLEAR_FLAG(SDMMCx, SDMMC_STATIC_CMD_FLAGS);
  }
  
  return SDMMC_ERROR_NONE;
}

// Checks for error conditions for R6 (RCA) response.
static uint32_t SDMMC_GetCmdResp6(SDMMC_TypeDef *SDMMCx, uint8_t SD_CMD, uint16_t *pRCA)
{
  uint32_t response_r1;
  
  /* 8 is the number of required instructions cycles for the below loop statement.
  The SDMMC_CMDTIMEOUT is expressed in ms */
  register uint32_t count = SDMMC_CMDTIMEOUT * (SystemCoreClock / 8 /1000);
  
  do
  {
    if (count-- == 0)
    {
      return SDMMC_ERROR_TIMEOUT;
    }
    
  }while(!__SDMMC_GET_FLAG(SDMMCx, SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CMDREND | SDMMC_FLAG_CTIMEOUT));
  
  if(__SDMMC_GET_FLAG(SDMMCx, SDMMC_FLAG_CTIMEOUT))
  {
    __SDMMC_CLEAR_FLAG(SDMMCx, SDMMC_FLAG_CTIMEOUT);
    
    return SDMMC_ERROR_CMD_RSP_TIMEOUT;
  }
  else if(__SDMMC_GET_FLAG(SDMMCx, SDMMC_FLAG_CCRCFAIL))
  {
    __SDMMC_CLEAR_FLAG(SDMMCx, SDMMC_FLAG_CCRCFAIL);
    
    return SDMMC_ERROR_CMD_CRC_FAIL;
  }
  
  /* Check response received is of desired command */
  if(SDMMC_GetCommandResponse(SDMMCx) != SD_CMD)
  {
    return SDMMC_ERROR_CMD_CRC_FAIL;
  }
  
  /* Clear all the static flags */
  __SDMMC_CLEAR_FLAG(SDMMCx, SDMMC_STATIC_CMD_FLAGS);
  
  /* We have received response, retrieve it.  */
  response_r1 = SDMMC_GetResponse(SDMMCx, SDMMC_RESP1);
  
  if((response_r1 & (SDMMC_R6_GENERAL_UNKNOWN_ERROR | SDMMC_R6_ILLEGAL_CMD | SDMMC_R6_COM_CRC_FAILED)) == SDMMC_ALLZERO)
  {
    *pRCA = (uint16_t) (response_r1 >> 16);
    
    return SDMMC_ERROR_NONE;
  }
  else if((response_r1 & SDMMC_R6_ILLEGAL_CMD) == SDMMC_R6_ILLEGAL_CMD)
  {
    return SDMMC_ERROR_ILLEGAL_CMD;
  }
  else if((response_r1 & SDMMC_R6_COM_CRC_FAILED) == SDMMC_R6_COM_CRC_FAILED)
  {
    return SDMMC_ERROR_COM_CRC_FAILED;
  }
  else
  {
    return SDMMC_ERROR_GENERAL_UNKNOWN_ERR;
  }
}

// Checks for error conditions for R7 response.
static uint32_t SDMMC_GetCmdResp7(SDMMC_TypeDef *SDMMCx)
{
  /* 8 is the number of required instructions cycles for the below loop statement.
  The SDMMC_CMDTIMEOUT is expressed in ms */
  register uint32_t count = SDMMC_CMDTIMEOUT * (SystemCoreClock / 8 /1000);
  
  do
  {
    if (count-- == 0)
    {
      return SDMMC_ERROR_TIMEOUT;
    }
    
  }while(!__SDMMC_GET_FLAG(SDMMCx, SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CMDREND | SDMMC_FLAG_CTIMEOUT));

  if(__SDMMC_GET_FLAG(SDMMCx, SDMMC_FLAG_CTIMEOUT))
  {
    /* Card is SD V2.0 compliant */
    __SDMMC_CLEAR_FLAG(SDMMCx, SDMMC_FLAG_CTIMEOUT);
    
    return SDMMC_ERROR_CMD_RSP_TIMEOUT;
  }
  
  else if(__SDMMC_GET_FLAG(SDMMCx, SDMMC_FLAG_CCRCFAIL))
  {
    /* Card is SD V2.0 compliant */
    __SDMMC_CLEAR_FLAG(SDMMCx, SDMMC_FLAG_CCRCFAIL);
    
    return SDMMC_ERROR_CMD_CRC_FAIL;
  }

  if(__SDMMC_GET_FLAG(SDMMCx, SDMMC_FLAG_CMDREND))
  {
    /* Card is SD V2.0 compliant */
    __SDMMC_CLEAR_FLAG(SDMMCx, SDMMC_FLAG_CMDREND);
  }
  
  return SDMMC_ERROR_NONE;
  
}

