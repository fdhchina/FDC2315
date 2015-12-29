#ifndef __VS1003_H__
#define __VS1003_H__


#define VS_WRITE_COMMAND 	0x02
#define VS_READ_COMMAND 	0x03
//VS1003寄存器定义
#define SPI_MODE        	0x00   
#define SPI_STATUS      	0x01   
#define SPI_BASS        	0x02   
#define SPI_CLOCKF      	0x03   
#define SPI_DECODE_TIME 	0x04   
#define SPI_AUDATA      	0x05   
#define SPI_WRAM        	0x06   
#define SPI_WRAMADDR    	0x07   
#define SPI_HDAT0       	0x08   
#define SPI_HDAT1       	0x09 
  
#define SPI_AIADDR      	0x0a   
#define SPI_VOL         	0x0b   
#define SPI_AICTRL0     	0x0c   
#define SPI_AICTRL1     	0x0d   
#define SPI_AICTRL2     	0x0e   
#define SPI_AICTRL3     	0x0f   
#define SM_DIFF         	0x01   
#define SM_JUMP         	0x02   
#define SM_RESET        	0x04   
#define SM_OUTOFWAV     	0x08   
#define SM_PDOWN        	0x10   
#define SM_TESTS        	0x20   
#define SM_STREAM       	0x40   
#define SM_PLUSV        	0x80   
#define SM_DACT         	0x100   
#define SM_SDIORD       	0x200   
#define SM_SDISHARE     	0x400   
#define SM_SDINEW       	0x800   
#define SM_ADPCM        	0x1000   
#define SM_ADPCM_HP     	0x2000 		 
//	MP3_DCS	  PB1  MP3_RST   PB2   MP3_DREO  PE7  MP3_CS    PB12 
#if (GC_ZR&GC_ZR_GPIO)
#define MP3_CCS_SET(x)  (x ?(GPIOB->BSRR=GPIO_Pin_12):(GPIOB->BRR=GPIO_Pin_12))		 // 命令片选
#define MP3_RST_SET(x)  (x ?(GPIOB->BSRR=GPIO_Pin_2):(GPIOB->BRR=GPIO_Pin_2))		// 复位
#define MP3_DCS_SET(x)  (x ?(GPIOB->BSRR=GPIO_Pin_1):(GPIOB->BRR=GPIO_Pin_1))     //数据片选 
#define VS1003_DREQ		(GPIOE->IDR&GPIO_Pin_7)
#else
#define MP3_CCS_SET(x)  (x ?(GPIOB->BSRR=GPIO_Pin_12):(GPIOB->BRR=GPIO_Pin_12))		 // 命令片选
#define MP3_RST_SET(x)  (x ?(GPIOB->BSRR=GPIO_Pin_11):(GPIOB->BRR=GPIO_Pin_11))		// 复位
#define MP3_DCS_SET(x)  (x ?(GPIOB->BSRR=GPIO_Pin_9):(GPIOB->BRR=GPIO_Pin_9))     //数据片选 
#define VS1003_DREQ		(GPIOB->IDR&GPIO_Pin_10)
#endif
#if VS1003_INT_FLAG_EN
void vs1003_IntEnable(FunctionalState bEnable);
#endif

u16  vs1003_REGRead(u8 address);//读寄存器
void vs1003_DATAWrite(u8 data);//写数据
void vs1003_32byteWrite(u8 *data);
void vs1003_WriteData(unsigned char *buf,int len);
void vs1003_CMDWrite(u8 address,u16 data);//写命令
void vs1003_Init(void);			//初始化VS1003
u16 vs1003_GetVolume(void);
void vs1003_SetVolume(u16 wVol);
u16 vs1003_GetBass(void);
void vs1003_SetBass(u16 wBass);
void vs1003_ClearDataBuf(void);
//void Vs1003_DATA_Write(u8 data);//向vs1003写数据
void vs1003_Reset(void);			//硬复位
void vs1003_SoftReset(void);     //软复位
void vs1003_RamTest(void);           //RAM测试
void vs1003_SineTest(void);          //正弦测试
u16 vs1003_GetDecodeTime(void);   //得到解码时间
u16 vs1003_GetHeadInfo(void);     //得到比特率
void vs1003_ResetDecodeTime(void);//重设解码时间
// void LoadPatch(void);      //加载频谱分析代码
// void GetSpec(u8 *p);       //得到分析数据
void vs1003_RecordInit(u32 isline, u32 SampleFrequency);
#endif

















