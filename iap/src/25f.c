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
**函数名:sst25_Init
**功能:初始化串行FLASH的SPI接口
**注意事项:串行FLASH使用了flash_spi接口
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
	// SPI configuration  注意25系列的沿操作
	SPI_Cmd(flash_spi, DISABLE); 												//必须先禁能,才能改变MODE
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;		//两线全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;							//主
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;						//8位
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;								//CPOL=0 时钟悬空低
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;							//CPHA=0 数据捕获第1个
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;								//软件NSS
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;		// 2分频
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;						// 高位在前
	SPI_InitStructure.SPI_CRCPolynomial = 7;								// CRC7
	SST_DESELECT();
	SPI_Init(flash_spi, &SPI_InitStructure);
	// SPI_SSOutputCmd(flash_spi, ENABLE); //使能NSS脚可用
	SPI_Cmd(flash_spi, ENABLE); 

}

void sst25_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	SST25_SPI_Config();
	//  片选配置
	SST_DESELECT();	//	预置为高
	GPIO_InitStructure.GPIO_Pin = SST25_CE_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SST25_CE_Port, &GPIO_InitStructure);

	SPIByte(0xff);	//复位总线
}

/***************************************
**函数名:SPIByte
**功能:读写SPI总线
**注意事项:对于SPI来说，主机的读也需要先写，
**使用此函数，读的时候建议参数设置为0xff，写的时候则写参数.这里使用直接操作寄存器的办法实现SPI硬件层读写,是为了加快速写速度
***************************************/
static u8 SPIByte(u8 byte)
{
	/*等待发送寄存器空*/
	while((flash_spi->SR & SPI_I2S_FLAG_TXE)==RESET);
    /*发送一个字节*/
	flash_spi->DR = byte;
	/* 等待接收寄存器有效*/
	while((flash_spi->SR & SPI_I2S_FLAG_RXNE)==RESET);
	return(flash_spi->DR);
}


/*****************************************
**函数名:SSTCmd1/2/4
**功能:写一个SST命令/写一个命令后接一个数据/写一个命令后再写3个数据
**注意事项:这是一个完整的单命令操作，不返回
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
	SPIByte(cmd);	//首命令
	SPIByte(*addr++);
	SPIByte(*addr++);
	SPIByte(*addr);
	SST_DESELECT();
}
/****************************************
**函数名:SSTCmdb1b/SSTCmd4bs
**功能:写一个SST命令，返回1字节数据/写1个命令字，3个地址字，返回多个字节
**注意事项:
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
	SPIByte(cmd);	//首命令
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
  SST25WREN  允许写功能
***************************************/
void SST25WREN(void)
{
	SSTCmd1(0x06);
}

/***********************************
  SST25WRDI  屏蔽写功能
***********************************/
void SST25WRDI(void)
{
	SSTCmd1(0x04);
}

/**********************************
  SST25BY  检测忙
**********************************/
u8 SST25BY(void)
{
	u8 sta;
	sta=SSTCmdb1b(0x05);
	return(sta&0x01);
}

/***********************************
  SST25WPEN 允许软件写保护
  注意事项:25的写入比较繁琐，建议在每次操作前都取消掉写保护，操作完成后则重新允许写保护
***********************************/
void SST25WPEN(void)
{
	u8 sta;
	sta=SSTCmdb1b(0x05)|0x1c;		//读出寄存器并加入保护位
	SSTCmd1(0x50);					//允许写
	SSTCmd2(0x01,sta);
}

//先消除保护位，再允许写位
void SST25WriteEn(void)
{
	u8 sta;
	sta=SSTCmdb1b(0x05)&(~0x1c);	//读出寄存器并消除保护位
	SSTCmd1(0x50);					//允许写寄存器
	SSTCmd2(0x01,sta);				//写寄存器
	SSTCmd1(0x06);					//允许写
}

/**********************************
  SST25ReadID 读取SST的ID
**********************************/
u16 SST25ReadID(void)
{
	u8 id[3];
	u8 addr[3]={0,0,0};
	
	SSTCmd4bs(0x90,addr,id,3);
	return((id[0]<<8)+id[1]);
}

/**********************************
  SST25ChipErase 刷除CHIP
**********************************/
void SST25ChipErase(void)
{
	SST25WriteEn();
	SSTCmd1(0x60);
	while(SST25BY());
	SST25WPEN();
}

/***********************************
  SST25SectorErase  刷扇区
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
  SST25ByteProgram  写一个字节*注意在此前要调用取消写保护,实际写应使用AAI,此函数在AAI中调用，用于写奇数个字节
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
  SST25Write 写多个字节
***********************************/
void sst25_Write(u32 addr,u8* p_data,u32 no)
{
	u8 ad[3];
	u32 cnt;
	if(no==0)
		return;
		
	SST25WriteEn();
	
	if(no==1)	//no<2则应使用普通单字节方式
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
		while(SST25BY());//判忙
		
		//中间的双字节写
		for(;cnt>1;cnt-=2)
		{
			SST_SELECT();
			SPIByte(0xad);
			SPIByte(*p_data++);
			SPIByte(*p_data++);
			SST_DESELECT();
			while(SST25BY());//判忙
		}
		SST25WRDI();//WRDI用于退出AAI写模式
		
		//如果有最后一个字节(no为奇数）
		if(cnt==1)
		{
			SST25WriteEn();
			SST25ByteProgram(addr+no-1,*p_data);
		}
	}
	while(SST25BY());
	SST25WPEN();//WP保护
}

/*************************************
  SST25Read 高速读 对于后续带5的芯片，可调用此函数读
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
SST25ReadL  低速读
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
