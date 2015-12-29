/* *****************************************************************************
2012-10-11
 	16G SD����û���⣬д������
****************************************************************************** */
#include "hal.h"
#include "diskio.h"
#include "ff.h"
#include "msd.h"

static volatile DSTATUS msd_Stat = STA_NOINIT;	/* Disk status */

/******************************************************************************* 
*  ���ļ�ΪSPI����SD���ĵײ������ļ�
*  ����SPIģ�鼰���IO�ĳ�ʼ����SPI��дSD����дָ������ݵȣ�
*******************************************************************************/   
#define SD_TYPE_MMC     0
#define SD_TYPE_V1      1
#define SD_TYPE_V2      2
#define SD_TYPE_V2HC    4

/* SPI�����ٶ�����*/
#define SPI_SPEED_LOW   0
#define SPI_SPEED_HIGH  1

/* SD�������ݽ������Ƿ��ͷ����ߺ궨�� */
#define NO_RELEASE      0
#define RELEASE         1
							  
#if 1
/* SD��ָ��� */
#define CMD0    0       //����λ
#define CMD8	   8
#define CMD9    9       //����9 ����CSD����
#define CMD10   10      //����10����CID����
#define CMD12   12      //����12��ֹͣ���ݴ���
#define CMD16   16      //����16������SectorSize Ӧ����0x00
#define CMD17   17      //����17����sector
#define CMD18   18      //����18����Multi sector
#define ACMD23  23      //����23�����ö�sectorд��ǰԤ�Ȳ���N��block
#define CMD24   24      //����24��дsector
#define CMD25   25      //����25��дMulti sector
#define ACMD41  41      //����41��Ӧ����0x00
#define CMD55   55      //����55��Ӧ����0x01
#define CMD58   58      //����58����OCR��Ϣ
#define CMD59   59      //����59��ʹ��/��ֹCRC��Ӧ����0x00
#endif

#define SD_CS_ENABLE()  GPIO_ResetBits(GPIOA, GPIO_Pin_4)  //ѡ��SD��
#define SD_CS_DISABLE() GPIO_SetBits(GPIOA, GPIO_Pin_4)	  //ȡ��ѡ��				    	 

/* Private function prototypes -----------------------------------------------*/
/*
void SPI_Configuration(void);
void SPI_SetSpeed(u8 SpeedSet);

u8 SPI_ReadWriteByte(u8 TxData);                //SPI���߶�дһ���ֽ�
u8 SD_WaitReady(void);                          //�ȴ�SD������
u8 SD_SendCommand(u8 cmd, u32 arg, u8 crc);     //SD������һ������
u8 SD_SendCommand_NoDeassert(u8 cmd, u32 arg, u8 crc);
*/
u8 SD_Init(void);                               //SD����ʼ��
/*                                                //
u8 SD_ReceiveData(u8 *data, u16 len, u8 release);//SD��������
u8 SD_GetCID(u8 *cid_data);                     //��SD��CID
u8 SD_GetCSD(u8 *csd_data);                     //��SD��CSD
*/
u32 SD_GetCapacity(void);                       //ȡSD������

u8 SD_ReadSingleBlock(u32 sector, u8 *buffer);  //��һ��sector
u8 SD_WriteSingleBlock(u32 sector, const u8 *buffer); //дһ��sector
u8 SD_ReadMultiBlock(u32 sector, u8 *buffer, u8 count); //�����sector
u8 SD_WriteMultiBlock(u32 sector, const u8 *data, u8 count);  //д���sector
u8 SD_Read_Bytes(unsigned long address,unsigned char *buf,unsigned int offset,unsigned int bytes);//��ȡһbyte

volatile BOOL SD_changed=TRUE;
volatile BOOL sd_insert_flag= FALSE;
volatile BOOL sd_valid_flag= FALSE;

u8  SD_Type=0;//SD��������	 
////////////////////////////////////////////////////////////////////////////////
// ������SPIģ��ĳ�ʼ�����룬���ó�����ģʽ������SD��
////////////////////////////////////////////////////////////////////////////////
//SD���������  SD_CD     PC4	SD��������
#if (GC_ZR&GC_ZR_GPIO)
#define SD_NCD_PORT	GPIOC
#define SD_NCD_Pin		GPIO_Pin_4
#define SD_SPI_NAME		1
#define sd_spi		SPI1
#define SD_DETECT	(!GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4))
#else
#define SD_NCD_PORT	GPIOA
#define SD_NCD_Pin		GPIO_Pin_3
#define SD_SPI_NAME		1
#define sd_spi		SPI1
#define SD_DETECT	(!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3))
#endif

BOOL	sd_IsInserted()
{
	return (BOOL)SD_DETECT;
}

void sd_Config()
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
#if 1
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
#endif	

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA  | RCC_APB2Periph_AFIO ,  ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;// | GPIO_Pin_3| GPIO_Pin_5| GPIO_Pin_6| GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*
	GPIO_SetBits(GPIOA, GPIO_Pin_3);
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
	GPIO_SetBits(GPIOA, GPIO_Pin_5);
	GPIO_SetBits(GPIOA, GPIO_Pin_6);
	GPIO_SetBits(GPIOA, GPIO_Pin_7);

	GPIO_ResetBits(GPIOA, GPIO_Pin_3);
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);
	GPIO_ResetBits(GPIOA, GPIO_Pin_5);
	GPIO_ResetBits(GPIOA, GPIO_Pin_6);
	GPIO_ResetBits(GPIOA, GPIO_Pin_7);
	*/
#if (GC_ZR&GC_ZR_GPIO)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
#else
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif
#if (SD_SPI_NAME==1)

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA  |
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
	

#elif (SD_SPI_NAME==2)

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	// SCK, MISO and MOSI  B13=CLK,B14=MISO,B15=MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);


#elif (SD_SPI_NAME==3)

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	// SCK, MISO and MOSI  B3=CLK,B4=MISO,B5=MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

#endif 
	// SPI configuration  ע��25ϵ�е��ز���
	SPI_Cmd(sd_spi, DISABLE); 												//�����Ƚ���,���ܸı�MODE
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;		//����ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;							//��
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;						//8λ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;								//CPOL=1 ʱ�����ո�
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;							//CPHA=1 ���ݲ���ڶ���
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;								//���NSS
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;	//256��Ƶ
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;						//��λ��ǰ
	SPI_InitStructure.SPI_CRCPolynomial = 7;								//CRC7

	SD_CS_DISABLE();
	SPI_Init(sd_spi, &SPI_InitStructure);
	// SPI_SSOutputCmd(sd_spi, ENABLE); //ʹ��NSS�ſ���
	SPI_Cmd(sd_spi, ENABLE); 
#if 1	// SD�����뼰�������жϼ��
	/* ����IO�ڵ��ж��� */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource3);

	/*����Ϊ�½��ش���*/
	EXTI_InitStructure.EXTI_Line = EXTI_Line3;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	// ����ʹ��EXTI8-EXTI11�ж�
	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// sd_insert_flag= sd_IsInserted();
#endif
}

/*******************************************************************************
* Function Name  : sd_spi_SetSpeed
* Description    : SPI�����ٶ�Ϊ����
* Input          : u8 SpeedSet 
*                  ����ٶ���������0�������ģʽ����0�����ģʽ
*                  SPI_SPEED_HIGH   1
*                  SPI_SPEED_LOW    0
* Output         : None
* Return         : None
*******************************************************************************/
void sd_spi_SetSpeed(u8 SpeedSet)
{
	sd_spi->CR1&=0XFFC7;//Fsck=Fcpu/256
	if(SpeedSet==SPI_SPEED_HIGH)//����
	{
		sd_spi->CR1|=1<<3;//Fsck=Fpclk/4=18Mhz	
	}else//����
	{
		sd_spi->CR1|=6<<3; //Fsck=Fpclk/256=562.5Khz
	}
	sd_spi->CR1|=1<<6; //SPI�豸ʹ��	  
}
	 
/*******************************************************************************
* Function Name  : sd_ReadWriteByte
* Description    : SPI��дһ���ֽڣ�������ɺ󷵻ر���ͨѶ��ȡ�����ݣ�
* Input          : u8 TxData �����͵���
* Output         : None
* Return         : u8 RxData �յ�����
*******************************************************************************/
u8 sd_ReadWriteByte(u8 TxData)
{
	while((sd_spi->SR&1<<1)==0);//�ȴ���������				  
	sd_spi->DR=TxData;	 	  //����һ��byte   
	while((sd_spi->SR&1<<0)==0);//�ȴ�������һ��byte  
	return sd_spi->DR;          //�����յ�������				    
}

static
void SD_SPI_Release(void)
{
	SD_CS_DISABLE();
	sd_ReadWriteByte(0xff);;
}

/*******************************************************************************
* Function Name  : SD_WaitReady
* Description    : �ȴ�SD��Ready
* Input          : None
* Output         : None
* Return         : u8 
*                   0�� �ɹ�
*                   other��ʧ��
*******************************************************************************/
u8 SD_WaitReady(void)
{
	u8 r1=MSD_DATA_OTHER_ERROR;
	u32 retry;
#if 0	
	/* retry=0;
	do
	{
		r1=sd_ReadWriteByte(0xFF)&0x1F;//������Ӧ
		if(retry==0xfffe)return 1;
		retry++;
		switch (r1)
		{
		case MSD_DATA_OK://���ݽ�����ȷ��
			r1=MSD_DATA_OK;
			break;
		case MSD_DATA_CRC_ERROR: //CRCУ�����
			return MSD_DATA_CRC_ERROR;
		case MSD_DATA_WRITE_ERROR://����д�����
			return MSD_DATA_WRITE_ERROR;
		default://δ֪����
			r1=MSD_DATA_OTHER_ERROR;
			break;
		}
	}while(r1==MSD_DATA_OTHER_ERROR); //���ݴ���ʱһֱ�ȴ�
	*/
	retry=0;
	while(sd_ReadWriteByte(0xFF)==0)//��������Ϊ0,�����ݻ�δд���
	{
		retry++;
		//delay_us(10);//SD��д�ȴ���Ҫ�ϳ���ʱ��
		if(retry>=0XFFFFFFFE)return 0XFF;//�ȴ�ʧ����
	};
#else	
	retry = 0;
	do
	{
		r1 = sd_ReadWriteByte(0xFF);
		retry++;
		if(retry>=0xfffffe)return 1; 
	}while(r1!=0xFF); 
#endif	
	return 0;
}	 
/*******************************************************************************
* Function Name  : SD_SendCommand
* Description    : ��SD������һ������
* Input          : u8 cmd   ���� 
*                  u32 arg  �������
*                  u8 crc   crcУ��ֵ
* Output         : None
* Return         : u8 r1 SD�����ص���Ӧ
*******************************************************************************/
u8 SD_SendCommand(u8 cmd, u32 arg, u8 crc)
{
    unsigned char r1;
    unsigned char Retry = 0;

    //????????
    sd_ReadWriteByte(0xff);
    //Ƭѡ���õͣ�ѡ��SD��
    SD_CS_ENABLE();

    //����
    sd_ReadWriteByte(cmd | 0x40);                         //�ֱ�д������
    sd_ReadWriteByte(arg >> 24);
    sd_ReadWriteByte(arg >> 16);
    sd_ReadWriteByte(arg >> 8);
    sd_ReadWriteByte(arg);
    sd_ReadWriteByte(crc);
    
    //�ȴ���Ӧ����ʱ�˳�
    while((r1 = sd_ReadWriteByte(0xFF))==0xFF)
    {
        Retry++;
        if(Retry > 200)break; 
    }   
    //�ر�Ƭѡ
    SD_CS_DISABLE();
    //�������϶�������8��ʱ�ӣ���SD�����ʣ�µĹ���
    sd_ReadWriteByte(0xFF);

    //����״ֵ̬
    return r1;
}


/*******************************************************************************
* Function Name  : SD_SendCommand_NoDeassert
* Description    : ��SD������һ������(�����ǲ�ʧ��Ƭѡ�����к������ݴ�����
* Input          : u8 cmd   ���� 
*                  u32 arg  �������
*                  u8 crc   crcУ��ֵ
* Output         : None
* Return         : u8 r1 SD�����ص���Ӧ
*******************************************************************************/
u8 SD_SendCommand_NoDeassert(u8 cmd, u32 arg, u8 crc)
{
    unsigned char r1;
    unsigned char Retry = 0;

    //????????
    sd_ReadWriteByte(0xff);
    //Ƭѡ���õͣ�ѡ��SD��
    SD_CS_ENABLE();

    //����
    sd_ReadWriteByte(cmd | 0x40);                         //�ֱ�д������
    sd_ReadWriteByte(arg >> 24);
    sd_ReadWriteByte(arg >> 16);
    sd_ReadWriteByte(arg >> 8);
    sd_ReadWriteByte(arg);
    sd_ReadWriteByte(crc);

    //�ȴ���Ӧ����ʱ�˳�
    while((r1 = sd_ReadWriteByte(0xFF))==0xFF)
    {
        Retry++;
        if(Retry > 200)break;   
    }
    //������Ӧֵ
    return r1;
}

/*******************************************************************************
* Function Name  : SD_Init
* Description    : ��ʼ��SD��
* Input          : None
* Output         : None
* Return         : u8 
*                  0��NO_ERR
*                  1��TIME_OUT
*                  99��NO_CARD
*******************************************************************************/
u8 SD_Init(void)
{
	u16 i;      // ����ѭ������
	u8 r1;      // ���SD���ķ���ֵ
	u16 retry;  // �������г�ʱ����
	u8 buff[6];
	if(!SD_DETECT) return 99;
	
	sd_spi_SetSpeed(0);
	SD_CS_ENABLE();
					  
	// ����ʱ���ȴ�SD���ϵ����
	for(i=0;i<0xf00;i++);

	//�Ȳ���>74�����壬��SD���Լ���ʼ�����
	for(i=0;i<10;i++)
	{
		sd_ReadWriteByte(0xFF);
	}

	//-----------------SD����λ��idle��ʼ-----------------
	//ѭ����������CMD0��ֱ��SD������0x01,����IDLE״̬
	//��ʱ��ֱ���˳�
	retry = 0;
	do
	{
		//����CMD0����SD������IDLE״̬
		r1 = SD_SendCommand(CMD0, 0, 0x95);
		retry++;
	}while((r1 != 0x01) && (retry<200));
	//����ѭ���󣬼��ԭ�򣺳�ʼ���ɹ���or ���Գ�ʱ��
	if(retry==200) return 1;   //��ʱ����1	  
	//-----------------SD����λ��idle����-----------------	 
	//��ȡ��Ƭ��SD�汾��Ϣ
	r1 = SD_SendCommand_NoDeassert(CMD8, 0x1aa, 0x87);	     
	//�����Ƭ�汾��Ϣ��v1.0�汾�ģ���r1=0x05����������³�ʼ��
	if(r1 == 0x05)
	{
		//���ÿ�����ΪSDV1.0����������⵽ΪMMC�������޸�ΪMMC
		SD_Type = SD_TYPE_V1;	   
		//�����V1.0����CMD8ָ���û�к�������
		//Ƭѡ�øߣ�������������
		SD_CS_DISABLE();
		//�෢8��CLK����SD������������
		sd_ReadWriteByte(0xFF);	  
		//-----------------SD����MMC����ʼ����ʼ-----------------	 
		//������ʼ��ָ��CMD55+ACMD41
		// �����Ӧ��˵����SD�����ҳ�ʼ�����
		// û�л�Ӧ��˵����MMC�������������Ӧ��ʼ��
		retry = 0;
		do
		{
			//�ȷ�CMD55��Ӧ����0x01���������
			r1 = SD_SendCommand(CMD55, 0, 0);
			if(r1 != 0x01)return r1;	  
			//�õ���ȷ��Ӧ�󣬷�ACMD41��Ӧ�õ�����ֵ0x00����������200��
			r1 = SD_SendCommand(ACMD41, 0, 0);
			retry++;
		}while((r1!=0x00) && (retry<400));
		// �ж��ǳ�ʱ���ǵõ���ȷ��Ӧ
		// ���л�Ӧ����SD����û�л�Ӧ����MMC��
        
		//----------MMC�������ʼ��������ʼ------------
		if(retry==400)
		{
			retry = 0;
			//����MMC����ʼ�����û�в��ԣ�
			do
			{
				r1 = SD_SendCommand(1, 0, 0);
				retry++;
			}while((r1!=0x00)&& (retry<400));
			if(retry==400)return 1;   //MMC����ʼ����ʱ		    
			//д�뿨����
			SD_Type = SD_TYPE_MMC;
		}
		//----------MMC�������ʼ����������------------	    
		//����SPIΪ����ģʽ
		sd_spi_SetSpeed(1);   
		sd_ReadWriteByte(0xFF);
        
		//��ֹCRCУ��	   
		r1 = SD_SendCommand(CMD59, 0, 0x95);
		if(r1 != 0x00)return r1;  //������󣬷���r1   	   
		//����Sector Size
		r1 = SD_SendCommand(CMD16, 512, 0x95);
		if(r1 != 0x00)return r1;//������󣬷���r1		 
		//-----------------SD����MMC����ʼ������-----------------

	}//SD��ΪV1.0�汾�ĳ�ʼ������	 
	//������V2.0���ĳ�ʼ��
	//������Ҫ��ȡOCR���ݣ��ж���SD2.0����SD2.0HC��
	else 
		if(r1 == 0x01)
		{
			//V2.0�Ŀ���CMD8�����ᴫ��4�ֽڵ����ݣ�Ҫ�����ٽ���������
			buff[0] = sd_ReadWriteByte(0xFF);  //should be 0x00
			buff[1] = sd_ReadWriteByte(0xFF);  //should be 0x00
			buff[2] = sd_ReadWriteByte(0xFF);  //should be 0x01
			buff[3] = sd_ReadWriteByte(0xFF);  //should be 0xAA	    
			SD_CS_DISABLE();	  
			sd_ReadWriteByte(0xFF);//the next 8 clocks			 
			//�жϸÿ��Ƿ�֧��2.7V-3.6V�ĵ�ѹ��Χ
			//if(buff[2]==0x01 && buff[3]==0xAA) //���жϣ�����֧�ֵĿ�����
			{	  
				retry = 0;
				//������ʼ��ָ��CMD55+ACMD41
				do
				{
					r1 = SD_SendCommand(CMD55, 0, 0);
					if(r1!=0x01)return r1;	   
					r1 = SD_SendCommand(ACMD41, 0x40000000, 0);
					if(retry>200)return r1;  //��ʱ�򷵻�r1״̬  
				}while(r1!=0);		  
				//��ʼ��ָ�����ɣ���������ȡOCR��Ϣ		   
				//-----------����SD2.0���汾��ʼ-----------
				r1 = SD_SendCommand_NoDeassert(CMD58, 0, 0);
				if(r1!=0x00)return r1;  //�������û�з�����ȷӦ��ֱ���˳�������Ӧ��		 
				//��OCRָ����󣬽�������4�ֽڵ�OCR��Ϣ
				buff[0] = sd_ReadWriteByte(0xFF);
				buff[1] = sd_ReadWriteByte(0xFF); 
				buff[2] = sd_ReadWriteByte(0xFF);
				buff[3] = sd_ReadWriteByte(0xFF);

				//OCR������ɣ�Ƭѡ�ø�
				SD_CS_DISABLE();
				sd_ReadWriteByte(0xFF);

				//�����յ���OCR�е�bit30λ��CCS����ȷ����ΪSD2.0����SDHC
				//���CCS=1��SDHC   CCS=0��SD2.0
				if(buff[0]&0x40)
					SD_Type = SD_TYPE_V2HC;    //���CCS	 
				else 
					SD_Type = SD_TYPE_V2;	    
				//-----------����SD2.0���汾����----------- 
				//����SPIΪ����ģʽ
				sd_spi_SetSpeed(1);  
			}	    
		}
	return r1;
}



/*******************************************************************************
* Function Name  : SD_ReceiveData
* Description    : ��SD���ж���ָ�����ȵ����ݣ������ڸ���λ��
* Input          : u8 *data(��Ŷ������ݵ��ڴ�>len)
*                  u16 len(���ݳ��ȣ�
*                  u8 release(������ɺ��Ƿ��ͷ�����CS�ø� 0�����ͷ� 1���ͷţ�
* Output         : None
* Return         : u8 
*                  0��NO_ERR
*                  other��������Ϣ
*******************************************************************************/
u8 SD_ReceiveData(u8 *data, u16 len, u8 release)
{
    u16 retry;
    u8 r1;

    // ����һ�δ���
    SD_CS_ENABLE();
    //�ȴ�SD������������ʼ����0xFE
    retry = 0;										   
	do
    {
        r1 = sd_ReadWriteByte(0xFF);
        retry++;
        if(retry>16000)  //2000�εȴ���û��Ӧ���˳�����
        {
            SD_CS_DISABLE();
            return 1;
        }
    }while(r1 != 0xFE);
		   
    //��ʼ��������
    while(len--)
    {
        *data = sd_ReadWriteByte(0xFF);
        data++;
    }
    //������2��αCRC��dummy CRC��
    sd_ReadWriteByte(0xFF);
    sd_ReadWriteByte(0xFF);
    //�����ͷ����ߣ���CS�ø�
    if(release == RELEASE)
    {
        //�������
        SD_CS_DISABLE();
        sd_ReadWriteByte(0xFF);
    }											  					    
    return 0;
}


/*******************************************************************************
* Function Name  : SD_GetCID
* Description    : ��ȡSD����CID��Ϣ��������������Ϣ
* Input          : u8 *cid_data(���CID���ڴ棬����16Byte��
* Output         : None
* Return         : u8 
*                  0��NO_ERR
*                  1��TIME_OUT
*                  other��������Ϣ
*******************************************************************************/
u8 SD_GetCID(u8 *cid_data)
{
    u8 r1;

    //��CMD10�����CID
    r1 = SD_SendCommand(CMD10, 0, 0xFF);
    if(r1 != 0x00)return r1;  //û������ȷӦ�����˳�������    
    //����16���ֽڵ�����
    SD_ReceiveData(cid_data, 16, RELEASE);	 
    return 0;
}


/*******************************************************************************
* Function Name  : SD_GetCSD
* Description    : ��ȡSD����CSD��Ϣ�������������ٶ���Ϣ
* Input          : u8 *cid_data(���CID���ڴ棬����16Byte��
* Output         : None
* Return         : u8 
*                  0��NO_ERR
*                  1��TIME_OUT
*                  other��������Ϣ
*******************************************************************************/
u8 SD_GetCSD(u8 *csd_data)
{
    u8 r1;

    //��CMD9�����CSD
    r1 = SD_SendCommand(CMD9, 0, 0xFF);
    if(r1 != 0x00)return r1;  //û������ȷӦ�����˳�������  
    //����16���ֽڵ�����
    SD_ReceiveData(csd_data, 16, RELEASE);

    return 0;
}


/*******************************************************************************
* Function Name  : SD_GetCapacity
* Description    : ��ȡSD�����������ֽڣ�
* Input          : None
* Output         : None
* Return         : u32 capacity 
*                   0�� ȡ�������� 
*******************************************************************************/
u32 SD_GetCapacity(void)
{
    u8 csd[16];
    u32 Capacity;
    u8 r1;
    u16 i;
	u16 temp;

    //ȡCSD��Ϣ������ڼ��������0
    if(SD_GetCSD(csd)!=0) return 0;	    
    //���ΪSDHC�����������淽ʽ����
    if((csd[0]&0xC0)==0x40)
    {									  
	    Capacity=((u32)csd[8])<<8;
		Capacity+=(u32)csd[9]+1;	 
        Capacity = (Capacity)*1024;//�õ�������
		Capacity*=512;//�õ��ֽ���			   
    }
    else
    {		    
    	i = csd[6]&0x03;
    	i<<=8;
    	i += csd[7];
    	i<<=2;
    	i += ((csd[8]&0xc0)>>6);
    
        //C_SIZE_MULT
    	r1 = csd[9]&0x03;
    	r1<<=1;
    	r1 += ((csd[10]&0x80)>>7);	 
    	r1+=2;//BLOCKNR
    	temp = 1;
    	while(r1)
    	{
    		temp*=2;
    		r1--;
    	}
    	Capacity = ((u32)(i+1))*((u32)temp);	 
        // READ_BL_LEN
    	i = csd[5]&0x0f;
        //BLOCK_LEN
    	temp = 1;
    	while(i)
    	{
    		temp*=2;
    		i--;
    	}
        //The final result
    	Capacity *= (u32)temp;//�ֽ�Ϊ��λ 	  
    }
    return (u32)Capacity;
}


/*******************************************************************************
* Function Name  : SD_ReadSingleBlock
* Description    : ��SD����һ��block
* Input          : u32 sector ȡ��ַ��sectorֵ���������ַ�� 
*                  u8 *buffer ���ݴ洢��ַ����С����512byte�� 
* Output         : None
* Return         : u8 r1 
*                   0�� �ɹ�
*                   other��ʧ��
*******************************************************************************/
u8 SD_ReadSingleBlock(u32 sector, u8 *buffer)
{
	u8 r1;

    //����Ϊ����ģʽ
    sd_spi_SetSpeed(SPI_SPEED_HIGH);
    
    //�������SDHC����������sector��ַ������ת����byte��ַ
    if(SD_Type!=SD_TYPE_V2HC)
    {
        sector = sector<<9;
    }

	r1 = SD_SendCommand(CMD17, sector, 0);//������
												    
	if(r1 != 0x00)return r1; 		   							  
	r1 = SD_ReceiveData(buffer, 512, RELEASE);		 
	if(r1 != 0)return r1;   //�����ݳ���
    else return 0; 
}

/*******************************************************************************
* Function Name  : SD_WriteSingleBlock
* Description    : д��SD����һ��block
* Input          : u32 sector ������ַ��sectorֵ���������ַ�� 
*                  u8 *buffer ���ݴ洢��ַ����С����512byte�� 
* Output         : None
* Return         : u8 r1 
*                   0�� �ɹ�
*                   other��ʧ��
*******************************************************************************/
u8 SD_WriteSingleBlock(u32 sector, const u8 *data)
{
    u8 r1;
    u16 i;
    u16 retry;

    //����Ϊ����ģʽ
    sd_spi_SetSpeed(SPI_SPEED_HIGH);

    //�������SDHC����������sector��ַ������ת����byte��ַ
    if(SD_Type!=SD_TYPE_V2HC)
    {
        sector = sector<<9;
    }

    r1 = SD_SendCommand(CMD24, sector, 0x00);
    if(r1 != 0x00)
    {
        return r1;  //Ӧ����ȷ��ֱ�ӷ���
    }
    
    //��ʼ׼�����ݴ���
    SD_CS_ENABLE();
    //�ȷ�3�������ݣ��ȴ�SD��׼����
    sd_ReadWriteByte(0xff);
    sd_ReadWriteByte(0xff);
    sd_ReadWriteByte(0xff);
    //����ʼ����0xFE
    sd_ReadWriteByte(0xFE);

    //��һ��sector������
    for(i=0;i<512;i++)
    {
        sd_ReadWriteByte(*data++);
    }
    //��2��Byte��dummy CRC
    sd_ReadWriteByte(0xff);
    sd_ReadWriteByte(0xff);
    
    //�ȴ�SD��Ӧ��
    r1 = sd_ReadWriteByte(0xff);
    if((r1&0x1F)!=0x05)
    {
        SD_CS_DISABLE();
        return r1;
    }
    
    //�ȴ��������
    retry = 0;
    while(!sd_ReadWriteByte(0xff))
    {
        retry++;
        if(retry>0xfffe)        //�����ʱ��д��û����ɣ������˳�
        {
            SD_CS_DISABLE();
            return 1;           //д�볬ʱ����1
        }
    }

    //д����ɣ�Ƭѡ��1
    SD_CS_DISABLE();
    sd_ReadWriteByte(0xff);

    return 0;
}


/*******************************************************************************
* Function Name  : SD_ReadMultiBlock
* Description    : ��SD���Ķ��block
* Input          : u32 sector ȡ��ַ��sectorֵ���������ַ�� 
*                  u8 *buffer ���ݴ洢��ַ����С����512byte��
*                  u8 count ������count��block
* Output         : None
* Return         : u8 r1 
*                   0�� �ɹ�
*                   other��ʧ��
*******************************************************************************/
u8 SD_ReadMultiBlock(u32 sector, u8 *buffer, u8 count)
{
    u8 r1;	 			 
    sd_spi_SetSpeed(SPI_SPEED_HIGH);//����Ϊ����ģʽ  
    if(SD_Type != SD_TYPE_V2HC)
		sector = sector<<9;//�������SDHC����sector��ַת��byte��ַ
    // SD_WaitReady();
    //�����������
	r1 = SD_SendCommand(CMD18, sector, 0);//������
	if(r1 != 0x00)return r1;	 
    do//��ʼ��������
    {
        if(SD_ReceiveData(buffer, 512, NO_RELEASE) != 0x00)
        {
            break;
        }
        buffer += 512;
    } while(--count);		 
    //ȫ��������ϣ�����ֹͣ����
    SD_SendCommand(CMD12, 0, 0);
    //�ͷ�����
    SD_CS_DISABLE();
    sd_ReadWriteByte(0xFF);    
    if(count != 0)return count;   //���û�д��꣬����ʣ�����	 
    else return 0;	 
}


/*******************************************************************************
* Function Name  : SD_WriteMultiBlock
* Description    : д��SD����N��block
* Input          : u32 sector ������ַ��sectorֵ���������ַ�� 
*                  u8 *buffer ���ݴ洢��ַ����С����512byte��
*                  u8 count д���block��Ŀ
* Output         : None
* Return         : u8 r1 
*                   0�� �ɹ�
*                   other��ʧ��
*******************************************************************************/
u8 SD_WriteMultiBlock(u32 sector, const u8 *data, u8 count)
{
    u8 r1;
    u16 i;	 		 
    sd_spi_SetSpeed(SPI_SPEED_HIGH);//����Ϊ����ģʽ	 
    if(SD_Type != SD_TYPE_V2HC)sector = sector<<9;//�������SDHC����������sector��ַ������ת����byte��ַ  
    if(SD_Type != SD_TYPE_MMC) r1 = SD_SendCommand(ACMD23, count, 0x00);//���Ŀ�꿨����MMC��������ACMD23ָ��ʹ��Ԥ����   
    r1 = SD_SendCommand(CMD25, sector, 0x00);//�����д��ָ��
    if(r1 != 0x00)return r1;  //Ӧ����ȷ��ֱ�ӷ���	 
    SD_CS_ENABLE();//��ʼ׼�����ݴ���   
    sd_ReadWriteByte(0xff);//�ȷ�3�������ݣ��ȴ�SD��׼����
    sd_ReadWriteByte(0xff);   
    sd_ReadWriteByte(0xff);   
    //--------������N��sectorд���ѭ������
    do
    {
        //����ʼ����0xFC �����Ƕ��д��
        sd_ReadWriteByte(0xFC);	  
        //��һ��sector������
        for(i=0;i<512;i++)
        {
            sd_ReadWriteByte(*data++);
        }
        //��2��Byte��dummy CRC
        sd_ReadWriteByte(0xff);
        sd_ReadWriteByte(0xff);
        
        //�ȴ�SD��Ӧ��
        r1 = sd_ReadWriteByte(0xff);
        if((r1&0x1F)!=0x05)
        {
            SD_CS_DISABLE();    //���Ӧ��Ϊ��������������ֱ���˳�
            return r1;
        }

        //�ȴ�SD��д�����
        if(SD_WaitReady()==1)
        {
            SD_CS_DISABLE();    //�ȴ�SD��д����ɳ�ʱ��ֱ���˳�����
            return 1;
        }	   
    }while(--count);//��sector���ݴ������
    
    //��������������0xFD
    r1 = sd_ReadWriteByte(0xFD);
    if(r1==0x00)
    {
        count =  0xfe;
    }		   
    if(SD_WaitReady()) //�ȴ�׼����
	{
		SD_CS_DISABLE();
		return 1;  
	}
    //д����ɣ�Ƭѡ��1
    SD_CS_DISABLE();
    sd_ReadWriteByte(0xff);  
    return count;   //����countֵ�����д����count=0������count=1
}
											 
/*******************************************************************************
* Function Name  : SD_Read_Bytes
* Description    : ��ָ������,��offset��ʼ����bytes���ֽ�
* Input          : u32 address ������ַ��sectorֵ���������ַ�� 
*                  u8 *buf     ���ݴ洢��ַ����С<=512byte��
*                  u16 offset  �����������ƫ����
                   u16 bytes   Ҫ�������ֽ���
* Output         : None
* Return         : u8 r1 
*                   0�� �ɹ�
*                   other��ʧ��
*******************************************************************************/
u8 SD_Read_Bytes(unsigned long address,unsigned char *buf,unsigned int offset,unsigned int bytes)
{
    u8 r1;u16 i=0;  
    r1=SD_SendCommand(CMD17,address<<9,0);//���Ͷ���������      
    if(r1!=0x00)return r1;  //Ӧ����ȷ��ֱ�ӷ���
	SD_CS_ENABLE();//ѡ��SD��
	while (sd_ReadWriteByte(0xff)!= 0xFE)//ֱ����ȡ�������ݵĿ�ʼͷ0XFE���ż���
	{
		i++;
		if(i>2000)
		{
			SD_CS_DISABLE();//�ر�SD��
			return 1;//��ȡʧ��
		}
	}; 		 
	for(i=0;i<offset;i++)sd_ReadWriteByte(0xff);//����offsetλ 
    for(;i<offset+bytes;i++)*buf++=sd_ReadWriteByte(0xff);//��ȡ��������	
    for(;i<512;i++) sd_ReadWriteByte(0xff); 	 //����ʣ���ֽ�
    sd_ReadWriteByte(0xff);//����αCRC��
    sd_ReadWriteByte(0xff);  
    SD_CS_DISABLE();//�ر�SD��
	return 0;
}

/*--------------------------------------------------------------------------

  Public Functions

  ---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/
//��auto_mount()�ڵ���
DSTATUS msd_disk_initialize ()
{
	BYTE ty;//, ocr[16], cmd;
	//int n;
#if 1
	ty= SD_Init();
#else	
	SD_SPI_Config(); //��ʼ��IO

	for (Timer1=50; Timer1; );              

	sd_spi_SetSpeed(SPI_SPEED_LOW);
	SD_CS_DISABLE();							//��ѡSD

	/* Wait for enter Idle state in timeout of 5000 msec */
	Timer1 = 500;
	do
	{
		for (n = 10; n; n--) sd_ReadWriteByte(0xff);		/* 80 dummy clocks */
	}
	while ((SD_Send_Command(CMD0,0) != 1) && Timer1);

	ty = 0;
	Timer1 = 200;					/* Initialization timeout of 2000 msec */
	if (Send_Command(CMD8, 0x1AA) == 1)		  /* ����Ƿ�֧��SDC Ver2 */
	{
		for (n = 0; n < 4; n++)
			ocr[n] = SPI_ReadWrite_Byte(0xff);	/* Get trailing return value of R7 resp */
		if (ocr[2] == 0x01 && ocr[3] == 0xAA)
		{
			/* The card can work at vdd range of 2.7-3.6V */
			/* Wait for leaving idle state (ACMD41 with HCS bit) */
			while (Timer1 && Send_Command(ACMD41, 1UL << 30));                  
			if (Timer1 && Send_Command(CMD58, 0) == 0)
			{
				/* Check CCS bit in the OCR */
				for (n = 0; n < 4; n++)
					ocr[n] = SPI_ReadWrite_Byte(0xff);
				/* When CCS bit is set  R/W in block address insted of byte address */
				ty = (ocr[0] & 0x40) ? 12 : 4;
			}
		}
	}
	else
	{
		/* SDSC or MMC */
		if (Send_Command(ACMD41, 0) <= 1)			/* initialize successful will response 0x00 */
		{
			ty = 2; cmd = ACMD41;	/* SDv1 */
		}
		else
		{
			ty = 1; cmd = CMD1;		/* MMC */
		}
		while (Timer1 && Send_Command(cmd, 0));			/* Wait for leaving idle state */
		if (!Timer1 || Send_Command(CMD16, 512) != 0)	/* Set R/W block length to 512 */
			ty = 0;
	}

	CardType = ty;  
	SPI_HighSpeed();
	SD_SPI_Release();
#endif
	if (ty==0)
	{
		/* Initialization succeded */
		msd_Stat &= ~STA_NOINIT;    
		//    USART1_Puts("Initialization succeded.\n");
	}
	else
	{
		/* Initialization failed */
		//    USART1_Puts("Initialization failed.\n");
	}

	return msd_Stat;
}


/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/
DSTATUS msd_disk_status ()
{
	return msd_Stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT msd_disk_read (
				  BYTE *buff,		  /* Pointer to the data buffer to store read data */
				  DWORD sector,				  /* Start sector number (LBA) */
				  BYTE count		  /* Sector count (1..255) */
				  )
{
	if (!count) return RES_PARERR;
	if (msd_Stat & STA_NOINIT) return RES_NOTRDY;

	return SD_ReadMultiBlock(sector, (u8 *)buff, count) ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
#if _READONLY == 0
DRESULT msd_disk_write (
				   const BYTE *buff,   /* Pointer to the data to be written */
				   DWORD sector,	   /* Start sector number (LBA) */
				   BYTE count		   /* Sector count (1..255) */
				   )
{
	if (!count) return RES_PARERR;
	if (msd_Stat & STA_NOINIT) return RES_NOTRDY;
	if (msd_Stat & STA_PROTECT)	return RES_WRPRT;
#if 0
	while(count--)
	{
		if( SD_WriteSingleBlock(sector++, buff))
			break;
		buff+= 512;
	}
	return count?RES_ERROR:RES_OK;
#else
	return SD_WriteMultiBlock(sector, (u8 *)buff, count)? RES_ERROR : RES_OK;
#endif
}
#endif /* _READONLY == 0 */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL != 0
DRESULT msd_disk_ioctl (
				   BYTE ctrl,	   /* Control code */
				   void *buff	   /* Buffer to send/receive control data */
				   )
{
	DRESULT res;
	BYTE n, csd[16], *ptr = buff;
	//WORD csize;

	res = RES_ERROR;

	if (ctrl == CTRL_POWER)
	{
		switch (*ptr)
		{
		case 0:		/* Sub control code == 0 (POWER_OFF) */
			res = RES_OK;
			break;
		case 1:		/* Sub control code == 1 (POWER_ON) */
			res = RES_OK;
			break;
		case 2:		/* Sub control code == 2 (POWER_GET) */
			*(ptr+1) = (BYTE)1;//chk_power();
			res = RES_OK;
			break;
		default :
			res = RES_PARERR;
		}
	}
	else
	{
		if (msd_Stat & STA_NOINIT) return RES_NOTRDY;

		switch (ctrl)
		{
		case CTRL_SYNC :		/* Make sure that no pending write process */
			SD_CS_ENABLE();
			if (SD_WaitReady()== 0)
				res = RES_OK;
			break;

		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
			if ((*(DWORD*)buff = SD_GetCapacity())>0)
				res = RES_OK;
			break;

		case GET_SECTOR_SIZE :	/* Get R/W sector size (WORD) */
			*(WORD*)buff = 512;
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
			if(SD_GetCSD(csd)==0)
			{
				if (SD_Type>=SD_TYPE_V2)		  /* SDC ver 2.00 */
				{
					for (n = 64 - 16; n; n--) sd_ReadWriteByte(0xff);	/* Purge trailing data */
					*(DWORD*)buff = 16UL << (csd[10] >> 4);
				}else
				{
					if (SD_Type == SD_TYPE_V1)		  /* SDC ver 1.XX */
					{
						*(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
					}
					else					/* MMC */
					{
						*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
					}
				}
				res = RES_OK;
			}
			break;

		case MMC_GET_TYPE :		/* Get card type flags (1 byte) */
			*ptr = SD_Type;
			res = RES_OK;
			break;

		case MMC_GET_CSD :		/* Receive CSD as a data block (16 bytes) */
			if(SD_GetCSD((u8 *)buff)==0)
				res = RES_OK;
			break;

		case MMC_GET_CID :		/* Receive CID as a data block (16 bytes) */
			if (SD_GetCID((u8 *)buff)==0)
				res = RES_OK;
			break;
#if 0
		case MMC_GET_OCR :		/* Receive OCR as an R3 resp (4 bytes) */
			if (SD_SendCommand(CMD58, 0, 0xFF) == 0)  /* READ_OCR */
			{
				for (n = 4; n; n--)	*ptr++ = SPI_ReadWrite_Byte(0xff);
				res = RES_OK;
			}
			break;

		case MMC_GET_SDSTAT :	/* Receive SD statsu as a data block (64 bytes) */
			if (Send_Command(ACMD13, 0) == 0) /* SD_STATUS */
			{
				SPI_ReadWrite_Byte(0xff);
				if (Receive_DataBlock(ptr, 64))
					res = RES_OK;
			}
			break;
#endif
		default:
			res = RES_PARERR;
		}

	}

	SD_SPI_Release();
	return res;
}
#endif /* _USE_IOCTL != 0 */

DWORD get_fattime (void)
{
	return 0;  
}

