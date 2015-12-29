/***************************************************
**HAL.c
**主要用于芯片硬件的内部外围和外部外围的初始化，两大INIT函数
**在MAIN中调用，使MAIN函数中尽量与硬件库无关
***************************************************/

#include "hal.h"


//各个内部硬件模块的配置函数
extern void GPIO_Configuration(void);			//GPIO
extern void RCC_Configuration(void);			//RCC
//extern void USART_Configuration(void);			//USART
extern void CH375_Configuration(void);
extern void lcm_Configuration(void);
extern void sst25_Init(void);
extern void kb_Init(void);
/*******************************
**函数名:ChipHalInit()
**功能:片内硬件初始化
*******************************/
void  ChipHalInit(void)
{
	//初始化时钟源
	RCC_Configuration();
	
	//串口初始化
	// USART_Configuration();

	CH375_Configuration();

	lcm_Configuration();
	
}


/*********************************
**函数名:ChipOutHalInit()
**功能:片外硬件初始化
*********************************/
void  ChipOutHalInit(void)
{
	kb_Init();
#if(_HAVE_FONTLIB_==FONT_EXTERN)	
	sst25_Init();
#endif
}
