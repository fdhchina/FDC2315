#include "hal.h"
#include "audio.h"
#include "tuner.h"
#include "lcm19264.h"
// 当前音源
static u8 m_audioSource= srcInvaild;
static BOOL m_AmpPowerOpened= FALSE, m_AuxPowerOpened= FALSE;
 u8 m_AmpPowerNum = 0;
void audio_Init()
{
#if(GC_ZR&GC_ZR_GPIO)  //AMP1 AMP2 AUX 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);

	// PA11--PC16短路输出  PC3-4051~A, PC1-4051~B,  PC0-4051~C,PC10-Amplifier1 Power,  PC11-Amplifier2 Power,PC12-Pre Power, 分区1~分区6  PA8,PC9,PC8,PC7,PC6,PD15 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_9 | GPIO_Pin_8 | GPIO_Pin_7 | GPIO_Pin_6 | GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		// 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	// 50M时钟速度
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_11;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_SetBits(GPIOC, GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_0); //A,,B,C初始化为0，0，0 选择CH0（无输出）
	GPIO_SetBits(GPIOC, GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_9 | GPIO_Pin_8 | GPIO_Pin_7 | GPIO_Pin_6);
	GPIO_SetBits(GPIOA, GPIO_Pin_8 | GPIO_Pin_11); GPIO_SetBits(GPIOD, GPIO_Pin_15);  //功放1，2，AUX，分区初始化为高电平（关）
#else
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE);

	//PA1-4051~A, PA2-4051~B, PC6-Amplifier Power,  PA12-Pre Power, PA8~PA11-分区1~分区4
	GPIO_SetBits(GPIOA, GPIO_Pin_1 | GPIO_Pin_2);
	GPIO_SetBits(GPIOA, GPIO_Pin_8| GPIO_Pin_9| GPIO_Pin_10| GPIO_Pin_11|GPIO_Pin_12);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_8| GPIO_Pin_9| GPIO_Pin_10| GPIO_Pin_11| GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		// 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	// 50M时钟速度
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOC, GPIO_Pin_6);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// PC7-话筒信号检测0~有信号1~无信号
	GPIO_SetBits(GPIOC, GPIO_Pin_7);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	// 浮空输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		// 50M时钟速度
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	adc_GPIOConfiguration(); 
	//初始化DMA
	adc_NVICConfiguration();
#endif
}

u8 audio_getSource()
{
	return m_audioSource;
}

void _setSource(u8 nSrc)
{
#if(GC_ZR&GC_ZR_GPIO)
	switch(nSrc){
		case srcInvaild: //CH0 CBA--000--(IO口111)
			GPIO_SetBits(GPIOC, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3 );
			break;
		case srcAux: //CH2 CBA--010--(IO口101)
			GPIO_SetBits(GPIOC, GPIO_Pin_0 | GPIO_Pin_3);
			GPIO_ResetBits(GPIOC,GPIO_Pin_1);
			break;
		case srcMP3://CH4 CBA--100--(IO口011)
			GPIO_SetBits(GPIOC, GPIO_Pin_1 | GPIO_Pin_3);
			GPIO_ResetBits(GPIOC, GPIO_Pin_0);
			break;
		case srcTuner: //CH6 CBA--110--(IO口001)
			GPIO_SetBits(GPIOC, GPIO_Pin_3);
			GPIO_ResetBits(GPIOC, GPIO_Pin_0|GPIO_Pin_1);
			break;
		case srcPSTN://CH1 CBA--001--(IO口110)
			GPIO_SetBits(GPIOC, GPIO_Pin_0 | GPIO_Pin_1);
			GPIO_ResetBits(GPIOC,GPIO_Pin_3);
			break;
	}
#else
	nSrc--;
	if(nSrc&0x01)
		GPIO_SetBits(GPIOA, GPIO_Pin_1);
	else
		GPIO_ResetBits(GPIOA, GPIO_Pin_1);
	if(nSrc&0x02)
		GPIO_SetBits(GPIOA, GPIO_Pin_2);
	else
		GPIO_ResetBits(GPIOA, GPIO_Pin_2);
#endif
}
void audio_setSource(u8 nSrc)
{
	if(m_audioSource==nSrc) return;
	m_audioSource= nSrc;

	if(nSrc==srcAux)
	{
		audio_AuxPowerOpen(TRUE);
		DelayMS(100);
	}
	_setSource(nSrc);
	// GPIOA->BSRR= nSrc|(((~(u32)nSrc)<<16)&0x00060000);
	if((nSrc!=srcAux)&&m_AuxPowerOpened)
	{
		DelayMS(100);
		audio_AuxPowerOpen(FALSE);
	}
	tuner_setMute((BOOL)(m_audioSource!=srcTuner));
}

void audio_setSourceMute(BOOL bMuted)
{
	if(bMuted)
		_setSource(srcInvaild);
	else
		_setSource(m_audioSource);
}

void audio_setOutputMute(BOOL bMuted)
{
}


void audio_AuxPowerOpen(BOOL bOpened)
{
	m_AuxPowerOpened = bOpened;
#if(GC_ZR&GC_ZR_GPIO)
	if(bOpened)
		GPIO_ResetBits(GPIOC, GPIO_Pin_12);
	else
		GPIO_SetBits(GPIOC, GPIO_Pin_12);
#else
	if(bOpened)
		GPIO_ResetBits(GPIOA, GPIO_Pin_12);
	else
		GPIO_SetBits(GPIOA, GPIO_Pin_12);
#endif
}

BOOL audio_GetAuxPowerState()
{
	return m_AuxPowerOpened;
}


void AutoAmpRun() //每秒执行一次
{
	if (m_AmpPowerOpened)
	{
		switch (m_AmpPowerNum)
		{
		case 0:
			m_AmpPowerNum = 1;
			break;
		case 1:
			m_AmpPowerNum = 2;
			break;
		default:
			break;
		}
	}else
	{
		switch (m_AmpPowerNum)
		{
		case 2:
			m_AmpPowerNum = 1;
			break;
		case 1:
			m_AmpPowerNum = 0;
			break;
		default:
			break;
		}
	}
	if (m_AmpPowerNum == 0)
		GPIO_SetBits(GPIOC, GPIO_Pin_11 | GPIO_Pin_10);
	else if (m_AmpPowerNum == 1)
	{
		GPIO_ResetBits(GPIOC ,GPIO_Pin_10); //功放1开
		GPIO_SetBits(GPIOC, GPIO_Pin_11);//功放2关
	}
	else if (m_AmpPowerNum==2)
		GPIO_ResetBits(GPIOC ,GPIO_Pin_10|GPIO_Pin_11); //功放1,2开
	if (m_AmpPowerOpened) //PC16 短路输出
		GPIO_ResetBits(GPIOA, GPIO_Pin_11);		
	else
		GPIO_SetBits(GPIOA,GPIO_Pin_11);
}

void audio_AmpPowerOpen(BOOL	bOpened)
{
	m_AmpPowerOpened= bOpened;
#if (!(GC_ZR&GC_ZR_GPIO))
	if(bOpened)
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_6);
		adc1_Start(TRUE);
	}	else
	{
		adc1_Start(FALSE);
		GPIO_ResetBits(GPIOC, GPIO_Pin_6);
	}
#endif
}

BOOL audio_GetAmpPowerState()
{
	return m_AmpPowerOpened;
}
#if (GC_ZR & GC_ZR_FC)//XXX
// ================================================================================
// 分区控制
//static GPIO_TypeDef * const portname_tab[] = { GPIOA, GPIOC, GPIOC, GPIOC, GPIOC, GPIOD };
// 分区控制端口引脚表
//static u16 const portpin_tab[] = { GPIO_Pin_8, GPIO_Pin_9, GPIO_Pin_8, GPIO_Pin_7, GPIO_Pin_6, GPIO_Pin_15 };
static GPIO_TypeDef * const portname_tab[] = { GPIOD, GPIOC, GPIOC, GPIOC, GPIOC, GPIOA };
// 分区控制端口引脚表
static u16 const portpin_tab[] = { GPIO_Pin_15, GPIO_Pin_6, GPIO_Pin_7, GPIO_Pin_8, GPIO_Pin_9, GPIO_Pin_8 };

static u16 m_PortState = 0xFFFF;
#else
// ================================================================================
// 分区控制
static GPIO_TypeDef * const portname_tab[] = { GPIOA, GPIOA, GPIOA, GPIOA };
// 分区控制端口引脚表
static u16 const portpin_tab[] = { GPIO_Pin_11, GPIO_Pin_10, GPIO_Pin_9, GPIO_Pin_8 };

static u8 m_PortState = 0xFF;
#endif

#if (GC_ZR & GC_ZR_FC)
BOOL audio_GetPortState(BYTE nPort/* 0~15 */)
{
	return (BOOL)((m_PortState&(1<<nPort)) > 0);
}
#else
BOOL audio_GetPortState(BYTE nPort/* 0~15 */)
{
	return (BOOL)(m_PortState&(1 << nPort));
}
#endif
void _SetPortState(BYTE nPort, BOOL bState)
{
	if(bState)
	{
		GPIO_ResetBits(portname_tab[nPort], portpin_tab[nPort]);
	}	else
	{
		GPIO_SetBits(portname_tab[nPort], portpin_tab[nPort]);
	}
}

#if (GC_ZR & GC_ZR_FC)
void _SetFC16PortState(WORD nState, BOOL type) {
	u8 sendCode[10];
	u8 i;
	
	sendCode[0] = 0x85;
	sendCode[1] = 0x07;
	sendCode[2] = 0x00;
	if (type == FALSE) {
		sendCode[3] = (nState & 0x7F);
		sendCode[4] = ((nState >> 7) & 0x7F);
		sendCode[5] = ((nState >> 14) & 0x03);
		sendCode[6] = 0x00;
		sendCode[7] = 0x00;
	}
	else {
		sendCode[3] = 0;
		sendCode[4] = 0;
		sendCode[5] = (nState & 0x7C);
		sendCode[6] = ((nState >> 5) & 0x7F);
		sendCode[7] = ((nState >> 12) & 0x0F);
	}
	sendCode[8] =0;
	for (i = 0; i < 8; i++) {
		sendCode[8] += sendCode[i];
	} 
	sendCode[8] = sendCode[8]&0x7F;
	
	UART_PutStr( USART1, sendCode,9);
	SysTickDelay(1);
	UART_PutStr( USART1, sendCode,9);
	//USART1_Putcs(sendCode, 9);
	//USART1_Putcs(sendCode, 9);
}

void audio_SetPortState(WORD nPort/* 0~15 */, BOOL bState)
{
	u16 byState;
	byState= 1<<nPort;
	if(bState)
	{
		m_PortState |= byState;
	}	else
	{
		m_PortState &= ~byState;
	}
	if (nPort <6)
		_SetPortState(nPort, bState);
	_SetFC16PortState(m_PortState, FALSE);
}

void audio_SetBjPortState(WORD nState, BOOL type) {
	int i;
	m_PortState = nState;
	for (i = 0; i<6; i++)
		_SetPortState(i, (BOOL)((nState&(1 << i))>0));
	_SetFC16PortState(m_PortState, type);
}

void audio_SetAllPortState(WORD nState)
{
	int i;
	m_PortState= nState;
	for(i=0;i<6;i++)
		_SetPortState(i, (BOOL)((nState&(1 << i))>0));
	_SetFC16PortState(m_PortState, FALSE);
}
#else
void audio_SetPortState(BYTE nPort/* 0~3 */, BOOL bState)
{
	u8 byState;
	if(nPort==0xFF)
	{
		audio_SetAllPortState(m_PortState);
		return;
	}
	byState = 1 << nPort;
	if (bState)
	{
		m_PortState |= byState;
	}
	else
	{
		m_PortState &= ~byState;
	}
	_SetPortState(nPort, bState);
}

void audio_SetAllPortState(BYTE nState)
{
	int i;
	m_PortState = nState;
	for (i = 0; i<4; i++)
		_SetPortState(i, (BOOL)((nState&(1 << i))>0));
}
#endif
/*
// 话筒插入否
BOOL mic_IsValid()
{
	if((GPIOD->IDR&0x0001)==0)
	{
		DelayMS(10);
		return (BOOL)((GPIOD->IDR&0x0001)==0);
	}
	return FALSE;
}
*/
// 话筒有信号否
BOOL adc_ReadMicSingle(void)
{
	return (BOOL)((GPIOC->IDR&GPIO_Pin_7)==0);
}

