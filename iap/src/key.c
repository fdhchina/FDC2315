/**************************************************************
key.c 键盘的底层应用,暂时是16个按键的扫描
***************************************************************/

#include "hal.h"


void key_out_no()
{
	GPIO_SetBits(GPIOE,GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_6);
	GPIO_SetBits(GPIOC,GPIO_Pin_14);
}
void key_out_all()
{
	GPIO_ResetBits(GPIOE, GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_6);
	GPIO_ResetBits(GPIOC, GPIO_Pin_14);
}
u8 key_in()
{
	u8 ret=0;
	ret |= (!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2)) << 3;
	ret |= (!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_1) )<< 2;
	ret |= (!GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_15)) << 1;
	ret |= (!GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) )<<0;
	return ((~ret)&0x0f);
}
#define KEY_OUT_LINE_NULL		key_out_no()	/*所有输出为高*/
#define KEY_OUT_LINE1 	GPIOE->BRR=GPIO_Pin_3
#define KEY_OUT_LINE2  GPIOE->BRR=GPIO_Pin_4
#define KEY_OUT_LINE3	GPIOE->BRR=GPIO_Pin_6
#define KEY_OUT_LINE4  GPIOC->BRR=GPIO_Pin_14
#define KEY_OUT_LINE_ALL  key_out_all()	/*所有输出为低*/
#define KEY_IN   key_in()

static void keyscan_delay()
{
	u8 cnt=20;
	while(cnt--);
}

void Key_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
//	EXTI_InitTypeDef EXTI_InitStructure;
	
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOE, ENABLE);

	/*PE1,2,PC13,15上拉输入 检测脚*/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		//上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_15;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/*PE3,4,6,PC14推挽输出*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/*报警输入PA12紧急按键PE5*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

#if 0
	/* 连接IO口到中断线 */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource8);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource9);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource10);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource11);
	/*配置为下降沿触发*/
	EXTI_InitStructure.EXTI_Line = EXTI_Line8|EXTI_Line9|EXTI_Line10|EXTI_Line11;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	// 键盘使用EXTI8-EXTI11中断
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn| EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

#endif
}

/**************************************************************
** 函数名:pollingkey
** 功能:扫描键盘
** 注意事项:这个扫描过程是不延迟的,此函数仅为扫描键盘的硬件底层, 
**  		用户函数需要多番调用此函数实现消抖,
**  		可在此函数中实现键值的转换,也可在键盘应用层在转换
***************************************************************/
//硬件搜索键盘,暂时不支持重键
u16 PollingKey(void)
{
	//u8 i;
	u16 hal_key=0;

	//再到数值键
	KEY_OUT_LINE_NULL;
	KEY_OUT_LINE1;
	keyscan_delay();
	hal_key|=KEY_IN;

	KEY_OUT_LINE_NULL;
	KEY_OUT_LINE2;
	keyscan_delay();
	hal_key|=KEY_IN<<4;

	KEY_OUT_LINE_NULL;
	KEY_OUT_LINE3;
	keyscan_delay();
	hal_key|=KEY_IN<<8;

	KEY_OUT_LINE_NULL;
	KEY_OUT_LINE4;
	keyscan_delay();
	hal_key|=KEY_IN<<12;
	
	KEY_OUT_LINE_ALL;
	hal_key= ~hal_key;
	return hal_key;			// 返回扫描结果
	//转换键值-简单的转换一下,以数值键中以低键优先
	/*for(i=1;i<17;i++) 
	{
		if(hal_key&0x01)
		{
			return i;
		}
		hal_key>>=1;
	}*/

	//return(0);
}


