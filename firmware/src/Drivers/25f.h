#ifndef FLASH_25F_H
#define FLASH_25F_H

// ========================================================================================
// Ó²¼þ¶Ë¿Ú
#define SST25_CE_Port		GPIOE
#define SST25_CE_Pin		GPIO_Pin_15
#define SST25_SPI			1


// ========================================================================================
void sst25_Init(void);
void sst25_Read(u32 addr,u8* p_data,u32 no);
void sst25_ReadL(u32 addr,u8* p_data,u32 no);
void sst25_Write(u32 addr,u8* p_data,u32 no);
void sst25_SectorErase(u32 addr);

#endif
