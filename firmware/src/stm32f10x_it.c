/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : stm32f10x_it.c
* Author             : MCD Application Team
* Version            : V3.1.1
* Date               : 04/07/2010
* Description        : Main Interrupt Service Routines.
*                      This file provides template for all exceptions handler
*                      and peripherals interrupt service routine.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hal.h"
#include "stm32f10x_it.h"
#if 0
#include "usb_lib.h"
#include "usb_istr.h"
#include "usb_pwr.h"
#endif
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/*******************************************************************************
* Function Name  : NMI_Handler
* Description    : This function handles NMI exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NMI_Handler(void)
{
}

/*******************************************************************************
* Function Name  : HardFault_Handler
* Description    : This function handles Hard Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : MemManage_Handler
* Description    : This function handles Memory Manage exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : BusFault_Handler
* Description    : This function handles Bus Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : UsageFault_Handler
* Description    : This function handles Usage Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : SVC_Handler
* Description    : This function handles SVCall exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SVC_Handler(void)
{
}

/*******************************************************************************
* Function Name  : DebugMon_Handler
* Description    : This function handles Debug Monitor exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DebugMon_Handler(void)
{
}

/*******************************************************************************
* Function Name  : PendSV_Handler
* Description    : This function handles PendSVC exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PendSV_Handler(void)
{
}

/*******************************************************************************
* Function Name  : SysTick_Handler
* Description    : This function handles SysTick Handler.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

volatile u32 dwTickCount=0;
volatile u32 ampTickCount = 0;
void SysTick_Handler(void)
{
	if(Timer1)
		Timer1--;
	
	if(Timer2)
		Timer2--;
	
	//32MS标志
	if((dwTickCount&0x7)==0)
		Ms_8=1;
	if((dwTickCount&0x7F)==0)
		Ms_128=1;
	if((dwTickCount&0x1FF)==0)
		Ms_256=1;
#if(GC_ZR&GC_ZR_GPIO)
	if (ampTickCount == 1000)
	{
		S_1 = 1;
		ampTickCount = 0;
	}
#endif
	Ms_1=1;
	dwTickCount++;
	ampTickCount++;
}

/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/

#ifndef STM32F10X_CL
/*******************************************************************************
* Function Name  : USB_HP_CAN1_TX_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts requests
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_HP_CAN1_TX_IRQHandler(void)
{
  	// CTR_HP();
}

/*******************************************************************************
* Function Name  : USB_LP_CAN1_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
 	//USB_Istr();
}
#endif /* STM32F10X_CL */

#ifdef STM32F10X_HD
/*******************************************************************************
* Function Name  : SDIO_IRQHandler
* Description    : This function handles SDIO global interrupt request.
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SDIO_IRQHandler(void)
{ 
  /* Process All SDIO Interrupt Sources */
}
#endif /* STM32F10X_HD */

#ifdef STM32F10X_CL
/*******************************************************************************
* Function Name  : OTG_FS_IRQHandler
* Description    : This function handles USB-On-The-Go FS global interrupt request.
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void OTG_FS_IRQHandler(void)
{
 	STM32_PCD_OTG_ISR_Handler(); 
}
#endif /* STM32F10X_CL */

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/*******************************************************************************
* Function Name  : PPP_IRQHandler
* Description    : This function handles PPP interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/*void PPP_IRQHandler(void)
{
}*/
#if 0
/*******************************************************************************
* Function Name  : TIM2_IRQHandler TIM2中断
* Description    : This function handles TIM2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		/* Clear TIM2 update interrupt */
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	
		if ((Out_Data_Offset < In_Data_Offset) && ((u8)(MUTE_DATA) == 0))
		{
			TIM_SetCompare1(TIM1, Stream_Buff[Out_Data_Offset]);
			Out_Data_Offset++;
			if(Out_Data_Offset>=In_Data_Offset)
				Out_Data_Offset= 0;
		}
	}
}
#endif

extern volatile BOOL SD_changed;
extern volatile BOOL sd_insert_flag;
void EXTI3_IRQHandler(void)
{
	ITStatus status= EXTI_GetITStatus(EXTI_Line3);
	if(status==SET)
	{
		SD_changed= TRUE;
		// sd_insert_flag= (BOOL)(!(GPIOA->IDR&0x0008));
		EXTI_ClearFlag(EXTI_Line3);
	}
}

extern void _keyscan(void);
void EXTI9_5_IRQHandler(void)
{
	ITStatus status= EXTI_GetITStatus(EXTI_Line8|EXTI_Line9);
	if(status==SET)
	{
		_keyscan();
		EXTI_ClearFlag(EXTI_Line8|EXTI_Line9);
	}
}

extern void	ch375_irq( void );
void EXTI15_10_IRQHandler(void)
{
/*	
	ITStatus status= EXTI_GetITStatus(EXTI_Line13);
	if(status==SET)
	{
		ch375_irq();
		EXTI_ClearFlag(EXTI_Line13);
	}
	status= EXTI_GetITStatus(EXTI_Line10|EXTI_Line11);
	if(status==SET)
	{
		_keyscan();
		EXTI_ClearFlag(EXTI_Line10|EXTI_Line11);
	}
*/
}
#if (!(GC_ZR&GC_ZR_GPIO))
extern volatile BOOL ADC1_Ok;
extern void adc1_FillData(u16);
//extern void adc2_FillData(u16);
void ADC1_2_IRQHandler(void)
{
	if(ADC_GetITStatus(ADC1, ADC_IT_EOC)==SET)
	{
		adc1_FillData(ADC_GetConversionValue(ADC1));
		ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
	}
	/*status= ADC_GetITStatus(ADC2, ADC_IT_EOC);
	if(status==SET)
	{
		adc2_FillData(ADC_GetConversionValue(ADC2));
		ADC_ClearFlag(ADC2, ADC_FLAG_EOC);
	}*/
}
#endif
/*******************************************************************************
* Function Name  : DMAChannel1_IRQHandler
* Description    : This function handles DMA Stream 1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
// extern void dspAmplifierMeter(void);
#if (!(GC_ZR&GC_ZR_GPIO))
void DMA1_Channel1_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_IT_TC1))
 	{
		ADC_DMACmd(ADC1, DISABLE);
		DMA_Cmd(DMA1_Channel1, DISABLE);
		ADC_SoftwareStartConvCmd(ADC1, DISABLE);
 		
		DMA_ClearITPendingBit(DMA1_IT_GL1);	// 清除中断标志
		ADC1_Ok=TRUE;
		// dspAmplifierMeter();
	}
}
#endif
/*
void DMA2_Channel4_5_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA2_IT_TC5))
 	{
		DMA_ClearITPendingBit(DMA2_IT_GL5);	//清除全部中断标志
		ADC2_Ok=TRUE;
	}
}
*/

void USART1_IRQHandler(void)
{

}

extern u8 YK_Key; //KEY:1111  
extern u8 YK_SAVE;
extern char YK_Rev[12];

extern u8 DH_SAVE;
extern char DH_Rev[12];
u8 hx;
u8 jx_num;
void USART2_IRQHandler(void)
{
	static u8 tem=0; static u8 ix=0; static u8 jx=0; u8 i;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断
	{
		if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == SET)
		{
			//UART_PutChar(USART2, USART_ReceiveData(USART2));
			//while(USART_GetFlagStatus(USART2, USART_FLAG_TC) != SET);
			tem = USART_ReceiveData(USART2); 
			YK_Rev[ix]=tem;
			//UART_PutChar(USART2,jx);
			//遥控
			if(YK_Rev[0] == 'K')
			{
				ix++;
				if(ix==8)
				{
					ix=0;
					YK_Rev[8]='\0';
					YK_SAVE=1;
				}
			}
				//电话
		if(jx>12) 
			jx=0;
		if(tem == 0x68)
			jx=0;
		DH_Rev[jx]=tem;
		//UART_PutChar(USART2,DH_Rev[jx]);
		 if(DH_Rev[0] == 0x68) //'h'
			{			
				if(DH_Rev[jx] == 0x63) //收到gc 接受完成
				{
					if(DH_Rev[jx-1]==0x67)
					{
						DH_Rev[jx+1]='\0';
						DH_SAVE=1;
						jx_num=jx-3;
						jx=0; 
						return;  //退出中断
					}
				}
				jx++; 
			}
		} 
	}
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

