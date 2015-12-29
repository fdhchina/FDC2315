#include "stm32f10x.h"

RCC_ClocksTypeDef RCC_ClockFreq;

void RCC_Configuration(void)
{
	SystemInit();//源自system_stm32f10x.c文件,只需要调用此函数,则可完成RCC的配置.

	/**************************************************
	获取RCC的信息,调试用
	请参考RCC_ClocksTypeDef结构体的内容,当时钟配置完成后,
	里面变量的值就直接反映了器件各个部分的运行频率
	***************************************************/
	RCC_GetClocksFreq(&RCC_ClockFreq);
	
	/* 这个配置可使外部晶振停振的时候,产生一个NMI中断,不需要用的可屏蔽掉*/
	//RCC_ClockSecuritySystemCmd(ENABLE);
}

void delay_us(u32 nus)
{   
	u32 temp;   
	SysTick->LOAD = 9*nus;   
	SysTick->VAL=0X00;//清空计数器   
	SysTick->CTRL=0X01;//使能，减到零是无动作，采用外部时钟源  
	do  {    
		temp=SysTick->CTRL;//读取当前倒计数值   
	}while((temp&0x01)&&(!(temp&(1<<16))));//等待时间到达      
	SysTick->CTRL=0x00; //关闭计数器     
	SysTick->VAL =0X00; //清空计数器 
}  
void delay_ms(u16 nms) 
{   
	u32 temp;   
	SysTick->LOAD = 9000*nms;   
	SysTick->VAL=0X00;//清空计数器   
	SysTick->CTRL=0X01;//使能，减到零是无动作，采用外部时钟源  
	do  {    
		temp=SysTick->CTRL;//读取当前倒计数值   
	}while((temp&0x01)&&(!(temp&(1<<16))));//等待时间到达     
	SysTick->CTRL=0x00; //关闭计数器     
	SysTick->VAL =0X00; //清空计数器 
} 

void SysTickDelay(u16 dly_ms)
{
	delay_ms(dly_ms);
}
