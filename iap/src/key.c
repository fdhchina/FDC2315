/**************************************************************
key.c ���̵ĵײ�Ӧ��,��ʱ��16��������ɨ��
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
#define KEY_OUT_LINE_NULL		key_out_no()	/*�������Ϊ��*/
#define KEY_OUT_LINE1 	GPIOE->BRR=GPIO_Pin_3
#define KEY_OUT_LINE2  GPIOE->BRR=GPIO_Pin_4
#define KEY_OUT_LINE3	GPIOE->BRR=GPIO_Pin_6
#define KEY_OUT_LINE4  GPIOC->BRR=GPIO_Pin_14
#define KEY_OUT_LINE_ALL  key_out_all()	/*�������Ϊ��*/
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

	/*PE1,2,PC13,15�������� ����*/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		//��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_15;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/*PE3,4,6,PC14�������*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//�������
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/*��������PA12��������PE5*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

#if 0
	/* ����IO�ڵ��ж��� */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource8);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource9);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource10);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource11);
	/*����Ϊ�½��ش���*/
	EXTI_InitStructure.EXTI_Line = EXTI_Line8|EXTI_Line9|EXTI_Line10|EXTI_Line11;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	// ����ʹ��EXTI8-EXTI11�ж�
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn| EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

#endif
}

/**************************************************************
** ������:pollingkey
** ����:ɨ�����
** ע������:���ɨ������ǲ��ӳٵ�,�˺�����Ϊɨ����̵�Ӳ���ײ�, 
**  		�û�������Ҫ�෬���ô˺���ʵ������,
**  		���ڴ˺�����ʵ�ּ�ֵ��ת��,Ҳ���ڼ���Ӧ�ò���ת��
***************************************************************/
//Ӳ����������,��ʱ��֧���ؼ�
u16 PollingKey(void)
{
	//u8 i;
	u16 hal_key=0;

	//�ٵ���ֵ��
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
	return hal_key;			// ����ɨ����
	//ת����ֵ-�򵥵�ת��һ��,����ֵ�����Եͼ�����
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


