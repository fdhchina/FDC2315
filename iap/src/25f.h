#ifndef FLASH_25F_H
#define FLASH_25F_H

// ========================================================================================
// 硬件端口
#define SST25_CE_Port		GPIOE
#define SST25_CE_Pin		GPIO_Pin_15
#define SST25_SPI			1

/*
typedef enum _sst25fAddressConfig
{
	sacFont_Ascii12= 0x0000,	// 6*12点阵Ascii字节占用1152字节
	sacFont_Hzk12= 0x1000,		// 12*12点阵GB2312二级全字库占用196272字节
	sacFont_Ascii16= 0x31000,	// 8*16点阵Ascii字节占用1536字节
	sacFont_Hzk16= 0x32000,		// 16*16点阵GB2312二级全字库占用261696字节
	sacHushan= 0x72000,			// 湖山商标图案
	sacRevser= 0x73000,			// 此段未用
	sacSysConfiger= 0xf8000		// 用最后的32K字节来保存系统配置
}ConfigerAddress;
*/
// ========================================================================================
void sst25_Init(void);
void sst25_Read(u32 addr,u8* p_data,u32 no);
void sst25_ReadL(u32 addr,u8* p_data,u32 no);
void sst25_Write(u32 addr,u8* p_data,u32 no);
void sst25_SectorErase(u32 addr);

#endif
