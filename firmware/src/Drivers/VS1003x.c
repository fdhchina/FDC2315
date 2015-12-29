#include "hal.h"
#include "vs1003.h"	 

// #include "spec_rew.h"

extern void SysTickDelay(unsigned short dly_ms);
//VS1003��ȫ���ܺ���
//֧��SIN���Ժ�RAM����
//��������VS1003��Ƶ����ʾ���룬����˵ʵ����զ�أ��������Լ�д��Ƶ�׷����������ǲ�����ʵ��Ƶ�ױ任��  
//����ԭ��@SCUT
//V1.1
unsigned char vs1003_enter_flag=0;
// #define VS1003_DREQ		(GPIOD->IDR&MP3_DREQ)
//VS1003���ò���
//0,henh.1,hfreq.2,lenh.3,lfreq 5,������
unsigned char vs1003ram[5]={0,0,0,0,250};
/*
//����VS1003������
//EEPROM��ַ��486~490 �����
void Save_VS_Set(void)
{
	unsigned char t;
	for(t=0;t<5;t++)FM24C16_WriteOneByte(488+t,vs1003ram[t]);//vs1003ram���� 	 
}
//��ȡVS1003������
//EEPROM��ַ��486~490 �����
void Read_VS_Set(void)
{
	unsigned char t;
	for(t=0;t<5;t++)vs1003ram[t]=FM24C16_ReadOneByte(488+t);//vs1003ram����
}
*/	 					 
#if VS1003_INT_FLAG_EN
void vs1003_IntEnable(FunctionalState bEnable)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	// VS1003ʹ��EXTI10�ж�
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = bEnable;
	NVIC_Init(&NVIC_InitStructure);
}
#endif
//SPI2�ڶ�дһ���ֽ�
//TxData:Ҫ���͵��ֽ�
//����ֵ:��ȡ�����ֽ�
unsigned char SPI2_ReadWriteByte(unsigned char TxData)
{
	while((SPI2->SR&1<<1)==0);//�ȴ���������				  
	SPI2->DR=TxData;	 	  //����һ��byte   
	while((SPI2->SR&1<<0)==0);//�ȴ�������һ��byte  
	return SPI2->DR;          //�����յ�������				    
} 
//����SPI2���ٶ�
//SpeedSet:1,����;0,����;
void SPI2_SetSpeed(unsigned char SpeedSet)
{
	SPI2->CR1&=0XFFC7;
	if(SpeedSet==1)//����
	{
		SPI2->CR1|=1<<3;//Fsck=Fpclk/4=18Mhz	
	}else//����
	{
		SPI2->CR1|=5<<3; //Fsck=Fpclk/64=1.125Mhz
	}
	SPI2->CR1|=1<<6; //SPI�豸ʹ��	  
} 
//��λVS1003
void vs1003_SoftReset(void)
{	 
	//unsigned char retry; 				   
//	if(touch_enter_flag==TRUE) return;
//	vs1003_enter_flag= TRUE;
	//while(VS1003_DREQ==0);//�ȴ������λ����
	SPI2_ReadWriteByte(0x00);//��������
	//retry=0;
	//while(vs1003_REGRead(SPI_MODE)!=0x0804)// �����λ,��ģʽ  
	//{
		vs1003_CMDWrite(SPI_MODE,0x0804);// �����λ,��ģʽ
		SysTickDelay(2); //�ȴ�����1.35ms 
	//	if(retry++>100)break; 
	//}	 				  
	while (VS1003_DREQ == 0);//�ȴ������λ����	   

	//retry=0;
	//while(vs1003_REGRead(SPI_CLOCKF)!=0X9800)//����vs1003��ʱ��,3��Ƶ ,1.5xADD 
	//{
		vs1003_CMDWrite(SPI_CLOCKF,0X9800);//����vs1003��ʱ��,3��Ƶ ,1.5xADD
	//	if(retry++>100)break; 
	//}		   
	//retry=0;
	//while(vs1003_REGRead(SPI_AUDATA)!=0XBB81)//����vs1003��ʱ��,3��Ƶ ,1.5xADD 
	//{
		vs1003_CMDWrite(SPI_AUDATA,0XBB81);
	//	if(retry++>100)break; 
	//}

	vs1003_CMDWrite(SPI_BASS, 0x0055);		//��������
	//vs1003_CMDWrite(SPI_CLOCKF,0X9800); 	    
	//vs1003_CMDWrite(SPI_AUDATA,0XBB81); //������48k��������	 
	// set1003();//����VS1003����Ч				 
	vs1003_ResetDecodeTime();//��λ����ʱ��	    
    //��vs1003����4���ֽ���Ч���ݣ���������SPI����
    MP3_DCS_SET(0);//ѡ�����ݴ���
	SPI2_ReadWriteByte(0XFF);
	SPI2_ReadWriteByte(0XFF);
	SPI2_ReadWriteByte(0XFF);
	SPI2_ReadWriteByte(0XFF);
	MP3_DCS_SET(1);//ȡ�����ݴ���
	vs1003_enter_flag= FALSE;
	SysTickDelay(20);
} 
//Ӳ��λMP3
void vs1003_Reset(void)
{
	//if(touch_enter_flag==TRUE) return;
	vs1003_enter_flag= TRUE;
	MP3_RST_SET(0);
	SysTickDelay(20);
	MP3_DCS_SET(1);//ȡ�����ݴ���
	MP3_CCS_SET(1);//ȡ�����ݴ���
	MP3_RST_SET(1);    
	while(VS1003_DREQ==0);	//�ȴ�DREQΪ��
	SysTickDelay(20);				 
	vs1003_enter_flag= FALSE;
}
//���Ҳ��� 
void vs1003_SineTest(void)
{											    
	//if(touch_enter_flag==TRUE) return;
	vs1003_enter_flag= TRUE;
	vs1003_Reset();	 
	vs1003_CMDWrite(0x0b,0X2020);	  //��������	 
 	vs1003_CMDWrite(SPI_MODE,0x0820);//����vs1003�Ĳ���ģʽ	    
	while (VS1003_DREQ == 0);     //�ȴ�DREQΪ��
 	//��vs1003�������Ҳ������0x53 0xef 0x6e n 0x00 0x00 0x00 0x00
 	//����n = 0x24, �趨vs1003�����������Ҳ���Ƶ��ֵ��������㷽����vs1003��datasheet
    MP3_DCS_SET(0);//ѡ�����ݴ���
	SPI2_ReadWriteByte(0x53);
	SPI2_ReadWriteByte(0xef);
	SPI2_ReadWriteByte(0x6e);
	SPI2_ReadWriteByte(0x24);
	SPI2_ReadWriteByte(0x00);
	SPI2_ReadWriteByte(0x00);
	SPI2_ReadWriteByte(0x00);
	SPI2_ReadWriteByte(0x00);
	SysTickDelay(100);
	MP3_DCS_SET(1); 
    //�˳����Ҳ���
    MP3_DCS_SET(0);//ѡ�����ݴ���
	SPI2_ReadWriteByte(0x45);
	SPI2_ReadWriteByte(0x78);
	SPI2_ReadWriteByte(0x69);
	SPI2_ReadWriteByte(0x74);
	SPI2_ReadWriteByte(0x00);
	SPI2_ReadWriteByte(0x00);
	SPI2_ReadWriteByte(0x00);
	SPI2_ReadWriteByte(0x00);
	SysTickDelay(100);
	MP3_DCS_SET(1);		 

    //�ٴν������Ҳ��Բ�����nֵΪ0x44���������Ҳ���Ƶ������Ϊ�����ֵ
    MP3_DCS_SET(0);//ѡ�����ݴ���      
	SPI2_ReadWriteByte(0x53);
	SPI2_ReadWriteByte(0xef);
	SPI2_ReadWriteByte(0x6e);
	SPI2_ReadWriteByte(0x44);
	SPI2_ReadWriteByte(0x00);
	SPI2_ReadWriteByte(0x00);
	SPI2_ReadWriteByte(0x00);
	SPI2_ReadWriteByte(0x00);
	SysTickDelay(100);
	MP3_DCS_SET(1);
    //�˳����Ҳ���
    MP3_DCS_SET(0);//ѡ�����ݴ���
	SPI2_ReadWriteByte(0x45);
	SPI2_ReadWriteByte(0x78);
	SPI2_ReadWriteByte(0x69);
	SPI2_ReadWriteByte(0x74);
	SPI2_ReadWriteByte(0x00);
	SPI2_ReadWriteByte(0x00);
	SPI2_ReadWriteByte(0x00);
	SPI2_ReadWriteByte(0x00);
	SysTickDelay(100);
	MP3_DCS_SET(1);	 
	vs1003_enter_flag= FALSE;
}	 
//ram ���� 																				 
void vs1003_RamTest(void)
{
	//unsigned short regvalue ;	   
	//if(touch_enter_flag==TRUE) return;
	vs1003_enter_flag= TRUE;
	vs1003_Reset();     
 	vs1003_CMDWrite(SPI_MODE,0x0820);// ����vs1003�Ĳ���ģʽ
	while (VS1003_DREQ==0); // �ȴ�DREQΪ��
 	MP3_DCS_SET(0);	       			  // xDCS = 1��ѡ��vs1003�����ݽӿ�
	SPI2_ReadWriteByte(0x4d);
	SPI2_ReadWriteByte(0xea);
	SPI2_ReadWriteByte(0x6d);
	SPI2_ReadWriteByte(0x54);
	SPI2_ReadWriteByte(0x00);
	SPI2_ReadWriteByte(0x00);
	SPI2_ReadWriteByte(0x00);
	SPI2_ReadWriteByte(0x00);
	SysTickDelay(50);  
	MP3_DCS_SET(1);
	/*regvalue=*/vs1003_REGRead(SPI_HDAT0); // ����õ���ֵΪ0x807F���������á�
	//printf("regvalueH:%x\n",regvalue>>8);//������ 
	//printf("regvalueL:%x\n",regvalue&0xff);//������ 
	vs1003_enter_flag= FALSE;
}     
//��VS1003д����
//address:�����ַ
//data:��������
void vs1003_CMDWrite(unsigned char address,unsigned short data)
{  
	//if(touch_enter_flag==TRUE) return;
	vs1003_enter_flag= TRUE;
    	while(VS1003_DREQ==0);//�ȴ�����
	SPI2_SetSpeed(0);//���� 
	 
	MP3_DCS_SET(1); //MP3_DATA_CS=1;
	MP3_CCS_SET(0); //MP3_CMD_CS=0; 
	
	SPI2_ReadWriteByte(VS_WRITE_COMMAND);//����VS1003��д����
	SPI2_ReadWriteByte(address); //��ַ
	SPI2_ReadWriteByte(data>>8); //���͸߰�λ
	SPI2_ReadWriteByte(data);	 //�ڰ�λ
	MP3_CCS_SET(1);          //MP3_CMD_CS=1; 
	SPI2_SetSpeed(1);//����
	vs1003_enter_flag= FALSE;
} 
//��VS1003д����
void vs1003_DATAWrite(unsigned char data)
{
	//if(touch_enter_flag==TRUE) return;
	vs1003_enter_flag= TRUE;
	MP3_DCS_SET(0);   //MP3_DATA_CS=0;
	SPI2_ReadWriteByte(data);
	MP3_DCS_SET(1);   //MP3_DATA_CS=1;
	MP3_CCS_SET(1);   //MP3_CMD_CS=1; 
	vs1003_enter_flag= FALSE;
}         

/**********************************************************/
/*  �������� :  vs1003_WriteData                                */
/*  �������� �� ͨ��SPI����len���ֽڵ�����                 */
/*  ����     :  �����͵��ֽ�����                          */
/*  ����ֵ   :  ��                                        */
/*--------------------------------------------------------*/
void vs1003_WriteData(unsigned char *buf,int len)
{
	MP3_DCS_SET(0);

	while(len--)
		SPI2_ReadWriteByte(*buf++);

	MP3_DCS_SET(1);
}

//��VS1003д����
void vs1003_32byteWrite(unsigned char *data)
{
	unsigned char* i=data+32;
	//if(touch_enter_flag==TRUE) return;
	vs1003_enter_flag= TRUE;
	MP3_DCS_SET(0);   //MP3_DATA_CS=0;
	while(data<i) 
		SPI2_ReadWriteByte(*data++);
	MP3_DCS_SET(1);   //MP3_DATA_CS=1;
	MP3_CCS_SET(1);   //MP3_CMD_CS=1; 
	vs1003_enter_flag= FALSE;
}         
//��VS1003�ļĴ���           
//��VS1003
//ע�ⲻҪ�ñ��ٶ�ȡ,�����
unsigned short vs1003_REGRead(unsigned char address)
{ 
	unsigned short temp=0; 
	//if(touch_enter_flag==TRUE) return 0;
	vs1003_enter_flag= TRUE;
	while(VS1003_DREQ==0);//�ǵȴ�����״̬ 
	SPI2_SetSpeed(0);//���� 
	MP3_DCS_SET(1);       //MP3_DATA_CS=1;
	MP3_CCS_SET(0);       //MP3_CMD_CS=0;
	SPI2_ReadWriteByte(VS_READ_COMMAND);//����VS1003�Ķ�����
	SPI2_ReadWriteByte(address);        //��ַ
	temp=SPI2_ReadWriteByte(0xff);		//��ȡ���ֽ�
	temp=temp<<8;
	temp+=SPI2_ReadWriteByte(0xff); 	//��ȡ���ֽ�
	MP3_CCS_SET(1);      //MP3_CMD_CS=1; 
	SPI2_SetSpeed(1);//����
	vs1003_enter_flag= FALSE;
    	return temp; 
}  

// *************************************************************************************
// ��������2048��0��vs1003 ���������
void vs1003_ClearDataBuf()
{
	u16	n;
	unsigned char i,j;
	for(i=0;i<64;i++)
	{
	    	MP3_DCS_SET(0);
		n=65535;
		while ((VS1003_DREQ == 0)&&(n--)); //�ȴ������λ����
		for(j=0;j<32;j++)
		{
			SPI2_ReadWriteByte(0x00);
		}
		MP3_DCS_SET(1);
	}
}
// *************************************************************************************

// ***************************************************************************************************
// �ر�ģ�ⲿ�ֵ�Դ
void vs1003_AnalogPwrdown(BOOL pwrdown)
{
	static u16 mp3_oldvol= 0;  // Ĭ��Ϊ�������
	u16	tmp;
	
	if(pwrdown==TRUE)
	{
		tmp=vs1003_REGRead(SPI_VOL);
		if(tmp==0xFFFF) return;
		mp3_oldvol= tmp;
		vs1003_CMDWrite(SPI_VOL, 0xFFFF);
	}else
		 vs1003_CMDWrite(SPI_VOL, mp3_oldvol);
}
// ***************************************************************************************************

//FOR WAV HEAD0 :0X7761 HEAD1:0X7665    
//FOR MIDI HEAD0 :other info HEAD1:0X4D54
//FOR WMA HEAD0 :data speed HEAD1:0X574D
//FOR MP3 HEAD0 :data speed HEAD1:ID
//������Ԥ��ֵ
const unsigned short bitrate[2][16]=
{ 
	{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0}, 
	{0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0}
};
//����Kbps�Ĵ�С
//�õ�mp3&wma�Ĳ�����
unsigned short vs1003_GetHeadInfo(void)
{
	unsigned int HEAD0;
	unsigned int HEAD1;            
	HEAD0=vs1003_REGRead(SPI_HDAT0); 
	HEAD1=vs1003_REGRead(SPI_HDAT1);
	switch(HEAD1)
	{        
	case 0x7665:return 0;//WAV��ʽ
	case 0X4D54:return 1;//MIDI��ʽ 
	case 0X574D://WMA��ʽ
		{
		HEAD1=HEAD0*2/25;
		if((HEAD1%10)>5)
			return HEAD1/10+1;
		else 
			return HEAD1/10;
        	}
	default://MP3��ʽ
		{
		HEAD1>>=3;
		HEAD1=HEAD1&0x03; 
		if(HEAD1==3)
			HEAD1=1;
		else 
			HEAD1=0;
		return bitrate[HEAD1][HEAD0>>12];
		}
	} 
}  
//�������ʱ��                          
void vs1003_ResetDecodeTime(void)
{
	vs1003_CMDWrite(SPI_DECODE_TIME,0x0000);
	vs1003_CMDWrite(SPI_DECODE_TIME,0x0000);//��������
}
//�õ�mp3�Ĳ���ʱ��n sec
unsigned short vs1003_GetDecodeTime(void)
{ 
    return vs1003_REGRead(SPI_DECODE_TIME);   
} 

u16 vs1003_GetVolume()
{
	return vs1003_REGRead(SPI_VOL);
}

void vs1003_SetVolume(u16 wVol)
{
	vs1003_CMDWrite(SPI_VOL, wVol);
}
u16 vs1003_GetBass()
{
	return vs1003_REGRead(SPI_BASS);
}

void vs1003_SetBass(u16 wBass)
{
	vs1003_CMDWrite(SPI_BASS, wBass);
}
//����Ƶ�׷����Ĵ��뵽VS1003
/*
void LoadPatch(void)
{
	unsigned short i;
	for (i=0;i<943;i++)Vs1003_CMD_Write(atab[i],dtab[i]); 
	SysTickDelay(10);
}
//�õ�Ƶ������
void GetSpec(unsigned char *p)
{
	unsigned char byteIndex=0;
	unsigned char temp;
	Vs1003_CMD_Write(SPI_WRAMADDR,0x1804);                                                                                             
	for (byteIndex=0;byteIndex<14;byteIndex++) 
	{                                                                               
		temp=Vs1003_REG_Read(SPI_WRAM)&0x63;//ȡС��100����    
		*p++=temp;
	} 
}
*/

void SPI2_RST(void)
{
	RCC->APB2RSTR|=1<<12;//��λSPI2
	SysTickDelay(10); 	
	RCC->APB2RSTR&=~(1<<12);//������λSPI2
	SysTickDelay(10); 
	SPI2->CR1|=0<<10;//ȫ˫��ģʽ	
	SPI2->CR1|=1<<9; //���nss����
	SPI2->CR1|=1<<8;  

	SPI2->CR1|=1<<2; //SPI����
	SPI2->CR1|=0<<11;//8bit���ݸ�ʽ	
	SPI2->CR1|=1<<1; //����ģʽ��SCKΪ1 CPOL=1
	SPI2->CR1|=1<<0; //���ݲ����ӵڶ���ʱ����ؿ�ʼ,CPHA=1  
	SPI2->CR1|=6<<3; //Fsck=Fpclk/128 =562.5khz
	SPI2->CR1|=0<<7; //MSBfirst 
	
	SPI2->CR1|=1<<6; //SPI�豸ʹ��
}	  
//�趨vs1003���ŵ������͸ߵ��� 
void set1003(void)
{
	unsigned char t;
	unsigned short bass=0; //�ݴ������Ĵ���ֵ
	unsigned short volt=0; //�ݴ�����ֵ
	unsigned char vset=0;  //�ݴ�����ֵ 	 
	vset=255-vs1003ram[4];//ȡ��һ��,�õ����ֵ,��ʾ���ı�ʾ 
	volt=vset;
	volt<<=8;
	volt+=vset;//�õ��������ú��С
	//0,henh.1,hfreq.2,lenh.3,lfreq        
	for(t=0;t<4;t++)
	{
		bass<<=4;
		bass+=vs1003ram[t]; 
	}     
	vs1003_CMDWrite(SPI_BASS,bass);//BASS   
	vs1003_CMDWrite(SPI_VOL,volt); //������ 
}    

//��ʼ��VS1003��IO��	 
void vs1003_Init()
{
	// SPI_InitTypeDef  SPI_InitStructure;
	// GPIO_InitTypeDef GPIO_InitStructure;
#if VS1003_INT_FLAG_EN	
	// EXTI_InitTypeDef EXTI_InitStructure;
#endif
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOE, ENABLE);
	// RCC->APB2ENR|=1<<12;      //SPI2ʱ��ʹ��	 
#if (GC_ZR&GC_ZR_GPIO)  
	GPIO_SetBits(GPIOB, GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_12);//Ԥ��Ϊ��
	GPIO_SetBits(GPIOE, GPIO_Pin_7);//Ԥ��Ϊ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//����
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//50Mʱ���ٶ�
	GPIO_Init(GPIOB, &GPIO_InitStructure); 

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		//��������
	GPIO_Init(GPIOE, &GPIO_InitStructure);
#else
	GPIO_SetBits(GPIOB, GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12);//Ԥ��Ϊ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_11|GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//����
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//50Mʱ���ٶ�
	GPIO_Init(GPIOB, &GPIO_InitStructure); 
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		//��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);

	/* PB15-MOSI2,PB13-SCK2*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 |GPIO_Pin_14 | GPIO_Pin_15;
    	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    	GPIO_Init(GPIOB, &GPIO_InitStructure);

    	/* SPI2 configuration */
	SPI_Cmd(SPI2, DISABLE); 												//�����Ƚ���,���ܸı�MODE
    	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;		//����ȫ˫��
    	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;							//��
    	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;						// 8λ
    	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;								//CPOL=1 ʱ�����ո�
    	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;							//CPHA=1 ���ݲ����2��
    	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;								//���NSS
    	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;		// 2��Ƶ
    	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;						//��λ��ǰ
    	SPI_InitStructure.SPI_CRCPolynomial = 7;								//CRC7
    
	SPI_Init(SPI2, &SPI_InitStructure);
	//SPI_SSOutputCmd(SPI2, ENABLE); //ʹ��NSS�ſ���
    	SPI_Cmd(SPI2, ENABLE); 

#if VS1003_INT_FLAG_EN	

	/* ����VS1003_DREQ �ӵ�GPD2������Ϊ�ж��� */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10);
	/*����Ϊ�½��ش���*/
	EXTI_InitStructure.EXTI_Line = EXTI_Line10;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	vs1003_IntEnable(DISABLE);
#endif

#if 0
// ************************************************************
//  �������� :  RecorInit                                    
//  �������� �� VS1033¼����ʼ��                             
//  ����     :  ��                      				     
//  ����ֵ   :  ��                                           
// ��ʼ����ADPCM¼��֮ǰ���û�Ӧ��дһ��ʱ�ӷ�Ƶֵ��SCI_AICTRL0������Ƶ�ʵļ��㹫ʽ���£�Fs = Fc / 256 * d��
// Fc���ڲ���ʱ��Ƶ�ʣ�CLKZ����d��д��SCI_AICTRL0�ķ�Ƶֵ��d��С����ֵΪ4�����SCI_AICTRL0Ϊ0����ʹ��Ĭ��ֵ12.
// ���磺Fc = 2.0 * 12.288MHZ��d = 12����Fs= ��2.0 * 12288000�� / ��256 * 12�� = 8000HZ
// -----------------------------------------------------------
#define CALCSAMPLE(x)	((u16)((3*12288000)/ 256 / x))
void vs1003_RecordInit(u32 isline, u32 SampleFrequency)
{
	unsigned int i;
	unsigned short tmp=0x1800 | 0x2000;
	if(isline)tmp|=0x4000;

	// Mp3AnalogPwrdown(FALSE); 
	vs1003_CMDWrite(SPI_CLOCKF, 0x9800);//����vs1003��ʱ��,4.5��Ƶ  0x98Ϊ3��Ƶ
	vs1003_CMDWrite(SPI_AICTRL0, CALCSAMPLE(SampleFrequency)); //������8k
	//vs1003_CMDWrite(SPI_AICTRL0, 0x0012); //������8k
	//Mp3WriteRegister (SPI_AICTRL0, 0x0009); //������16k

	// vs1003_CMDWrite(SPI_AUDATA, 0x1f40); //������8k��������
	
	vs1003_CMDWrite(SPI_AICTRL1, 0x0); // AutoGain On
	//vs1003_CMDWrite(SPI_AICTRL1, 0x1000); // AutoGain OFF, reclevel 0x1000
	vs1003_CMDWrite(SPI_MODE, tmp | 0x04);		//��λVS1003

	i=655350;
	while ((VS1003_DREQ == 0)&&(i--)); //�ȴ������λ����
	vs1003_CMDWrite(SPI_CLOCKF, 0x9800);//����vs1003��ʱ��,3��Ƶ

   	SysTickDelay(1); //��ʱ1ms
    	
}
#endif
// ***********************************************************************************

}


