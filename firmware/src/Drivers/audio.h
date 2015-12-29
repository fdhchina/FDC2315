#ifndef _AUDIO_H
#define _AUDIO_H
#if(GC_ZR&GC_ZR_GPIO)
enum	{
	srcInvaild=0,
	srcAux,
	srcMP3,
	srcTuner,
	srcPSTN,
};
#else
enum	{
	srcInvaild=0,
	srcAux,
	srcMP3,
	srcTuner,
};
#endif
void audio_Init(void);
// 音源选择
void audio_setSource(u8 nSource);
u8 audio_getSource(void);
// Aux电源控制
void audio_AuxPowerOpen(BOOL	bOpened);
BOOL audio_GetAuxPowerState(void);
// 功放电源控制
void audio_AmpPowerOpen(BOOL	bOpened);
BOOL audio_GetAmpPowerState(void);

void audio_setSourceMute(BOOL bMuted);
void audio_setOutputMute(BOOL bMuted);

BOOL audio_GetPortState(BYTE nPort);
#if (GC_ZR & GC_ZR_FC)
void audio_SetPortState(WORD nPort, BOOL bState);
void audio_SetBjPortState(WORD nState, BOOL type);
void audio_SetAllPortState(WORD nState);
#else
void audio_SetPortState(BYTE nPort, BOOL bState);
void audio_SetAllPortState(BYTE nState);
#endif


// 话筒插入否
BOOL mic_IsValid(void);

u16 adc_ReadAmpSingle(void);
BOOL adc_ReadMicSingle(void);

// ==================================================================================
extern volatile BOOL ADC1_Ok;
// extern void DMA1ReConfig(void);
extern void adc1_Start(BOOL bStart);
// ==================================================================================


#endif
