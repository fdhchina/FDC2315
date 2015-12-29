/**
  ******************************************************************************
  * @file I2C/M24C08_EEPROM/i2c_ee.c 
  * @author  MCD Application Team
  * @version  V3.0.0
  * @date  04/06/2009
  * @brief  This file provides a set of functions needed to manage the
  *         communication between I2C peripheral and I2C M24C08 EEPROM.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/
#include "hal.h"
#include "i2c_ee.h"


#define Set_SDA_High  GPIO_SetBits(I2C_GPIOX, I2C_SDA_GPIOPIN) 
#define Set_SDA_Low   GPIO_ResetBits(I2C_GPIOX, I2C_SDA_GPIOPIN) 
#define Set_SCL_High  GPIO_SetBits(I2C_GPIOX, I2C_SCL_GPIOPIN) 
#define Set_SCL_Low   GPIO_ResetBits(I2C_GPIOX, I2C_SCL_GPIOPIN) 




#define SDA_High     GPIO_ReadInputDataBit(I2C_GPIOX, I2C_SDA_GPIOPIN) !=0
#define SDA_Low     GPIO_ReadInputDataBit(I2C_GPIOX, I2C_SDA_GPIOPIN)==0 



void I2C_Delay(void);

void SDA_set_out(void);
void SDA_set_in(void);

void StopCondition(void);
unsigned char Send_Byte(unsigned char cData);
unsigned char Read_Byte(unsigned char cNum);
extern __IO uint32_t bDeviceState; /* USB device status */

void I2C_Delay(void)
{
	unsigned char	i;
	for(i=0;i<20;i++)
     		 __NOP();

}



void SDA_set_in(void)
{
	// GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = I2C_SDA_GPIOPIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//GPIO_Mode_IN_FLOATING;//GPIO_Mode_IPU;
	GPIO_Init(I2C_GPIOX, &GPIO_InitStructure);
}

void SDA_set_out(void)
{
	// GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = I2C_SDA_GPIOPIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(I2C_GPIOX, &GPIO_InitStructure);
}


void I2C_InitGPIO(void)
{
	//  GPIO_InitTypeDef  GPIO_InitStructure; 

	/* Configure I2C1 pins: SCL and SDA */
	GPIO_InitStructure.GPIO_Pin =  I2C_SDA_GPIOPIN | I2C_SCL_GPIOPIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(I2C_GPIOX, &GPIO_InitStructure);

	Set_SCL_High;
	I2C_Delay();
	Set_SDA_High;	
	I2C_Delay();
}
//----------------------------------------
// S_Condition for TWD protocol
//----------------------------------------

void StartCondition(void)
{    
        Set_SDA_High;           
        I2C_Delay();
        Set_SCL_High;        /* make sure two line is release */     
        I2C_Delay();        
        Set_SDA_Low;          
        I2C_Delay();
        Set_SCL_Low;       
        I2C_Delay();
}

//----------------------------------------
// P_Condition for TWD protocol
//----------------------------------------
void StopCondition(void)
{
	Set_SDA_Low;    
	I2C_Delay();
    
	Set_SCL_High;    
	I2C_Delay();
    
	Set_SDA_High;
	I2C_Delay();
	I2C_Delay();
}

//--------------------------------------------------
// Send_Byte
//    Send a byte to master with a acknowledge bit
//--------------------------------------------------
unsigned char Send_Byte(unsigned char cData)
{
	unsigned char ix,cAcknowledge;   

	cAcknowledge = 0;
	//printf("cData=%x\r\n",cData);
	for(ix = 0; ix < 8; ix++)
	{ 
		if(cData&0x80)
			Set_SDA_High;
		else
			Set_SDA_Low;
		cData<<=1;    
		I2C_Delay();    
		Set_SCL_High;
		I2C_Delay();
		Set_SCL_Low;
		I2C_Delay();
	}
	I2C_Delay();
	Set_SDA_High;                /* release data line for acknowledge */ 
	SDA_set_in();
	I2C_Delay();
	Set_SCL_High;                /* Send a clock for Acknowledge */
	I2C_Delay();
	if(SDA_High) 
	{
		cAcknowledge = 1; /* No Acknowledge */
	}
	I2C_Delay();    
	Set_SCL_Low;                   /* Finish Acknoledge */
	SDA_set_out(); 
	I2C_Delay(); 
	return(cAcknowledge);
}

//--------------------------------------------------
// Read_Byte
//    Read a byte from master with a acknowledge bit
//--------------------------------------------------
unsigned char Read_Byte(unsigned char cNum)
{
	unsigned char ix;
	unsigned char cRetval=0;
    
	SDA_set_in();
	I2C_Delay();
	for(ix=0;ix<8;ix++)
	{
		Set_SCL_High;         
		//I2C_Delay();
		cRetval = (SDA_High)? (cRetval|(1<<(7-ix))):cRetval ;  /* MSB First */
		I2C_Delay();
		Set_SCL_Low;
		I2C_Delay();
	}
	SDA_set_out(); 

	if(cNum==1)
		Set_SDA_High;
	else
		Set_SDA_Low;
	I2C_Delay();

	Set_SCL_High;
	I2C_Delay();
	Set_SCL_Low;
	I2C_Delay();
	// Set_SDA_High;
	// I2C_Delay();
	return cRetval;
}

//--------------------------------------------------
// Read Byte to Device
//--------------------------------------------------
unsigned char I2CReadByte(unsigned char cDevAddr, unsigned char cReg)
{
	unsigned char cResult = 0;

	StartCondition();
	if(Send_Byte(cDevAddr))
		return 0;  // Write address

	if(Send_Byte(cReg))
		return 0;

	/* read data */
	StartCondition();
	if (Send_Byte(cDevAddr | 0x01)) // Read address
		return 0;
	cResult = Read_Byte(1);

	StopCondition();
	return cResult;
}


unsigned char I2CWriteByte(unsigned char cDevAddr, unsigned char cReg, unsigned char cData)
{
	I2C_Delay();
  
	/* start condition */
	StartCondition();
	cDevAddr &= 0xFE;   // Force WRITE address
	/* send device ID and write data */
	if(!Send_Byte(cDevAddr))
	{
		if(!Send_Byte(cReg))
		{
			if(!Send_Byte(cData))
			{
				StopCondition();
				return (1);      /* success */
			}
		}
	}    
	StopCondition();
   
	return (0);
}

unsigned char I2CWriteNBytes(unsigned char cDevAddr, unsigned short cReg, unsigned char cRegmode, unsigned char *cData1,unsigned char nums)
{
	unsigned char i;
 
	I2C_Delay();
  
	/* start condition */
	StartCondition();
	cDevAddr &= 0xFE;   // Force WRITE address
	/* send device ID and write data */
	if(!Send_Byte(cDevAddr))
	{
		if(cRegmode)
		{
			if(cRegmode>1)
				if(Send_Byte(cReg>>8))
					return 0;
			if(Send_Byte(cReg&0xFF))
				return 0;
		}
		for(i=0;i<nums-1;i++)
		{
			if(Send_Byte(*cData1++))
			{ 
				StopCondition();
				return(0); 
			 }
   	   	}
		if(!Send_Byte(*cData1))
		{
			StopCondition();
			return(1);      /* success */
		}
	}    
	StopCondition();
  	
	return (0);
}
//--------------------------------------------------
// Read Multiple Bytes to Device
//--------------------------------------------------
unsigned char I2CReadNBytes(unsigned char cDevAddr, unsigned short cReg, unsigned char cRegmode, unsigned char *pString, unsigned char cNum)
{
	/* write reg offset */
	StartCondition();
	if(Send_Byte(cDevAddr))
		return 0;  // Write address
	if(cRegmode)
	{
		if(cRegmode>1)
	  		if(Send_Byte(cReg>>8))
				return 0;
	  	if(Send_Byte(cReg&0xFF))
			return 0;
	}
	/* read data */
	StartCondition();
	if (Send_Byte(cDevAddr | 0x01)) // Read address
		return 0;
	while(cNum)
	{
  		*pString++ = Read_Byte(cNum);
		cNum--;
	}

 	StopCondition();
	 return 1;
}

#if 0
#define AX8703_ADDRESS 0xA8
unsigned char AX8703_I2C_ByteWrite(unsigned char voldata, unsigned char flagmute)
{
 // I2C_WRITE_ENABLE();
  I2C_Delay();
  
    /* start condition */
    StartCondition();
  //AX8703_ADDRESS &= 0xFE;   // Force WRITE address
    /* send device ID and write data */
    if(!Send_Byte(AX8703_ADDRESS))
    {
        if(!Send_Byte(voldata&0x3f))
        {
        	if(!Send_Byte(flagmute?0xf1:0xf0))
          	{ 
                     StopCondition();
		    	return(0); 
		 }           
	            
        }
    }    
    StopCondition();

    return(1);

	
}

#endif


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
