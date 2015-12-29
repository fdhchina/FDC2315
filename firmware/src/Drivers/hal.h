#ifndef HAL_H
#define HAL_H

// #define STM32F10X_HD
// #define USE_STDPERIPH_DRIVER

#include "stm32f10x.h"
#define	_DEBUG			0

// 字库存储位置
#define FONT_EXTERN	0
#define FONT_CODE		1
#define FONT_FLASH		2
#define _HAVE_FONTLIB_		FONT_FLASH

#define CH375_INT_MODE		0
#define VS1003_INT_FLAG_EN		0								// 是否使用中断方式
#define PRODUCT_VER		"V1.07"  
/*
版本说明
V1.07 2015-12-21
	1.更改时钟芯片DS3231读取顺序，解决0：0：0 更新任务m_iNextTask错误问题 
		DS3231.c line 131
	2.增加日志调试功能log.c
V1.06 2015-12-16
	1.增加万能密码 4989  
		menu.c line 2226
V1.05 2015-10-15
	1.更改任务结束时间默认1分钟
V1.04 2015-10-14
	1.优化电话接收程序，实现与TPC210的更好融合
V1.03 2015-10-09
	1.增加独立看门狗功能，喂狗时间 40K/256 = 6.2ms * 4000 = 24.8s main.c
	2.手动播音时，遥控不能控制menc.c 972行
		if ((g_pCurMenuItem->byMenuID == MENUID_DESKTOP) && ((YK_Key <12) || (DH_Key <20000000)))
	3.更改play.c 444行 j<8 改为j<14 128比特的Mp3每帧为417个字节
	4.优化遥控任务
V1.02 2014-10-02
	1. 修正 “未插入U盘， 在播放U盘中直接按播放，此时播放的是内存的曲目”的问题
V1.01 2014-06-23
	1. 销售部要求增加U盘播放记忆功能。
V1.00 2014-03-25
	测试完成版本。
*/
// ==================================================================================

#define GC_ZR_ALL		0xFFFF
#define GC_ZR_NONE		0x0000
#define GC_ZR_FC		0x0001//修改分区
#define GC_ZR_MANAGE	0x0002//修改任务管理菜单
#define GC_ZR_REMOTE	0x0004//修改出发任务
#define GC_ZR_TUNER		0x0008//修改搜音机
#define GC_ZR_PLAY		0x0010//修改播放界面,增加应急和远程播放界面
#define GC_ZR_BJ		0x0020//报警设置界面
#define GC_ZR_GPIO  0x0080 //更改GPIO
//key_up 应急

#define GC_ZR	GC_ZR_ALL
// 数据类型

typedef enum _data_address_tag
{
#if(_HAVE_FONTLIB_==FONT_EXTERN)	// 保存在sst25VF中
// 一个扇区4K字节
	sacFont_Ascii12= 0x0000,		// 6*12点阵Ascii字节占用1152字节
	sacFont_Hzk12= 0x1000,		// 12*12点阵GB2312二级全字库占用196272字节
	sacFont_Ascii16= 0x31000,	// 8*16点阵Ascii字节占用1536字节
	sacFont_Hzk16= 0x32000,		// 16*16点阵GB2312二级全字库占用261696字节
	sacHushan= 0x72000,			// 湖山商标图案
	sacTaskBakup= 0x73000,			// 此段共512K用于保存任务数据备份
	sacTunerParam= 0xf8000,		// 4K保存收音配置
	sacSysConfiger= 0xfA000		// 用最后的28K字节来保存系统配置

#else	// 保存在CPU Flash中

	sacFont_Ascii12= 0x08048000,		// 6*12点阵Ascii字节占用1152字节
	sacTunerParam= 0x08049000,		// 2K保存收音配置
	sacSysConfiger= 0x08049800,		// 用最后的2K字节来保存系统配置
	sacFont_Hzk12= 0x08050000,		// 12*12点阵GB2312二级全字库占用196272字节 
	//sacSysConfiger = 0x08068000,
	//sacTunerParam = 0x08070000,
	//sacFont_Ascii16= 0x31000,	// 8*16点阵Ascii字节占用1536字节
	//sacFont_Hzk16= 0x32000,		// 16*16点阵GB2312二级全字库占用261696字节
	//sacHushan= 0x72000,			// 湖山商标图案
	//sacTaskBakup= 0x73000,			// 此段共512K用于保存任务数据备份
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
// 相关头文件
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
// 相关常量
#define	TUNERBAND_COUNT		2
#define 	FM_FREQ_MIN			8700	// 10KHz
#define	FM_FREQ_MAX			10800	// 10KHz
#define	AM_FREQ_MIN			522		// KHz
#define	AM_FREQ_MAX			1620	// KHz
#define	FM_FREQ_STEP			10		// 10KHz
#define	AM_FREQ_STEP			9		// KHz

#define	BANDSTATION_COUNT	40		// 每个波段电台总数


// ==================================================================================

// ==================================================================================
// 定义变量
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

//硬件初始化
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
