#include "hal.h"
#include "si47xx\si47xx.h"
#include "tuner.h"
#include "stmflash.h"
// ******************************************************************************************************************
#define TUNER_PARAM_VALID		0x55
// 收音数据参数结构
typedef struct	_tunerparam{
	u8	byValid;
	u8	byBand;						// 当前波段
	u8	byStationCounts[TUNERBAND_COUNT];	// 各波段电台数
	u16	wFreqs[TUNERBAND_COUNT];	// 各波段频率
	u16 wStationFreqs[TUNERBAND_COUNT][BANDSTATION_COUNT];	// 各波段电台频率
}TunerParam_t;

struct	{
	BOOL search_flag:1;
	BOOL autosearch_flag:1;
	u8 res:6;
	u16 wFreq;
}tuner_searchParam={FALSE, FALSE,  0};


TunerParam_t		tuner_Param;

#define	LO(x)		((unsigned char)(x&0xFF))
#define	HI(x)		((unsigned char)(x>>8))

char const bandname_tab[][3]={"FM","AM"};
u8 const bandmod_tab[2]={FM_RECEIVER, AM_RECEIVER};

static unsigned char read_buf[9];

u16 const tunerMinFreqs[]={FM_FREQ_MIN, AM_FREQ_MIN};
u16 const tunerMaxFreqs[]={FM_FREQ_MAX, AM_FREQ_MAX};
u8 const tunerStepFreqs[]={10, 9};
u8 const tunerValidSingles[]={0x18, 0x14};
// **************************************************************************************************************************
// 接收头初始化
BOOL tuner_Init(u8 band, u16 freq)
{
	tuner_Param.byBand= band;
	//if( Si47XX_Power_Down()!=OK) return FALSE;   // 2010-09-29 收音关闭马上重开时，会不能通过
	// Si47XX_Power_Down();
	if( Si47XX_Power_Up(bandmod_tab[tuner_Param.byBand])!=OK) return FALSE;
	if( Si47XX_Get_Rev(read_buf)!=OK) return FALSE;

	if(tuner_Param.byBand==tbFM){
		FM_SEEK_BAND_BOTTOM(HI(FM_FREQ_MIN), LO(FM_FREQ_MIN));	//0x22,0x2E);		// 87.5MHz
		FM_SEEK_BAND_TOP(HI(FM_FREQ_MAX), LO(FM_FREQ_MAX)); //0x2A,0x26);		// 107.9MHz
		FM_SEEK_SPACE(0x00,0x05);			// 步进频率为50KHz
	}else
		if(tuner_Param.byBand==tbAM)
		{
			AM_SEEK_BAND_BOTTOM(HI(AM_FREQ_MIN), LO(AM_FREQ_MIN));	// 0x02,0x0a);	  	// 522KHz
			AM_SEEK_BAND_TOP(HI(AM_FREQ_MAX), LO(AM_FREQ_MAX));	// 0x06,0x54);		// 1620KHz
			AM_SEEK_FREQ_SPACING(0x00,0x09);	// 9KHz 	
		}
	if(freq)
		tuner_Param.wFreqs[tuner_Param.byBand]= freq;
	Si47XX_Tune(bandmod_tab[tuner_Param.byBand], tuner_Param.wFreqs[tuner_Param.byBand]);	
	// Si47XX_Tune_Status(mod,0,1,&read_buf[0]);
	return TRUE;
}
// **************************************************************************************************************************

u8 tuner_getValidSingle(u8 byBand)
{
	if(byBand==0xFF)
		byBand= tuner_Param.byBand;
	return tunerValidSingles[byBand];
}

// **************************************************************************************************************************
u8 tuner_getStatus(u16 *freq)
{
	if(Si47XX_Tune_Status(bandmod_tab[tuner_Param.byBand], 0, 1, &read_buf[0])!=OK) return FALSE;
	if(freq)
		*freq= (read_buf[2]<<8) |read_buf[3];
	// read_buf[4] 是接收信号强度值
/*
	if(((*(p+1)&0x80) == 0))
		SeekFail = 0;
	else
		SeekFail = 1;		
	if((*(p+1)&0x01) == 0)
		valid_channel = 0;
	else
		valid_channel = 1;	
*/
	if (read_buf[1]&0x01)
		return read_buf[4];
	return 0;
}
// **************************************************************************************************************************

// **************************************************************************************************************************
// 设置收音频率
u8 tuner_setFreq(u16 freq)
{
	if(freq)
	{
		tuner_Param.wFreqs[tuner_Param.byBand]= freq;
		if(Si47XX_Tune(bandmod_tab[tuner_Param.byBand], freq)!=OK) 
			return 0;
		return tuner_getStatus(NULL);
	}
	return 0;
}
// **************************************************************************************************************************

// **************************************************************************************************************************
// 设置收音静音
void tuner_setMute(BOOL bMuted)
{
	RX_HARD_MUTE(bMuted?0x0003:0x0000);
}
// **************************************************************************************************************************

// **************************************************************************************************************************
// 搜索电台
u8 tuner_Seek(u8 dir)
{
	if(dir)
	{
		if(tuner_Param.wFreqs[tuner_Param.byBand]<tunerMaxFreqs[tuner_Param.byBand])
			tuner_Param.wFreqs[tuner_Param.byBand]+= tunerStepFreqs[tuner_Param.byBand];
		else
			tuner_Param.wFreqs[tuner_Param.byBand]= tunerMinFreqs[tuner_Param.byBand];
	}else
	{
		if(tuner_Param.wFreqs[tuner_Param.byBand]>tunerMinFreqs[tuner_Param.byBand])
			tuner_Param.wFreqs[tuner_Param.byBand]-= tunerStepFreqs[tuner_Param.byBand];
		else
			tuner_Param.wFreqs[tuner_Param.byBand]= tunerMaxFreqs[tuner_Param.byBand];
	}
	return Si47XX_Tune(bandmod_tab[tuner_Param.byBand], tuner_Param.wFreqs[tuner_Param.byBand]);
}


// **************************************************************************************************************************
// 收音的相关函数

// -------------------------------------------------------------------------------
// 返回波段显示字符串
char *tuner_getBandName(u8 byBand)
{
	if(byBand==0xFF)
		byBand= tuner_Param.byBand;
	return (char *)bandname_tab[byBand];
}

// 返回频率显示字符串
char *tuner_getFreqString(u8 byBand, u16 freq, char *str)
{
	if(byBand==0xFF)
		byBand= tuner_Param.byBand;
	if(freq==0)
		if(tuner_searchParam.search_flag)
			freq= tuner_searchParam.wFreq;
		else
			freq= tuner_Param.wFreqs[byBand];
	switch(byBand)
	{
	case tbFM:
		freq /= 10;
		strfmt(str,"%3d.%dMHz" ,  freq / 10, freq % 10);
		break;
	case tbAM:
		strfmt(str," %4dKHz", freq);
		break;
	}	
	return str;
}

u16	tuner_getMaxFreq(u8 byBand)
{
	if(byBand==0xFF)
		byBand= tuner_Param.byBand;
	return tunerMaxFreqs[byBand];
}

u16	tuner_getMinFreq(u8 byBand)
{
	if(byBand==0xFF)
		byBand= tuner_Param.byBand;
	return tunerMinFreqs[byBand];
}

u16	tuner_getStepFreq(u8 byBand)
{
	if(byBand==0xFF)
		byBand= tuner_Param.byBand;
	return tunerStepFreqs[byBand];
}

void tuner_SaveParam(void)
{
	if(tuner_searchParam.search_flag) return;
	tuner_Param.byValid= TUNER_PARAM_VALID;
#if(_HAVE_FONTLIB_==FONT_EXTERN)	// 保存在sst25VF中
	
	sst25_SectorErase(sacTunerParam);
	sst25_Write(sacTunerParam, (u8 *)&tuner_Param, sizeof(TunerParam_t));
#else
	STMFLASH_Write(sacTunerParam, (u16 *)&tuner_Param, sizeof(TunerParam_t)>>1);
#endif
}
BOOL tuner_LoadParam(void)
{
	// u8 i;
#if(_HAVE_FONTLIB_==FONT_EXTERN)	// 保存在sst25VF中
	sst25_Read(sacTunerParam, (u8 *)&tuner_Param, sizeof(TunerParam_t));
#else
	STMFLASH_Read(sacTunerParam, (u16 *)&tuner_Param, sizeof(TunerParam_t)>>1);
#endif
	if(tuner_Param.byValid!=TUNER_PARAM_VALID)
	{
		tuner_Param.byBand= tbFM;
		tuner_Param.wFreqs[tbAM]= tunerMinFreqs[tbAM];
		tuner_Param.wFreqs[tbFM]= tunerMinFreqs[tbFM];
		tuner_Param.byStationCounts[tbFM]= 0;
		tuner_Param.byStationCounts[tbAM]= 0;
		/* for(i=0;i<BANDSTATION_COUNT;i++)
		{
			tuner_Param.wStationFreqs[tbFM][i]= tunerMinFreqs[tbFM];
			tuner_Param.wStationFreqs[tbAM][i]= tunerMinFreqs[tbAM];
		}*/
	}	
	return TRUE;
}

#if (GC_ZR & GC_ZR_TUNER)
void tuner_SaveAll(void) {
	tuner_SaveParam();
}
#endif

u8 tuner_getBand()
{
	return tuner_Param.byBand;
}

void tuner_setBand(u8 byBand)
{
	if(tuner_Param.byBand!=byBand)
	{
		tuner_Init(byBand, 0);
		tuner_SaveParam();
	}
}

void tuner_ChangeBand()
{
	u8 const band_tab[]={tbAM, tbFM};
	tuner_setBand(band_tab[tuner_Param.byBand]);
}
// --------------------------------------------------------------------------------------------------
// 获取电台频率
// 参数byBand: 波段;byIndex: 0~BANDSTATION_COUNT-1
// 返回电台存在返回电台频率，否则返回该波段最低频率
u8 tuner_getStationCount(u8 byBand)
{
	if(byBand==0xFF)
		byBand= tuner_Param.byBand;
	return tuner_Param.byStationCounts[byBand];
}

u16 tuner_getStationFreq(u8 byBand, u8 byIndex)
{
	if(byBand==0xFF)
		byBand= tuner_Param.byBand;
	if(byIndex<tuner_Param.byStationCounts[byBand])
		return tuner_Param.wStationFreqs[byBand][byIndex];
	return tunerMinFreqs[byBand];
}

void tuner_saveFreq(u8 byIndex) {
	u8 i;
	tuner_Param.wStationFreqs[tuner_Param.byBand][byIndex] = tuner_Param.wFreqs[tuner_Param.byBand];
	i = tuner_Param.byStationCounts[tuner_Param.byBand];
	if((i<(byIndex+1))&&(byIndex<40))
		tuner_Param.byStationCounts[tuner_Param.byBand]=byIndex+1;
}

// 返回电台总数
u8 tuner_setStationFreq(u8 byBand, u8 byIndex, u16 freq)
{
	u8 i;
	if(byBand==0xFF)
		byBand= tuner_Param.byBand;
	if(freq==0)
		freq= tuner_Param.wFreqs[byBand];
	if(byIndex==0xFF)
		byIndex= tuner_Param.byStationCounts[byBand];
	tuner_Param.wStationFreqs[byBand][byIndex]= freq;
	i= tuner_Param.byStationCounts[byBand];
	tuner_Param.byStationCounts[byBand]= byIndex+1;
	for(;i<byIndex; i++)
		tuner_Param.wStationFreqs[byBand][i]= tunerMinFreqs[byBand];
	tuner_SaveParam();
	return tuner_Param.byStationCounts[byBand];
}

u16 tuner_getCurrentFreq(u8 byBand)
{
	if(byBand==0xFF)
		byBand= tuner_Param.byBand;
	return tuner_Param.wFreqs[byBand];
}

void tuner_setCurrentFreq(u8 byBand, u16 freq)
{
	if(byBand==0xFF)
		byBand= tuner_Param.byBand;
	if(tuner_Param.wFreqs[byBand]!= freq)
	{
		tuner_Param.wFreqs[byBand]= freq;
		if(byBand==tuner_Param.byBand)
			tuner_setFreq(freq);
		tuner_SaveParam();
	}
}

void tuner_removeStation(u8 byBand, u8 byIndex)
{
	if(byBand==0xFF)
		byBand= tuner_Param.byBand;
	tuner_Param.byStationCounts[byBand]--;
	for(;byIndex<tuner_Param.byStationCounts[byBand]; byIndex++)
		tuner_Param.wStationFreqs[byBand][byIndex]= tuner_Param.wStationFreqs[byBand][byIndex+1];
	tuner_SaveParam();
}

BOOL tuner_FreqInc()
{
	if(tuner_Param.wFreqs[tuner_Param.byBand]<tunerMaxFreqs[tuner_Param.byBand])
	{
		tuner_Param.wFreqs[tuner_Param.byBand] += tunerStepFreqs[tuner_Param.byBand];
		Si47XX_Tune(bandmod_tab[tuner_Param.byBand], tuner_Param.wFreqs[tuner_Param.byBand]); 
		return TRUE;
	}
	return FALSE;
}

BOOL tuner_FreqDec()
{
	if(tuner_Param.wFreqs[tuner_Param.byBand]>tunerMinFreqs[tuner_Param.byBand])
	{
		tuner_Param.wFreqs[tuner_Param.byBand] -= tunerStepFreqs[tuner_Param.byBand];
		Si47XX_Tune(bandmod_tab[tuner_Param.byBand], tuner_Param.wFreqs[tuner_Param.byBand]);
		return TRUE;
	}
	return FALSE;
}


BOOL tuner_IsSearching()
{
	return tuner_searchParam.search_flag;
}

void tuner_SearchStart()
{
	RX_HARD_MUTE(0x03);
	tuner_searchParam.search_flag= TRUE;
	tuner_searchParam.autosearch_flag= TRUE;
	tuner_searchParam.wFreq= tunerMinFreqs[tuner_Param.byBand];
	tuner_Param.byStationCounts[tuner_Param.byBand]= 0;
}

BOOL tuner_SearchDo(void)
{
	u8 nCnt;
	BOOL bSearch= tuner_searchParam.search_flag;
	if(tuner_searchParam.search_flag)
	{
		if(tuner_setFreq(tuner_searchParam.wFreq))
		{
			nCnt= tuner_setStationFreq(0xFF, 0xFF, tuner_searchParam.wFreq);
			tuner_searchParam.search_flag= (BOOL)(tuner_searchParam.autosearch_flag&&(nCnt<BANDSTATION_COUNT));
		}
		if(tuner_searchParam.wFreq<tunerMaxFreqs[tuner_Param.byBand])
		{
			tuner_searchParam.wFreq+= tunerStepFreqs[tuner_Param.byBand];
		}else
			tuner_searchParam.search_flag= FALSE;
		if(!tuner_searchParam.search_flag)
		{
			if(tuner_getStationCount(0xFF))
				tuner_setCurrentFreq(0xFF, tuner_getStationFreq(0xFF, 0));
			else
				tuner_setCurrentFreq(0xFF, tunerMinFreqs[tuner_Param.byBand]);
			RX_HARD_MUTE(0x00);
		}
	}
	return bSearch;
}

void tuner_SearchStop()
{
	tuner_searchParam.autosearch_flag= FALSE;
	tuner_searchParam.search_flag= FALSE;
	tuner_setCurrentFreq(0xFF, tuner_searchParam.wFreq);
	RX_HARD_MUTE(0x00);
}
