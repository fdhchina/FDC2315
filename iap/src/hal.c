/***************************************************
**HAL.c
**��Ҫ����оƬӲ�����ڲ���Χ���ⲿ��Χ�ĳ�ʼ��������INIT����
**��MAIN�е��ã�ʹMAIN�����о�����Ӳ�����޹�
***************************************************/

#include "hal.h"


//�����ڲ�Ӳ��ģ������ú���
extern void GPIO_Configuration(void);			//GPIO
extern void RCC_Configuration(void);			//RCC
//extern void USART_Configuration(void);			//USART
extern void CH375_Configuration(void);
extern void lcm_Configuration(void);
extern void sst25_Init(void);
extern void kb_Init(void);
/*******************************
**������:ChipHalInit()
**����:Ƭ��Ӳ����ʼ��
*******************************/
void  ChipHalInit(void)
{
	//��ʼ��ʱ��Դ
	RCC_Configuration();
	
	//���ڳ�ʼ��
	// USART_Configuration();

	CH375_Configuration();

	lcm_Configuration();
	
}


/*********************************
**������:ChipOutHalInit()
**����:Ƭ��Ӳ����ʼ��
*********************************/
void  ChipOutHalInit(void)
{
	kb_Init();
#if(_HAVE_FONTLIB_==FONT_EXTERN)	
	sst25_Init();
#endif
}
