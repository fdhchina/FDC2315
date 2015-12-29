#ifndef AMPMETER_H_
#define AMPMETER_H_

// ========================================================================================
// Ӳ���˿�
// PC0-5������ƽָʾ��1-6���͵�ƽ����
#define METER_Port				GPIOC
#define METER_Pin				(GPIO_Pin_0 | GPIO_Pin_1 |GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5)
#define METER_SET_DATA(dat)		(GPIOC->BSRR=((dat&0x3F)|(((~dat)<<16)&0x003F0000)))

// ========================================================================================
void meter_Init(void);
void meter_SetData(u16 byLevel);

#endif

