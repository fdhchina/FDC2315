#ifndef HAL_H
#define HAL_H

// #define STM32F10X_HD
// #define USE_STDPERIPH_DRIVER

#include "stm32f10x.h"
#define	_DEBUG			0

// �ֿ�洢λ��
#define FONT_EXTERN	0
#define FONT_CODE		1
#define FONT_FLASH		2
#define _HAVE_FONTLIB_		FONT_FLASH

#define CH375_INT_MODE		0
#define VS1003_INT_FLAG_EN		0								// �Ƿ�ʹ���жϷ�ʽ
#define PRODUCT_VER		"V1.07"  
/*
�汾˵��
V1.07 2015-12-21
	1.����ʱ��оƬDS3231��ȡ˳�򣬽��0��0��0 ��������m_iNextTask�������� 
		DS3231.c line 131
	2.������־���Թ���log.c
V1.06 2015-12-16
	1.������������ 4989  
		menu.c line 2226
V1.05 2015-10-15
	1.�����������ʱ��Ĭ��1����
V1.04 2015-10-14
	1.�Ż��绰���ճ���ʵ����TPC210�ĸ����ں�
V1.03 2015-10-09
	1.���Ӷ������Ź����ܣ�ι��ʱ�� 40K/256 = 6.2ms * 4000 = 24.8s main.c
	2.�ֶ�����ʱ��ң�ز��ܿ���menc.c 972��
		if ((g_pCurMenuItem->byMenuID == MENUID_DESKTOP) && ((YK_Key <12) || (DH_Key <20000000)))
	3.����play.c 444�� j<8 ��Ϊj<14 128���ص�Mp3ÿ֡Ϊ417���ֽ�
	4.�Ż�ң������
V1.02 2014-10-02
	1. ���� ��δ����U�̣� �ڲ���U����ֱ�Ӱ����ţ���ʱ���ŵ����ڴ����Ŀ��������
V1.01 2014-06-23
	1. ���۲�Ҫ������U�̲��ż��书�ܡ�
V1.00 2014-03-25
	������ɰ汾��
*/
// ==================================================================================

#define GC_ZR_ALL		0xFFFF
#define GC_ZR_NONE		0x0000
#define GC_ZR_FC		0x0001//�޸ķ���
#define GC_ZR_MANAGE	0x0002//�޸��������˵�
#define GC_ZR_REMOTE	0x0004//�޸ĳ�������
#define GC_ZR_TUNER		0x0008//�޸�������
#define GC_ZR_PLAY		0x0010//�޸Ĳ��Ž���,����Ӧ����Զ�̲��Ž���
#define GC_ZR_BJ		0x0020//�������ý���
#define GC_ZR_GPIO  0x0080 //����GPIO
//key_up Ӧ��

#define GC_ZR	GC_ZR_ALL
// ��������

typedef enum _data_address_tag
{
#if(_HAVE_FONTLIB_==FONT_EXTERN)	// ������sst25VF��
// һ������4K�ֽ�
	sacFont_Ascii12= 0x0000,		// 6*12����Ascii�ֽ�ռ��1152�ֽ�
	sacFont_Hzk12= 0x1000,		// 12*12����GB2312����ȫ�ֿ�ռ��196272�ֽ�
	sacFont_Ascii16= 0x31000,	// 8*16����Ascii�ֽ�ռ��1536�ֽ�
	sacFont_Hzk16= 0x32000,		// 16*16����GB2312����ȫ�ֿ�ռ��261696�ֽ�
	sacHushan= 0x72000,			// ��ɽ�̱�ͼ��
	sacTaskBakup= 0x73000,			// �˶ι�512K���ڱ����������ݱ���
	sacTunerParam= 0xf8000,		// 4K������������
	sacSysConfiger= 0xfA000		// ������28K�ֽ�������ϵͳ����

#else	// ������CPU Flash��

	sacFont_Ascii12= 0x08048000,		// 6*12����Ascii�ֽ�ռ��1152�ֽ�
	sacTunerParam= 0x08049000,		// 2K������������
	sacSysConfiger= 0x08049800,		// ������2K�ֽ�������ϵͳ����
	sacFont_Hzk12= 0x08050000,		// 12*12����GB2312����ȫ�ֿ�ռ��196272�ֽ� 
	//sacSysConfiger = 0x08068000,
	//sacTunerParam = 0x08070000,
	//sacFont_Ascii16= 0x31000,	// 8*16����Ascii�ֽ�ռ��1536�ֽ�
	//sacFont_Hzk16= 0x32000,		// 16*16����GB2312����ȫ�ֿ�ռ��261696�ֽ�
	//sacHushan= 0x72000,			// ��ɽ�̱�ͼ��
	//sacTaskBakup= 0x73000,			// �˶ι�512K���ڱ����������ݱ���
#endif
}data_address_t;

typedef union {
	u32		l;
	u16		w[2];
	u8		b[4];
}TSize;
typedef uint8_t		BYTE;
typedef uint16_t	WORD;
typedef bool		BOOL;

#ifndef NULL
#define NULL ((void *)0)
#endif


// ==================================================================================
// ���ͷ�ļ�
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "Com.h"
/* Definitions of physical drive number for each media */
#define MMC		0
#define USB		1

#include "ff.h"
#include "task.h"
#include "usrkey.h"
#include "25f.h"

// ==================================================================================
// ��س���
#define	TUNERBAND_COUNT		2
#define 	FM_FREQ_MIN			8700	// 10KHz
#define	FM_FREQ_MAX			10800	// 10KHz
#define	AM_FREQ_MIN			522		// KHz
#define	AM_FREQ_MAX			1620	// KHz
#define	FM_FREQ_STEP			10		// 10KHz
#define	AM_FREQ_STEP			9		// KHz

#define	BANDSTATION_COUNT	40		// ÿ�����ε�̨����


// ==================================================================================

// ==================================================================================
// �������
extern GPIO_InitTypeDef GPIO_InitStructure;
extern SPI_InitTypeDef  SPI_InitStructure;

#if VS1003_INT_FLAG_EN | CH375_INT_MODE

extern EXTI_InitTypeDef EXTI_InitStructure;
extern NVIC_InitTypeDef NVIC_InitStructure;

#endif

extern FATFS	fs_sd;
extern FATFS	fs_usb;
extern u8 volatile Ms_1, Ms_8, Ms_128, Ms_256,S_1;
extern u16 volatile Timer1,Timer2;
extern TMyDate	g_SysDate;
extern TMyTime g_SysTime;
extern u8		g_SysWeek;
extern volatile u32 dwTickCount;
// ==================================================================================

//Ӳ����ʼ��
extern void  ChipHalInit(void);
extern void  ChipOutHalInit(void);
extern void SysTickDelay(u16 dly_ms);
extern void strfmt(char * str, const char * fmt, ...);
extern BOOL GetSysTime(void);

#define delay_ms			SysTickDelay
#define DelayMS			SysTickDelay
void delay_us(u32 nus);
// ========================================================================================

#include "my375lib.h"

#endif
