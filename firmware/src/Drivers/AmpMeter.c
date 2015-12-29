#include "hal.h"
#include "ampMeter.h"

void meter_Configer()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// GPIOC
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = METER_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;	//50M时钟速度
	GPIO_Init(METER_Port, &GPIO_InitStructure);
}

void meter_Init()
{
	meter_Configer();
	meter_SetData(0);
}

#define CALC_METERVALUE(x) ((U16)(x*4096/3.3))
#define VALUE_INC	320
#define	VALUE_0		CALC_METERVALUE(0.19) // 10V // 250   // 0.20V
#define	VALUE_1		CALC_METERVALUE(0.40) // 10V // 420   // 0.33V (VALUE_0+VALUE_INC)
#define	VALUE_2		CALC_METERVALUE(0.82) // 10V // 770   // 0.62V (VALUE_1+VALUE_INC)
#define	VALUE_3		CALC_METERVALUE(1.21) // 10V // 1100  // 0.88V (VALUE_2+VALUE_INC)
#define	VALUE_4		CALC_METERVALUE(1.64) // 10V // 1450  // 1.16V (VALUE_3+VALUE_INC)
#define	VALUE_5		CALC_METERVALUE(1.97) // 10V // 1700  // 1.37V(VALUE_4+VALUE_INC)
void meter_SetData(u16 wLevel)
{
	u8 byRes=0;
#if 0	
	static u16 wOldLevel=0;
	if((wLevel<wOldLevel / 2)||(wLevel<200))
		wLevel= wOldLevel;
	else
		wOldLevel= wLevel;
#endif	
	if(wLevel>VALUE_0)
		byRes |= 1;
	if(wLevel>VALUE_1)
		byRes |= 2;
	if(wLevel>VALUE_2)
		byRes |= 4;
	if(wLevel>VALUE_3)
		byRes |= 8;
	if(wLevel>VALUE_4)
		byRes |= 0x10;
	if(wLevel>VALUE_5)
		byRes |= 0x20;
	
	METER_SET_DATA(~byRes);
}

