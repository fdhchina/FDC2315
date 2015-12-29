/**
  ******************************************************************************
  * @file I2C/M24C08_EEPROM/i2c_ee.h 
  * @author  MCD Application Team
  * @version  V3.0.0
  * @date  04/06/2009
  * @brief  Header for i2c_ee.c module
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

/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __I2C_EE_H
#define __I2C_EE_H

/* Includes ------------------------------------------------------------------*/


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* The FM24CL04 contains 2 blocks (256byte each)    */
/* EEPROM Addresses defines */
#define EEPROM_Block0_ADDRESS 0xA0    //PAGE1 256 BYTE
//#define EEPROM_Block1_ADDRESS 0xA2   //0XA2  PAGE 2  256 BYTE
#if(GC_ZR&GC_ZR_GPIO)
#define  I2C_GPIOX GPIOD

#define  I2C_SDA_GPIOPIN  GPIO_Pin_13
#define  I2C_SCL_GPIOPIN  GPIO_Pin_12
#else
#define  I2C_GPIOX GPIOB

#define  I2C_SDA_GPIOPIN  GPIO_Pin_7
#define  I2C_SCL_GPIOPIN  GPIO_Pin_6
#endif
void I2C_InitGPIO(void);
unsigned char I2CReadByte(unsigned char cDevAddr, unsigned char cReg);
unsigned char I2CWriteByte(unsigned char cDevAddr, unsigned char cReg, unsigned char cData);
unsigned char I2CWriteNBytes(unsigned char cDevAddr, unsigned short cReg, unsigned char cRegmode, unsigned char *cData1,unsigned char nums);
unsigned char I2CReadNBytes(unsigned char cDevAddr, unsigned short cReg, unsigned char cRegmode, unsigned char *pString, unsigned char cNum);

#if 0
unsigned char AX8703_I2C_ByteWrite(unsigned char voldata, unsigned char flagmute);
extern volatile unsigned char audio_vol;
extern unsigned char AX8703_I2C_ByteWrite(unsigned char voldata, unsigned char flagmute);
#define  AX8703_VOL() 	AX8703_I2C_ByteWrite(audio_vol,0)
#define AX8703_MuteEnable()  	AX8703_I2C_ByteWrite(audio_vol,1)
#define  AX8703_MuteDisable()	AX8703_I2C_ByteWrite(audio_vol,0)
#endif


#endif /* __I2C_EE_H */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/


