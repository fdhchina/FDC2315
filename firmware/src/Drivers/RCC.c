#include "hal.h"

void RCC_Configuration(void)
{
	// RCC_ClocksTypeDef RCC_ClockFreq;
	
	SystemInit();//Դ��system_stm32f10x.c�ļ�,ֻ��Ҫ���ô˺���,������RCC������.�����뿴2_RCC

	/**************************************************
	��ȡRCC����Ϣ,������
	��ο�RCC_ClocksTypeDef�ṹ�������,��ʱ��������ɺ�,
	���������ֵ��ֱ�ӷ�ӳ�������������ֵ�����Ƶ��
	***************************************************/
	//RCC_GetClocksFreq(&RCC_ClockFreq);
	
	/* ������ÿ�ʹ�ⲿ����ͣ���ʱ��,����һ��NMI�ж�,����Ҫ�õĿ����ε�*/
	//RCC_ClockSecuritySystemCmd(ENABLE);
	
	//SYSTICK��Ƶ--1ms��ϵͳʱ���ж�
	if (SysTick_Config(SystemFrequency / 1000))
  	{ 
  	  	/* Capture error */ 
	    	while (1);
  	}
	// SysTick_CounterCmd(SysTick_Counter_Enable);
}


/********************************************
**������:SysTickDelay
**����:ʹ��ϵͳʱ�ӵ�Ӳ�ӳ�
**ע������:һ���,��Ҫ���ж��е��ñ�����,����������������.�������������ȫ���ж�,��Ҫʹ�ô˺���
********************************************/
void SysTickDelay(u16 dly_ms)
{
	Timer1=dly_ms;
	while(Timer1);
}

/*
void delay_us(u32 nus)
{   
	u32 temp;   
	SysTick->LOAD = 9*nus;   
	SysTick->VAL=0X00;//��ռ�����   
	SysTick->CTRL=0X01;//ʹ�ܣ����������޶����������ⲿʱ��Դ  
	do  {    
		temp=SysTick->CTRL;//��ȡ��ǰ������ֵ   
	}while((temp&0x01)&&(!(temp&(1<<16))));//�ȴ�ʱ�䵽��      
	SysTick->CTRL=0x00; //�رռ�����     
	SysTick->VAL =0X00; //��ռ����� 
}  
void delay_ms(u16 nms) 
{   
	u32 temp;   
	SysTick->LOAD = 9000*nms;   
	SysTick->VAL=0X00;//��ռ�����   
	SysTick->CTRL=0X01;//ʹ�ܣ����������޶����������ⲿʱ��Դ  
	do  {    
		temp=SysTick->CTRL;//��ȡ��ǰ������ֵ   
	}while((temp&0x01)&&(!(temp&(1<<16))));//�ȴ�ʱ�䵽��     
	SysTick->CTRL=0x00; //�رռ�����     
	SysTick->VAL =0X00; //��ռ����� 
} 

void SysTickDelay(u16 dly_ms)
{
	delay_ms(dly_ms);
}*/
	
