#ifndef HAL_H
#define HAL_H

#include "stm32f10x.h"

// �ֿ�洢λ��
#define FONT_EXTERN		0
#define FONT_CODE		1
#define FONT_FLASH		2
#define _HAVE_FONTLIB_		FONT_FLASH

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
	//sacFont_Ascii16= 0x31000,	// 8*16����Ascii�ֽ�ռ��1536�ֽ�
	//sacFont_Hzk16= 0x32000,		// 16*16����GB2312����ȫ�ֿ�ռ��261696�ֽ�
	//sacHushan= 0x72000,			// ��ɽ�̱�ͼ��
	//sacTaskBakup= 0x73000,			// �˶ι�512K���ڱ����������ݱ���
#endif
}data_address_t;

//Ӳ����ʼ��
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
