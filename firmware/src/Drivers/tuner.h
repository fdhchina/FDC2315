#ifndef _TUNER_H_
#define _TUNER_H_

typedef enum _tuner_band{
	tbFM=0,
	tbAM
}tuner_band_t;

#define FREQ_ADD 1
#define FREQ_DEC 0

// extern char const bandname_tab[2][3];
u16	tuner_getMaxFreq(u8 byBand);
u16	tuner_getMinFreq(u8 byBand);
u16	tuner_getStepFreq(u8 byBand);
BOOL tuner_Init(u8 band, u16 freq);
u8 tuner_getBand(void);
void tuner_ChangeBand(void);
u8 tuner_getValidSingle(u8 byBand);
u8 tuner_getStatus(u16 *freq);
// ��������Ƶ��
u8 tuner_setFreq(u16 freq);
u8 tuner_Seek(u8 dir);
BOOL tuner_FreqInc(void);
BOOL tuner_FreqDec(void);
void tuner_setMute(BOOL bMuted);

BOOL tuner_IsSearching(void);
void tuner_SearchStart(void);
BOOL tuner_SearchDo(void);
void tuner_SearchStop(void);

#if (GC_ZR & GC_ZR_TUNER)
void tuner_SaveAll(void);
#endif
// -------------------------------------------------------------------------------
// ���ز�����ʾ�ַ���
char *tuner_getBandName(u8 byBand);

// -------------------------------------------------------------------------------
// ����Ƶ����ʾ�ַ���
// str�������11�ֽ�
char *tuner_getFreqString(u8 byBand, u16 freq, char *str);

void tuner_setBand(u8 byBand);

// --------------------------------------------------------------------------------------------------
BOOL tuner_LoadParam(void);
// ��ȡ��̨Ƶ��
// ����byBand: ����;byIndex: 0~BANDSTATION_COUNT-1
// ���ص�̨���ڷ��ص�̨Ƶ�ʣ����򷵻ظò������Ƶ��
void tuner_saveFreq(u8 byIndex);
u8 tuner_getStationCount(u8 byBand);
u16 tuner_getStationFreq(u8 byBand, u8 byIndex);
u8 tuner_setStationFreq(u8 byBand, u8 byIndex, u16 freq);
u16 tuner_getCurrentFreq(u8 byBand);
void tuner_setCurrentFreq(u8 byBand, u16 freq);
void tuner_removeStation(u8 byBand, u8 byIndex);

#endif
