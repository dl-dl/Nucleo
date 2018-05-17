#include "lsm303c.h"
#include "SPI.h"
#include "GPIO.h"



void AccWrite(uint8_t RegisterAddr, uint8_t Value)
{
 LL_GPIO_ResetOutputPin(XL_CS_GPIO_Port, XL_CS_Pin);
// HAL_GPIO_WritePin(XL_CS_GPIO_Port, XL_CS_Pin, GPIO_PIN_RESET);
 
 SPI2Write(&RegisterAddr, 1);
 SPI2Write(&Value, 1);

 LL_GPIO_SetOutputPin(XL_CS_GPIO_Port, XL_CS_Pin);
// HAL_GPIO_WritePin(XL_CS_GPIO_Port, XL_CS_Pin, GPIO_PIN_SET);
}


uint8_t AccRead(uint8_t RegisterAddr)
{
 uint8_t readData;
 RegisterAddr |= 0x80;
 
 LL_GPIO_ResetOutputPin(XL_CS_GPIO_Port, XL_CS_Pin);
// HAL_GPIO_WritePin(XL_CS_GPIO_Port, MAG_CS_Pin, GPIO_PIN_RESET);

 SPI2Write(&RegisterAddr, 1);

 SPI2Read(&readData, 1);

 LL_GPIO_SetOutputPin(XL_CS_GPIO_Port, XL_CS_Pin);
// HAL_GPIO_WritePin(XL_CS_GPIO_Port, MAG_CS_Pin, GPIO_PIN_SET);
 return readData;
}


uint8_t AccInit()
{  
 uint8_t ctrl = 0x00;
  
 // Write value to ACC MEMS CTRL_REG1 register 
 //ctrl = (uint8_t) InitStruct;
 //AccWrite(LSM303C_CTRL_REG1_A, ctrl);
  
 // Write value to ACC MEMS CTRL_REG4 register
 //ctrl = ((uint8_t) (InitStruct >> 8));
 //AccWrite(LSM303C_CTRL_REG4_A, ctrl);
  
 // Enabled SPI/I2C read communication 
 AccWrite(LSM303C_CTRL_REG4_A, 0x5);

 // Read value at Who am I register address 
 ctrl = AccRead(LSM303C_WHO_AM_I_ADDR);
 return ctrl;  
}


// Put Accelerometer in power down mode or not.
// Mode equal to LSM303C_ACC_ODR_OFF means enable Low Power Mode, otherwise Output data rate is set.
void AccLowPower(uint16_t Mode)
{  
 uint8_t ctrl = 0x00;
  
 // Read control register 1 value 
 ctrl = AccRead(LSM303C_CTRL_REG1_A);

 // Clear ODR bits 
 ctrl &= ~(LSM303C_ACC_ODR_BITPOSITION);

 // Set Power down
 ctrl |= (uint8_t)Mode;
  
 // write back control register 
 AccWrite(LSM303C_CTRL_REG1_A, ctrl);
}

// Set High Pass Filter Modality
void AccFilterConfig(uint8_t FilterStruct) 
{
//  uint8_t tmpreg;
  
//  /* Read CTRL_REG2 register */
//  tmpreg = AccRead(LSM303C_CTRL_REG2_A);
//  
//  tmpreg &= 0x0C;
//  tmpreg = FilterStruct;
  
  /* Write value to ACC MEMS CTRL_REG2 register */
//  AccWrite(LSM303C_CTRL_REG2_A, tmpreg);
}

//
void AccReadXYZ(int16_t* pData)
{
 int16_t pnRawData[3];
 uint8_t ctrlx[2]={0,0};
 uint8_t buffer[6];
 uint8_t i = 0;
 uint8_t sensitivity = LSM303C_ACC_SENSITIVITY_2G;
  
 // Read the acceleration control register content
 ctrlx[0] = AccRead(LSM303C_CTRL_REG4_A);
 ctrlx[1] = AccRead(LSM303C_CTRL_REG5_A);
  
 // Read output register X, Y & Z acceleration
 buffer[0] = AccRead(LSM303C_OUT_X_L_A); 
 buffer[1] = AccRead(LSM303C_OUT_X_H_A);
 buffer[2] = AccRead(LSM303C_OUT_Y_L_A);
 buffer[3] = AccRead(LSM303C_OUT_Y_H_A);
 buffer[4] = AccRead(LSM303C_OUT_Z_L_A);
 buffer[5] = AccRead(LSM303C_OUT_Z_H_A);
  
 for( i=0; i<3; i++ )
  {
   pnRawData[i]=((int16_t)((uint16_t)buffer[2*i+1] << 8) + buffer[2*i]);
  }
  
 // Normal mode
 // Switch the sensitivity value set in the CRTL4
 switch(ctrlx[0] & LSM303C_ACC_FULLSCALE_8G)
  {
   case LSM303C_ACC_FULLSCALE_2G:
     sensitivity = LSM303C_ACC_SENSITIVITY_2G;
   break;
   case LSM303C_ACC_FULLSCALE_4G:
     sensitivity = LSM303C_ACC_SENSITIVITY_4G;
   break;
   case LSM303C_ACC_FULLSCALE_8G:
     sensitivity = LSM303C_ACC_SENSITIVITY_8G;
   break;
  }
  
 // Obtain the mg value for the three axis
 for ( i=0; i<3; i++ )
  {
   pData[i]=(pnRawData[i] * sensitivity);
  }
}

//***********************************************************************************************
//   Magnetometer driver 
//***********************************************************************************************

void MagWrite(uint8_t RegisterAddr, uint8_t Value)
{
 uint8_t buffer[2];
 buffer[0] = RegisterAddr;
 buffer[1] = Value;
 
 MAG_CS_GPIO_Port->BRR = MAG_CS_Pin;
 
 SPI2Write(buffer,2);

 MAG_CS_GPIO_Port->BSRR = MAG_CS_Pin;
}

//---------------------------------------------------------------------------------------------------
uint8_t MagRead(uint8_t regAddr)
{
 uint8_t regData;

 MAG_CS_GPIO_Port->BRR = MAG_CS_Pin;

 regAddr |= 0x80;
 SPI2Write(&regAddr,1);
 SPI2Read(&regData,1);

 MAG_CS_GPIO_Port->BSRR = MAG_CS_Pin;
 return(regData);
}

uint8_t MagInit(void)
{
 SET_BIT(SPI2->CR2, SPI_CR2_FRXTH);                   // 1/4 Fifo = 8-bit
 SET_BIT(SPI2->CR1, SPI_CR1_BIDIOE);                  // Tx direction, clock off
//  SET_BIT(SPI2->CR1, SPI_CR1_SPE);                     // SPI On

 MagWrite(LSM303C_CTRL_REG1_M, LSM303C_MAG_TEMPSENSOR_DISABLE | LSM303C_MAG_OM_XY_ULTRAHIGH | LSM303C_MAG_ODR_40_HZ);
 MagWrite(LSM303C_CTRL_REG2_M, LSM303C_MAG_FS_16_GA | LSM303C_MAG_REBOOT_DEFAULT | LSM303C_MAG_SOFT_RESET_DEFAULT);
 MagWrite(LSM303C_CTRL_REG3_M, LSM303C_MAG_SPI_MODE | LSM303C_MAG_CONFIG_NORMAL_MODE | LSM303C_MAG_CONTINUOUS_MODE);
 MagWrite(LSM303C_CTRL_REG4_M, LSM303C_MAG_OM_Z_ULTRAHIGH | LSM303C_MAG_BLE_LSB);
 MagWrite(LSM303C_CTRL_REG5_M, LSM303C_MAG_BDU_CONTINUOUS);
 return(MagRead(LSM303C_WHO_AM_I_ADDR));
}


void MagLowPower(uint16_t Mode)
{  
 uint8_t ctrl = 0x00;
  
 /* Read control register 1 value */
 ctrl = MagRead(LSM303C_CTRL_REG3_M);

 /* Clear ODR bits */
 ctrl &= ~(LSM303C_MAG_SELECTION_MODE);

 /* Set mode */
 ctrl |= (uint8_t)Mode;
  
 /* write back control register */
 MagWrite(LSM303C_CTRL_REG3_M, ctrl);
}


uint8_t MagGetDataStatus(void)
{
 /* Read Mag STATUS register */
 return MagRead(LSM303C_STATUS_REG_M);
}


void MagReadXYZ(int16_t *xyzData)
{
 uint8_t i;
 uint8_t ctrlx;
 uint8_t buffer[6];

  // Read the magnetometer control register content
 ctrlx = MagRead(LSM303C_CTRL_REG4_M);

 uint8_t  RegisterAddr = LSM303C_OUT_X_L_M | 0xC0;          //Read and increment addres

 MAG_CS_GPIO_Port->BRR = MAG_CS_Pin;

 SPI2Write(&RegisterAddr, 1);
 SPI2Read(buffer, 6);

 MAG_CS_GPIO_Port->BSRR = MAG_CS_Pin;
 
 // Check in the control register4 the data alignment
 if((ctrlx & LSM303C_MAG_BLE_MSB)) 
  {
   for(i = 0; i<3; i++)
    {
     xyzData[i]=((int16_t)((uint16_t)buffer[2*i] << 8) + buffer[2*i+1]);
    }
  }
 else
  {
   for(i=0; i<3; i++)
    {
     xyzData[i]=((int16_t)((uint16_t)buffer[2*i+1] << 8) + buffer[2*i]);
    }
  }
}


void LSM303C_MagReadXYZ(int16_t* pData)
{
  uint8_t ctrlx;
  uint8_t buffer[6];
  uint8_t i=0;

  
  /* Read the magnetometer control register content */
  ctrlx = MagRead(LSM303C_CTRL_REG4_M);

  /* Read output register X, Y & Z magnetometer */
  buffer[0] = MagRead(LSM303C_OUT_X_L_M); 
  buffer[1] = MagRead(LSM303C_OUT_X_H_M);
  buffer[2] = MagRead(LSM303C_OUT_Y_L_M);
  buffer[3] = MagRead(LSM303C_OUT_Y_H_M);
  buffer[4] = MagRead(LSM303C_OUT_Z_L_M);
  buffer[5] = MagRead(LSM303C_OUT_Z_H_M);
  
 /* Check in the control register4 the data alignment*/
 if((ctrlx & LSM303C_MAG_BLE_MSB)) 
  {
   for(i = 0; i<3; i++)
    {
     pData[i]=((int16_t)((uint16_t)buffer[2*i] << 8) + buffer[2*i+1]);
    }
  }
 else
  {
   for(i=0; i<3; i++)
    {
     pData[i]=((int16_t)((uint16_t)buffer[2*i+1] << 8) + buffer[2*i]);
    }
  }
}

