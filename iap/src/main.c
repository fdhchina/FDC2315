/************************************************************
**实验名称:
**功能:
**注意事项:
**作者:电子白菜
*************************************************************/

#include "hal.h"
#include "my375lib.h"
#if(_HAVE_FONTLIB_==FONT_EXTERN)
#include "25f.h"
#endif
#include "stmflash.h"
#include "lcm19264.h"
#include "usrkey.h"

#define	FIRMWARE_ADR	0x08008000		//要写入数据的地址
#define	FLASH_DATA	0x5a5a5a5a		//要写入的数据

void UpdateFirmware(void);
void iap_load_app(u32 appxaddr);
#if(_HAVE_FONTLIB_==FONT_EXTERN)
u8 USBWriteFlash(void);
u8	FontExists(void);
u8 CheckFlashHz(void);
#endif
void UpdateFont(void);
int main(void)
{
	u8 byKey, byLoop;
#if(_HAVE_FONTLIB_==FONT_EXTERN)
	u8 tmp;
#endif
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);

	ChipHalInit();			//片内硬件初始化
	ChipOutHalInit();		//片外硬件初始化

	lcm_Init();
#if(_HAVE_FONTLIB_==FONT_EXTERN)
	tmp= CheckFlashHz();
#endif
	if(InitUDisk()==0)	// 初始化U盘成功
	{
		delay_ms(500);
		InitUDisk(); 
		if(OpenFile("\\UPLOAD-S\\FDC215.BIN")==0)	// 存在核心文件
		{
			lcm_Putstr(42, 2, "发现程序升级文件！", 18, 0);
			lcm_Putstr(60, 4,"需要升级吗？", 12, 1);
			lcm_BackLightCtrl(TRUE);
			lcm_DisplayOpen(TRUE);
			byLoop= TRUE;
			while(byLoop)
			{
				byKey= kb_GetKey();
				if(byKey)
				switch(byKey)
				{
				case VK_ENTER:
					lcm_Clr();
					lcm_Putstr(42, 0, "   正在升级程序！   ", 0, 0);
					lcm_Putstr(60, 2,"  请稍候…… ", 0, 0);
					lcm_Putstr(24, 6, "升级完毕系统会自动重启！", 0, 0);
					UpdateFirmware();
				case VK_ESC:
					byLoop= FALSE;
					break;
				}
			}
		}
		CloseFile();
#if(_HAVE_FONTLIB_==FONT_EXTERN)
		if(!tmp)			// 字库不存在则安装字库
			USBWriteFlash();
#endif
	}
#if(_HAVE_FONTLIB_==FONT_EXTERN)
	else
		if(!tmp)
		{
			lcm_BackLightCtrl(TRUE);
			lcm_DisplayOpen(TRUE);
			// 字库不存在， 提示需要安装字库
			lcm_DisplayNoFont();
			// while(1);
		}
#endif
	iap_load_app(FIRMWARE_ADR);
	for(;;)
	{
		
	}
}

//跳转到应用程序段
//appxaddr:用户代码起始地址.
typedef  void (*iapfun)(void);				//定义一个函数类型的参数.
//设置栈顶地址
//addr:栈顶地址
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}

void iap_load_app(u32 appxaddr)
{
	iapfun jump2app; 
	
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//检查栈顶地址是否合法.
	{ 
		jump2app=(iapfun)*(vu32*)(appxaddr+4);		//用户代码区第二个字为程序开始地址(复位地址)		
		MSR_MSP(*(vu32*)appxaddr);					//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
		jump2app();									//跳转到APP.
	}
}		 

uint16_t buf[1024];
void UpdateFirmware()
{
	uint16_t *pbuf= buf;
	uint32_t dwWrAddr= FIRMWARE_ADR, dwReadCnt, i;
	
	while((ReadFile((uint8_t *)pbuf, 2048, &dwReadCnt)==0)&&dwReadCnt)
	{
		FLASH_Unlock();
		FLASH_ErasePage(dwWrAddr);
		// STMFLASH_Write(dwWrAddr,pbuf, dwReadCnt / 2);	
		for(i=0;i<1024; i++)
		{
			FLASH_ProgramHalfWord(dwWrAddr, *pbuf++);
			dwWrAddr+= 2;
		}
		FLASH_Lock();
		pbuf= buf;
	}
}

#if(_HAVE_FONTLIB_==FONT_EXTERN)

const char sFontAscii12[]= "\\UPLOAD-S\\ASCII12.BIN";
const char sFontHzk12[]= "\\UPLOAD-S\\HZDOT12.BIN";
const char sFontAscii16[]= "\\UPLOAD-S\\ASCII16.BIN";
const char sFontHzk16[]= "\\UPLOAD-S\\HZDOT16.BIN";
const char sFontHushan[]= "\\UPLOAD-S\\HUSHAN.BIN";

const char * const sFontName[]={
	sFontAscii12,
	sFontHzk12,
	sFontAscii16,
	sFontHzk16,
	sFontHushan};

const u32 dwFontAddress[]={
	sacFont_Ascii12,		// 6*12点阵Ascii字节占用1152字节
	sacFont_Hzk12,		// 12*12点阵GB2312二级全字库占用196272字节
	sacFont_Ascii16,	// 8*16点阵Ascii字节占用1536字节
	sacFont_Hzk16,		// 16*16点阵GB2312二级全字库占用261696字节
	sacHushan			// 湖山商标图案
};	
u8 FlashBuf[1024];//为提高程序效率和编写难度,缓冲应等于一个FLASH扇区.
u8 FlashVerBuf[1024];//校正用~

u8 USBWriteFlash(void)
{
    u32 len;
	u32 addr=0;
	u32 i, index;
	u8 rtn;
	
	for(index=0; index<5; index++)
	{
		// USART1_Puts("现在打开文件...\r\n");
		rtn = OpenFile(sFontName[index]);//sFontAscii12);
		if(rtn!=0) 
		{
			// USART1_Puts("打开英文字库文件ASCII12.BIN失败\r\n");
			continue;
		}
		// USART1_Puts("ASCII12.BIN文件已打开，正在读取文件...\r\n");
		//读取文件并烧写到FLASH上.
		addr=  dwFontAddress[index];//sacFont_Ascii12;
		for(;;) 
		{
			rtn = ReadFile( FlashBuf, sizeof(FlashBuf), &len);
			if (rtn || len == 0) 
				break;   // error or eof
		
			if((addr%4096)==0)		//跨越一个扇区,则需要先刷除
			{
				sst25_SectorErase(addr);	
			}
			sst25_Write(addr,FlashBuf,len);
			sst25_Read(addr,FlashVerBuf,len);
			for(i=0;i<len;i++)
			{
				if(FlashBuf[i]!=FlashVerBuf[i])
				{
					// USART1_Puts("ASCII12.BIN校正错误!!\r\n");
					CloseFile();
					return 0;
				}
			}
			addr+=len;
		} 
		CloseFile();
				 
		// USART1_Puts("ASCII12.BIN文件烧写完毕,开始校正.....\r\n");
		/*OpenFile(sFontName[index]);//sFontAscii12);
		addr= dwFontAddress[index];//sacFont_Ascii12;
		for(;;) 
		{
			rtn = ReadFile( FlashBuf, sizeof(FlashBuf), &len);
			if (rtn || len == 0) 
				break;   // error or eof
			sst25_Read(addr,FlashVerBuf,len);
			addr+=len;
			for(i=0;i<len;i++)
			{
				if(FlashBuf[i]!=FlashVerBuf[i])
				{
					// USART1_Puts("ASCII12.BIN校正错误!!\r\n");
					CloseFile();
					return 0;
				}
			}
		} 
		CloseFile();*/
		// USART1_Puts("ASCII12.BIN校正完成!!\r\n");
	}
/*
	rtn = OpenFile(sFontHzk12);
    if(rtn!=0) 
    {
       // USART1_Puts("打开HZDOT12.BIN文件失败\r\n");
       return 0;
	}
	// USART1_Puts("HZDOT12.BIN文件已打开，正在读取文件...\r\n");
    //读取文件并烧写到FLASH上.
	addr=  sacFont_Hzk12;
   	for(;;) 
	{
		rtn = ReadFile( FlashBuf, sizeof(FlashBuf), &len);
   		if (rtn || len == 0) 
           	break;   // error or eof
	
		if((addr%4096)==0)		//跨越一个扇区,则需要先刷除
		{
			sst25_SectorErase(addr);	
		}
		sst25_Write(addr,FlashBuf,len);
		addr+=len;
	} 
			 
	//USART1_Puts("HZDOT12.BIN文件烧写完毕,开始校正.....\r\n");
	CloseFile();
	OpenFile(sFontHzk12);
	addr= sacFont_Hzk12;
	for(;;) 
	{
		rtn = ReadFile( FlashBuf, sizeof(FlashBuf), &len);
   		if (rtn || len == 0) 
           	break;   // error or eof
		sst25_Read(addr,FlashVerBuf,len);
		addr+=len;
		for(i=0;i<len;i++)
		{
			if(FlashBuf[i]!=FlashVerBuf[i])
			{
				//USART1_Puts("HZDOT12.BIN校正错误!!\r\n");
				CloseFile();
				return 0;
			}
		}
	} 
	CloseFile();
	//USART1_Puts("HZDOT12.BIN校正完成!!\r\n"); */
	// USART1_Puts("完成字库烧写!\r\n");
	return 1;
}

const u8 FlashHZ[16]={0x7E,0x42,0x7E,0xFF,0x89,0xF7,0x78,0x49,0x79,0x01,0xFF,0x00,0x00,0x00,0x00,0x07};
u8 CheckFlashHz(void)
{
	u8 i;
	u8 HzBuf[16];

	sst25_Read(sacFont_Hzk12+0x8430, HzBuf, 16);	// 检查第16区第1个汉字，字模是否正确
	for(i=0;i<16;i++)
	{
		if(HzBuf[i]!=FlashHZ[i])
		{
			i=0xff;
			break;
		}
	}

	//字库没有烧写
	return (i!=0xff);
}

u8	FontExists()
{
	u8 i;

	sst25_Read(sacFont_Ascii12, FlashVerBuf, 12);
	for(i=0;i<12;i++)
		if(FlashVerBuf[i])
			return FALSE;
	return TRUE;
}
#endif


