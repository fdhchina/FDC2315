#include "hal.h"
#if(_HAVE_FONTLIB_==FONT_EXTERN)
#include "25f.h"

#define SST_SELECT()			GPIO_ResetBits(SST25_CE_Port, SST25_CE_Pin)       /* SST CS = L */ 
#define SST_DESELECT()		GPIO_SetBits(SST25_CE_Port, SST25_CE_Pin)         /* SST CS = H */

#if (SST25_SPI==1)

#define flash_spi		SPI1

#elif (SST25_SPI==2)

#define flash_spi		SPI2

#elif (SST25_SPI==3)

#define flash_spi		SPI3

#endif
/***********************************************
**������:sst25_Init
**����:��ʼ������FLASH��SPI�ӿ�
**ע������:����FLASHʹ����flash_spi�ӿ�
***********************************************/
static u8 SPIByte(u8 byte);
void SST25_SPI_Config()
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
#if (SST25_SPI==1)

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC  |
            RCC_APB2Periph_AFIO |
            RCC_APB2Periph_SPI1,
            ENABLE);

	/*A5=CLK,A6=MISO,A7=MOSI*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5  | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	

#elif (SST25_SPI==2)

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	// SCK, MISO and MOSI  B13=CLK,B14=MISO,B15=MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);


#elif (SST25_SPI==3)

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	// SCK, MISO and MOSI  B3=CLK,B4=MISO,B5=MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

#endif 
	// SPI configuration  ע��25ϵ�е��ز���
	SPI_Cmd(flash_spi, DISABLE); 												//�����Ƚ���,���ܸı�MODE
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;		//����ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;							//��
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;						//8λ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;								//CPOL=0 ʱ�����յ�
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;							//CPHA=0 ���ݲ����1��
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;								//���NSS
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;		// 2��Ƶ
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;						// ��λ��ǰ
	SPI_InitStructure.SPI_CRCPolynomial = 7;								// CRC7
	SST_DESELECT();
	SPI_Init(flash_spi, &SPI_InitStructure);
	// SPI_SSOutputCmd(flash_spi, ENABLE); //ʹ��NSS�ſ���
	SPI_Cmd(flash_spi, ENABLE); 

}

void sst25_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	SST25_SPI_Config();
	//  Ƭѡ����
	SST_DESELECT();	//	Ԥ��Ϊ��
	GPIO_InitStructure.GPIO_Pin = SST25_CE_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SST25_CE_Port, &GPIO_InitStructure);

	SPIByte(0xff);	//��λ����
}

/***************************************
**������:SPIByte
**����:��дSPI����
**ע������:����SPI��˵�������Ķ�Ҳ��Ҫ��д��
**ʹ�ô˺���������ʱ�����������Ϊ0xff��д��ʱ����д����.����ʹ��ֱ�Ӳ����Ĵ����İ취ʵ��SPIӲ�����д,��Ϊ�˼ӿ���д�ٶ�
***************************************/
static u8 SPIByte(u8 byte)
{
	/*�ȴ����ͼĴ�����*/
	while((flash_spi->SR & SPI_I2S_FLAG_TXE)==RESET);
    /*����һ���ֽ�*/
	flash_spi->DR = byte;
	/* �ȴ����ռĴ�����Ч*/
	while((flash_spi->SR & SPI_I2S_FLAG_RXNE)==RESET);
	return(flash_spi->DR);
}


/*****************************************
**������:SSTCmd1/2/4
**����:дһ��SST����/дһ��������һ������/дһ���������д3������
**ע������:����һ�������ĵ����������������
*****************************************/
void SSTCmd1(u8 cmd)
{
	SST_SELECT();
	SPIByte(cmd);
	SST_DESELECT();
}

void SSTCmd2(u8 cmd,u8 data)
{
	SST_SELECT();
	SPIByte(cmd);
	SPIByte(data);
	SST_DESELECT();
}

void SSTCmd4(u8 cmd,u8 *addr)
{
	SST_SELECT();
	SPIByte(cmd);	//������
	SPIByte(*addr++);
	SPIByte(*addr++);
	SPIByte(*addr);
	SST_DESELECT();
}
/****************************************
**������:SSTCmdb1b/SSTCmd4bs
**����:дһ��SST�������1�ֽ�����/д1�������֣�3����ַ�֣����ض���ֽ�
**ע������:
****************************************/
u8 SSTCmdb1b(u8 cmd)
{
	u8 tmp;
	SST_SELECT();
	SPIByte(cmd);
	tmp=SPIByte(0xff);
	SST_DESELECT();
	return(tmp);
}
void SSTCmd4bs(u8 cmd,u8* addr,u8* data,u32 no)
{
	SST_SELECT();
	SPIByte(cmd);	//������
	SPIByte(*addr++);
	SPIByte(*addr++);
	SPIByte(*addr);
	for(;no>0;no--)
	{
		*data++=SPIByte(0xff);
	}
	SST_DESELECT();
}


/***************************************
  SST25WREN  ����д����
***************************************/
void SST25WREN(void)
{
	SSTCmd1(0x06);
}

/***********************************
  SST25WRDI  ����д����
***********************************/
void SST25WRDI(void)
{
	SSTCmd1(0x04);
}

/**********************************
  SST25BY  ���æ
**********************************/
u8 SST25BY(void)
{
	u8 sta;
	sta=SSTCmdb1b(0x05);
	return(sta&0x01);
}

/***********************************
  SST25WPEN �������д����
  ע������:25��д��ȽϷ�����������ÿ�β���ǰ��ȡ����д������������ɺ�����������д����
***********************************/
void SST25WPEN(void)
{
	u8 sta;
	sta=SSTCmdb1b(0x05)|0x1c;		//�����Ĵ��������뱣��λ
	SSTCmd1(0x50);					//����д
	SSTCmd2(0x01,sta);
}

//����������λ��������дλ
void SST25WriteEn(void)
{
	u8 sta;
	sta=SSTCmdb1b(0x05)&(~0x1c);	//�����Ĵ�������������λ
	SSTCmd1(0x50);					//����д�Ĵ���
	SSTCmd2(0x01,sta);				//д�Ĵ���
	SSTCmd1(0x06);					//����д
}

/**********************************
  SST25ReadID ��ȡSST��ID
**********************************/
u16 SST25ReadID(void)
{
	u8 id[3];
	u8 addr[3]={0,0,0};
	
	SSTCmd4bs(0x90,addr,id,3);
	return((id[0]<<8)+id[1]);
}

/**********************************
  SST25ChipErase ˢ��CHIP
**********************************/
void SST25ChipErase(void)
{
	SST25WriteEn();
	SSTCmd1(0x60);
	while(SST25BY());
	SST25WPEN();
}

/***********************************
  SST25SectorErase  ˢ����
***********************************/
void sst25_SectorErase(u32 addr)
{
	u8 ad[3];
	ad[0]=(addr>>16)&0xff;
	ad[1]=(addr>>8)&0xff;
	ad[2]=addr&0xff;
	
	
	SST25WriteEn();
	
	SST_SELECT();
	SPIByte(0x20);
	SPIByte(ad[0]);
	SPIByte(ad[1]);
	SPIByte(ad[2]);
	SST_DESELECT();
	
	while(SST25BY());
//	SST25WPEN();
}
/**********************************
  SST25ByteProgram  дһ���ֽ�*ע���ڴ�ǰҪ����ȡ��д����,ʵ��дӦʹ��AAI,�˺�����AAI�е��ã�����д�������ֽ�
**********************************/
static void SST25ByteProgram(u32 addr,u8 byte)
{
	u8 ad[3];
	ad[0]=(addr>>16)&0xff;
	ad[1]=(addr>>8)&0xff;
	ad[2]=addr&0xff;

	SST_SELECT();
	SPIByte(0x02);
	SPIByte(ad[0]);
	SPIByte(ad[1]);
	SPIByte(ad[2]);
	SPIByte(byte);
	SST_DESELECT();
	while(SST25BY());
}
/***********************************
  SST25Write д����ֽ�
***********************************/
void sst25_Write(u32 addr,u8* p_data,u32 no)
{
	u8 ad[3];
	u32 cnt;
	if(no==0)
		return;
		
	SST25WriteEn();
	
	if(no==1)	//no<2��Ӧʹ����ͨ���ֽڷ�ʽ
	{
		SST25ByteProgram(addr,*p_data);
	//	SST25WPEN();
	}	
	else
	{
		cnt=no;
		
		ad[2]=(addr>>16)&0xff;
		ad[1]=(addr>>8)&0xff;
		ad[0]=addr&0xff;
		
		SST_SELECT();
		SPIByte(0xad);
		SPIByte(ad[2]);
		SPIByte(ad[1]);
		SPIByte(ad[0]);
		SPIByte(*p_data++);
		SPIByte(*p_data++);
		SST_DESELECT();
		cnt-=2;
		while(SST25BY());//��æ
		
		//�м��˫�ֽ�д
		for(;cnt>1;cnt-=2)
		{
			SST_SELECT();
			SPIByte(0xad);
			SPIByte(*p_data++);
			SPIByte(*p_data++);
			SST_DESELECT();
			while(SST25BY());//��æ
		}
		SST25WRDI();//WRDI�����˳�AAIдģʽ
		
		//��������һ���ֽ�(noΪ������
		if(cnt==1)
		{
			SST25WriteEn();
			SST25ByteProgram(addr+no-1,*p_data);
		}
	}
	while(SST25BY());
	SST25WPEN();//WP����
}

/*************************************
  SST25Read ���ٶ� ���ں�����5��оƬ���ɵ��ô˺�����
*************************************/
void sst25_Read(u32 addr,u8* p_data,u32 no)
{
	SST_SELECT();

	SPIByte(0x0b);
	SPIByte(addr>>16);
	SPIByte(addr>>8);
	SPIByte(addr);
	SPIByte(0xff);

	for(;no>0;no--)
	*p_data++=SPIByte(0xff);
	SST_DESELECT();
}

/****************************************
SST25ReadL  ���ٶ�
****************************************/
void sst25_ReadL(u32 addr,u8* p_data,u32 no)
{
	u8 ad[3];
	ad[2]=(addr>>16)&0xff;
	ad[1]=(addr>>8)&0xff;
	ad[0]=addr&0xff;
	
	SSTCmd4bs(0x03,ad,p_data,no);
}


#endif
