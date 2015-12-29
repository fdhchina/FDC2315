/***************************************************
**HAL.c
**��Ҫ����оƬӲ�����ڲ���Χ���ⲿ��Χ�ĳ�ʼ��������INIT����
**��MAIN�е��ã�ʹMAIN�����о�����Ӳ�����޹�
***************************************************/

#include "hal.h"
#include "ds3231.h"

#if 0
#include "mass_mal.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_lib.h"
#endif

// ==================================================================================
// �������
GPIO_InitTypeDef GPIO_InitStructure;
SPI_InitTypeDef  SPI_InitStructure;

#if VS1003_INT_FLAG_EN | CH375_INT_MODE

EXTI_InitTypeDef EXTI_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;

#endif

FATFS	fs_sd;
FATFS	fs_usb;
u8 volatile Ms_1, Ms_8, Ms_128, Ms_256 , S_1;
u16 volatile Timer1,Timer2;
// ==================================================================================

//�����ڲ�Ӳ��ģ������ú���
extern void GPIO_Configuration(void);			//GPIO
extern void RCC_Configuration(void);			//RCC
extern void I2C_InitGPIO(void);
extern void lcm_Init(void);
//extern void USART_Configuration(void);			//USART
extern void sst25_Init(void);
extern void ResetSi47XX_2w(void);

/*******************************
**������:ChipHalInit()
**����:Ƭ��Ӳ����ʼ��
*******************************/
void  ChipHalInit(void)
{
	//��ʼ��ʱ��Դ
	RCC_Configuration();
	
	//��ʼ��GPIO
	//GPIO_Configuration();

	I2C_InitGPIO();
	//���ڳ�ʼ��
	Com_Init();	
}


/*********************************
**������:ChipOutHalInit()
**����:Ƭ��Ӳ����ʼ��
*********************************/
void  ChipOutHalInit(void)
{
	ResetSi47XX_2w();
#if(_HAVE_FONTLIB_==FONT_EXTERN)	// ������sst25VF��
	sst25_Init();
#endif
	//feng dai hang shi gua wa zi
	lcm_Init();
}

void strfmt(char * str, const char * fmt, ...)
{
    va_list ap;

    va_start(ap,fmt);
    vsprintf(str,fmt,ap);
    va_end(ap);
}

TMyDate	g_SysDate;
TMyTime g_SysTime;
u8		g_SysWeek;


BOOL GetSysTime()
{
	static u8 byOldSecond= 0xFF;
	if(rtc_GetTime(&g_SysTime))
	{
		if(byOldSecond!=g_SysTime.bySecond)	// ���б仯
		{
			byOldSecond= g_SysTime.bySecond;
			return TRUE;
		}
	}
	return FALSE;
}

void delay_us(u32 nus)
{
  int n;
  while(nus--)
  {
    for(n=0;n<72;n++);
  }
}

#if 0
/*******************************************************************************
* Function Name  : Set_USBClock
* Description    : Configures USB Clock input (48MHz)
* Input          : None.
* Return         : None.
*******************************************************************************/
void Set_USBClock(void)
{
  /* Select USBCLK source */
  RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
  
  /* Enable the USB clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}

/*******************************************************************************
* Function Name  : Enter_LowPowerMode
* Description    : Power-off system clocks and power while entering suspend mode
* Input          : None.
* Return         : None.
*******************************************************************************/
void Enter_LowPowerMode(void)
{
  /* Set the device state to suspend */
  bDeviceState = DEFAULT;
}

/*******************************************************************************
* Function Name  : Leave_LowPowerMode
* Description    : Restores system clocks and power while exiting suspend mode
* Input          : None.
* Return         : None.
*******************************************************************************/
void Leave_LowPowerMode(void)
{
  DEVICE_INFO *pInfo = &Device_Info;

  /* Set the device state to the correct state */
  if (pInfo->Current_Configuration != 0)
  {
    /* Device configured */
    bDeviceState = CONFIGURED;
  }
  else
  {
    bDeviceState = ATTACHED;
  }

}

/*******************************************************************************
* Function Name  : USB_Interrupts_Config
* Description    : Configures the USB interrupts
* Input          : None.
* Return         : None.
*******************************************************************************/
void USB_Interrupts_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Set the Vector Table base location at 0x08000000 */
  // NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);//jie+ �ж�������ΪFLASH
  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);//jie+ �ж�������ΪFLASH

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
/*jie-  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//jie+
  NVIC_Init(&NVIC_InitStructure);*/  
}

/*******************************************************************************
* Function Name  : USB_Cable_Config
* Description    : Software Connection/Disconnection of USB Cable.
* Input          : None.
* Return         : Status
*******************************************************************************/
void USB_Cable_Config (FunctionalState NewState)
{
#ifdef USE_STM3210C_EVAL  
  if (NewState != DISABLE)
  {
    USB_DevConnect();
  }
  else
  {
    USB_DevDisconnect();
  }
#else /* USE_STM3210B_EVAL or USE_STM3210E_EVAL */
  if (NewState != DISABLE)
  {
    GPIO_ResetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
  }
  else
  {
    GPIO_SetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
  }
#endif /* USE_STM3210C_EVAL */
}

/*******************************************************************************
* Function Name  : Get_SerialNum.
* Description    : Create the serial number string descriptor.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Get_SerialNum(void)
{
  uint32_t Device_Serial0, Device_Serial1, Device_Serial2;

  Device_Serial0 = *(__IO uint32_t*)(0x1FFFF7E8);
  Device_Serial1 = *(__IO uint32_t*)(0x1FFFF7EC);
  Device_Serial2 = *(__IO uint32_t*)(0x1FFFF7F0);

  if (Device_Serial0 != 0)
  {
    MASS_StringSerial[2] = (uint8_t)(Device_Serial0 & 0x000000FF);
    MASS_StringSerial[4] = (uint8_t)((Device_Serial0 & 0x0000FF00) >> 8);
    MASS_StringSerial[6] = (uint8_t)((Device_Serial0 & 0x00FF0000) >> 16);
    MASS_StringSerial[8] = (uint8_t)((Device_Serial0 & 0xFF000000) >> 24);

    MASS_StringSerial[10] = (uint8_t)(Device_Serial1 & 0x000000FF);
    MASS_StringSerial[12] = (uint8_t)((Device_Serial1 & 0x0000FF00) >> 8);
    MASS_StringSerial[14] = (uint8_t)((Device_Serial1 & 0x00FF0000) >> 16);
    MASS_StringSerial[16] = (uint8_t)((Device_Serial1 & 0xFF000000) >> 24);

    MASS_StringSerial[18] = (uint8_t)(Device_Serial2 & 0x000000FF);
    MASS_StringSerial[20] = (uint8_t)((Device_Serial2 & 0x0000FF00) >> 8);
    MASS_StringSerial[22] = (uint8_t)((Device_Serial2 & 0x00FF0000) >> 16);
    MASS_StringSerial[24] = (uint8_t)((Device_Serial2 & 0xFF000000) >> 24);
  }
}

/*******************************************************************************
* Function Name  : MAL_Config
* Description    : MAL_layer configuration
* Input          : None.
* Return         : None.
*******************************************************************************/
void MAL_Config(void)
{
  MAL_Init(0);//��ʼ��SD��
  //MAL_Init(1);//��ʼ��NAND Flash
}

/*******************************************************************************
* Function Name  : USB_Disconnect_Config
* Description    : Disconnect pin configuration
* Input          : None.
* Return         : None.
*******************************************************************************/
void USB_Disconnect_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable USB_DISCONNECT GPIO clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_DISCONNECT, ENABLE);

  /* USB_DISCONNECT_PIN used as USB pull-up */
  GPIO_InitStructure.GPIO_Pin = USB_DISCONNECT_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_Init(USB_DISCONNECT, &GPIO_InitStructure);

  //jie �ȶϿ�USB����
  GPIO_SetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
  //GPIO_ResetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
}
#endif
