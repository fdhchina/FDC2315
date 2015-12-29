#ifndef FLASH_25F_H
#define FLASH_25F_H

// ========================================================================================
// Ӳ���˿�
#define SST25_CE_Port		GPIOE
#define SST25_CE_Pin		GPIO_Pin_15
#define SST25_SPI			1

/*
typedef enum _sst25fAddressConfig
{
	sacFont_Ascii12= 0x0000,	// 6*12����Ascii�ֽ�ռ��1152�ֽ�
	sacFont_Hzk12= 0x1000,		// 12*12����GB2312����ȫ�ֿ�ռ��196272�ֽ�
	sacFont_Ascii16= 0x31000,	// 8*16����Ascii�ֽ�ռ��1536�ֽ�
	sacFont_Hzk16= 0x32000,		// 16*16����GB2312����ȫ�ֿ�ռ��261696�ֽ�
	sacHushan= 0x72000,			// ��ɽ�̱�ͼ��
	sacRevser= 0x73000,			// �˶�δ��
	sacSysConfiger= 0xf8000		// ������32K�ֽ�������ϵͳ����
}ConfigerAddress;
*/
// ========================================================================================
void sst25_Init(void);
void sst25_Read(u32 addr,u8* p_data,u32 no);
void sst25_ReadL(u32 addr,u8* p_data,u32 no);
void sst25_Write(u32 addr,u8* p_data,u32 no);
void sst25_SectorErase(u32 addr);

#endif
