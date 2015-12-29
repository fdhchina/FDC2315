/************************************************************
**ʵ������:
**����:
**ע������:
**����:���Ӱײ�
*************************************************************/

#include "hal.h"
#include "my375lib.h"
#if(_HAVE_FONTLIB_==FONT_EXTERN)
#include "25f.h"
#endif
#include "stmflash.h"
#include "lcm19264.h"
#include "usrkey.h"

#define	FIRMWARE_ADR	0x08008000		//Ҫд�����ݵĵ�ַ
#define	FLASH_DATA	0x5a5a5a5a		//Ҫд�������

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

	ChipHalInit();			//Ƭ��Ӳ����ʼ��
	ChipOutHalInit();		//Ƭ��Ӳ����ʼ��

	lcm_Init();
#if(_HAVE_FONTLIB_==FONT_EXTERN)
	tmp= CheckFlashHz();
#endif
	if(InitUDisk()==0)	// ��ʼ��U�̳ɹ�
	{
		delay_ms(500);
		InitUDisk(); 
		if(OpenFile("\\UPLOAD-S\\FDC215.BIN")==0)	// ���ں����ļ�
		{
			lcm_Putstr(42, 2, "���ֳ��������ļ���", 18, 0);
			lcm_Putstr(60, 4,"��Ҫ������", 12, 1);
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
					lcm_Putstr(42, 0, "   ������������   ", 0, 0);
					lcm_Putstr(60, 2,"  ���Ժ򡭡� ", 0, 0);
					lcm_Putstr(24, 6, "�������ϵͳ���Զ�������", 0, 0);
					UpdateFirmware();
				case VK_ESC:
					byLoop= FALSE;
					break;
				}
			}
		}
		CloseFile();
#if(_HAVE_FONTLIB_==FONT_EXTERN)
		if(!tmp)			// �ֿⲻ������װ�ֿ�
			USBWriteFlash();
#endif
	}
#if(_HAVE_FONTLIB_==FONT_EXTERN)
	else
		if(!tmp)
		{
			lcm_BackLightCtrl(TRUE);
			lcm_DisplayOpen(TRUE);
			// �ֿⲻ���ڣ� ��ʾ��Ҫ��װ�ֿ�
			lcm_DisplayNoFont();
			// while(1);
		}
#endif
	iap_load_app(FIRMWARE_ADR);
	for(;;)
	{
		
	}
}

//��ת��Ӧ�ó����
//appxaddr:�û�������ʼ��ַ.
typedef  void (*iapfun)(void);				//����һ���������͵Ĳ���.
//����ջ����ַ
//addr:ջ����ַ
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}

void iap_load_app(u32 appxaddr)
{
	iapfun jump2app; 
	
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//���ջ����ַ�Ƿ�Ϸ�.
	{ 
		jump2app=(iapfun)*(vu32*)(appxaddr+4);		//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)		
		MSR_MSP(*(vu32*)appxaddr);					//��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
		jump2app();									//��ת��APP.
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
	sacFont_Ascii12,		// 6*12����Ascii�ֽ�ռ��1152�ֽ�
	sacFont_Hzk12,		// 12*12����GB2312����ȫ�ֿ�ռ��196272�ֽ�
	sacFont_Ascii16,	// 8*16����Ascii�ֽ�ռ��1536�ֽ�
	sacFont_Hzk16,		// 16*16����GB2312����ȫ�ֿ�ռ��261696�ֽ�
	sacHushan			// ��ɽ�̱�ͼ��
};	
u8 FlashBuf[1024];//Ϊ��߳���Ч�ʺͱ�д�Ѷ�,����Ӧ����һ��FLASH����.
u8 FlashVerBuf[1024];//У����~

u8 USBWriteFlash(void)
{
    u32 len;
	u32 addr=0;
	u32 i, index;
	u8 rtn;
	
	for(index=0; index<5; index++)
	{
		// USART1_Puts("���ڴ��ļ�...\r\n");
		rtn = OpenFile(sFontName[index]);//sFontAscii12);
		if(rtn!=0) 
		{
			// USART1_Puts("��Ӣ���ֿ��ļ�ASCII12.BINʧ��\r\n");
			continue;
		}
		// USART1_Puts("ASCII12.BIN�ļ��Ѵ򿪣����ڶ�ȡ�ļ�...\r\n");
		//��ȡ�ļ�����д��FLASH��.
		addr=  dwFontAddress[index];//sacFont_Ascii12;
		for(;;) 
		{
			rtn = ReadFile( FlashBuf, sizeof(FlashBuf), &len);
			if (rtn || len == 0) 
				break;   // error or eof
		
			if((addr%4096)==0)		//��Խһ������,����Ҫ��ˢ��
			{
				sst25_SectorErase(addr);	
			}
			sst25_Write(addr,FlashBuf,len);
			sst25_Read(addr,FlashVerBuf,len);
			for(i=0;i<len;i++)
			{
				if(FlashBuf[i]!=FlashVerBuf[i])
				{
					// USART1_Puts("ASCII12.BINУ������!!\r\n");
					CloseFile();
					return 0;
				}
			}
			addr+=len;
		} 
		CloseFile();
				 
		// USART1_Puts("ASCII12.BIN�ļ���д���,��ʼУ��.....\r\n");
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
					// USART1_Puts("ASCII12.BINУ������!!\r\n");
					CloseFile();
					return 0;
				}
			}
		} 
		CloseFile();*/
		// USART1_Puts("ASCII12.BINУ�����!!\r\n");
	}
/*
	rtn = OpenFile(sFontHzk12);
    if(rtn!=0) 
    {
       // USART1_Puts("��HZDOT12.BIN�ļ�ʧ��\r\n");
       return 0;
	}
	// USART1_Puts("HZDOT12.BIN�ļ��Ѵ򿪣����ڶ�ȡ�ļ�...\r\n");
    //��ȡ�ļ�����д��FLASH��.
	addr=  sacFont_Hzk12;
   	for(;;) 
	{
		rtn = ReadFile( FlashBuf, sizeof(FlashBuf), &len);
   		if (rtn || len == 0) 
           	break;   // error or eof
	
		if((addr%4096)==0)		//��Խһ������,����Ҫ��ˢ��
		{
			sst25_SectorErase(addr);	
		}
		sst25_Write(addr,FlashBuf,len);
		addr+=len;
	} 
			 
	//USART1_Puts("HZDOT12.BIN�ļ���д���,��ʼУ��.....\r\n");
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
				//USART1_Puts("HZDOT12.BINУ������!!\r\n");
				CloseFile();
				return 0;
			}
		}
	} 
	CloseFile();
	//USART1_Puts("HZDOT12.BINУ�����!!\r\n"); */
	// USART1_Puts("����ֿ���д!\r\n");
	return 1;
}

const u8 FlashHZ[16]={0x7E,0x42,0x7E,0xFF,0x89,0xF7,0x78,0x49,0x79,0x01,0xFF,0x00,0x00,0x00,0x00,0x07};
u8 CheckFlashHz(void)
{
	u8 i;
	u8 HzBuf[16];

	sst25_Read(sacFont_Hzk12+0x8430, HzBuf, 16);	// ����16����1�����֣���ģ�Ƿ���ȷ
	for(i=0;i<16;i++)
	{
		if(HzBuf[i]!=FlashHZ[i])
		{
			i=0xff;
			break;
		}
	}

	//�ֿ�û����д
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


