#ifndef HAL_H
#define HAL_H

#include "stm32f10x.h"

// 字库存储位置
#define FONT_EXTERN		0
#define FONT_CODE		1
#define FONT_FLASH		2
#define _HAVE_FONTLIB_		FONT_FLASH

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
	//sacFont_Ascii16= 0x31000,	// 8*16点阵Ascii字节占用1536字节
	//sacFont_Hzk16= 0x32000,		// 16*16点阵GB2312二级全字库占用261696字节
	//sacHushan= 0x72000,			// 湖山商标图案
	//sacTaskBakup= 0x73000,			// 此段共512K用于保存任务数据备份
#endif
}data_address_t;

//硬件初始化
extern void  ChipHalInit(void);
extern void  ChipOutHalInit(void);
extern void SysTickDelay(u16 dly_ms);
void delay_ms(u16 nms);
void delay_us(u32 nus);

// ========================================================================================
void TurnToSST25(void);
void FLASH_SPI_Config(void);
void SST25Read(u32 addr,u8* p_data,u32 no);
void SST25ReadL(u32 addr,u8* p_data,u32 no);
void SST25Write(u32 addr,u8* p_data,u32 no);
void SST25SectorErase(u32 addr);
// ========================================================================================
#endif
