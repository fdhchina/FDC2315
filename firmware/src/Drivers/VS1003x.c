#include "hal.h"
#include "vs1003.h"	 

// #include "spec_rew.h"

extern void SysTickDelay(unsigned short dly_ms);
//VS1003的全功能函数
//支持SIN测试和RAM测试
//并加入了VS1003的频谱显示代码，不过说实话不咋地，还不如自己写的频谱分析，怀疑是不是真实的频谱变换？  
//正点原子@SCUT
//V1.1
unsigned char vs1003_enter_flag=0;
// #define VS1003_DREQ		(GPIOD->IDR&MP3_DREQ)
//VS1003设置参数
//0,henh.1,hfreq.2,lenh.3,lfreq 5,主音量
unsigned char vs1003ram[5]={0,0,0,0,250};
/*
//保存VS1003的设置
//EEPROM地址：486~490 共五个
void Save_VS_Set(void)
{
	unsigned char t;
	for(t=0;t<5;t++)FM24C16_WriteOneByte(488+t,vs1003ram[t]);//vs1003ram保存 	 
}
//读取VS1003的设置
//EEPROM地址：486~490 共五个
void Read_VS_Set(void)
{
	unsigned char t;
	for(t=0;t<5;t++)vs1003ram[t]=FM24C16_ReadOneByte(488+t);//vs1003ram调用
}
*/	 					 
#if VS1003_INT_FLAG_EN
void vs1003_IntEnable(FunctionalState bEnable)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	// VS1003使用EXTI10中断
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = bEnable;
	NVIC_Init(&NVIC_InitStructure);
}
#endif
//SPI2口读写一个字节
//TxData:要发送的字节
//返回值:读取到的字节
unsigned char SPI2_ReadWriteByte(unsigned char TxData)
{
	while((SPI2->SR&1<<1)==0);//等待发送区空				  
	SPI2->DR=TxData;	 	  //发送一个byte   
	while((SPI2->SR&1<<0)==0);//等待接收完一个byte  
	return SPI2->DR;          //返回收到的数据				    
} 
//设置SPI2的速度
//SpeedSet:1,高速;0,低速;
void SPI2_SetSpeed(unsigned char SpeedSet)
{
	SPI2->CR1&=0XFFC7;
	if(SpeedSet==1)//高速
	{
		SPI2->CR1|=1<<3;//Fsck=Fpclk/4=18Mhz	
	}else//低速
	{
		SPI2->CR1|=5<<3; //Fsck=Fpclk/64=1.125Mhz
	}
	SPI2->CR1|=1<<6; //SPI设备使能	  
} 
//软复位VS1003
void vs1003_SoftReset(void)
{	 
	//unsigned char retry; 				   
//	if(touch_enter_flag==TRUE) return;
//	vs1003_enter_flag= TRUE;
	//while(VS1003_DREQ==0);//等待软件复位结束
	SPI2_ReadWriteByte(0x00);//启动传输
	//retry=0;
	//while(vs1003_REGRead(SPI_MODE)!=0x0804)// 软件复位,新模式  
	//{
		vs1003_CMDWrite(SPI_MODE,0x0804);// 软件复位,新模式
		SysTickDelay(2); //等待至少1.35ms 
	//	if(retry++>100)break; 
	//}	 				  
	while (VS1003_DREQ == 0);//等待软件复位结束	   

	//retry=0;
	//while(vs1003_REGRead(SPI_CLOCKF)!=0X9800)//设置vs1003的时钟,3倍频 ,1.5xADD 
	//{
		vs1003_CMDWrite(SPI_CLOCKF,0X9800);//设置vs1003的时钟,3倍频 ,1.5xADD
	//	if(retry++>100)break; 
	//}		   
	//retry=0;
	//while(vs1003_REGRead(SPI_AUDATA)!=0XBB81)//设置vs1003的时钟,3倍频 ,1.5xADD 
	//{
		vs1003_CMDWrite(SPI_AUDATA,0XBB81);
	//	if(retry++>100)break; 
	//}

	vs1003_CMDWrite(SPI_BASS, 0x0055);		//设置重音
	//vs1003_CMDWrite(SPI_CLOCKF,0X9800); 	    
	//vs1003_CMDWrite(SPI_AUDATA,0XBB81); //采样率48k，立体声	 
	// set1003();//设置VS1003的音效				 
	vs1003_ResetDecodeTime();//复位解码时间	    
    //向vs1003发送4个字节无效数据，用以启动SPI发送
    MP3_DCS_SET(0);//选中数据传输
	SPI2_ReadWriteByte(0XFF);
	SPI2_ReadWriteByte(0XFF);
	SPI2_ReadWriteByte(0XFF);
	SPI2_ReadWriteByte(0XFF);
	MP3_DCS_SET(1);//取消数据传输
	vs1003_enter_flag= FALSE;
	SysTickDelay(20);
} 
//硬复位MP3
void vs1003_Reset(void)
{
	//if(touch_enter_flag==TRUE) return;
	vs1003_enter_flag= TRUE;
	MP3_RST_SET(0);
	SysTickDelay(20);
	MP3_DCS_SET(1);//取消数据传输
	MP3_CCS_SET(1);//取消数据传输
	MP3_RST_SET(1);    
	while(VS1003_DREQ==0);	//等待DREQ为高
	SysTickDelay(20);				 
	vs1003_enter_flag= FALSE;
}
//正弦测试 
void vs1003_SineTest(void)
{											    
	//if(touch_enter_flag==TRUE) return;
	vs1003_enter_flag= TRUE;
	vs1003_Reset();	 
	vs1003_CMDWrite(0x0b,0X2020);	  //设置音量	 
 	vs1003_CMDWrite(SPI_MODE,0x0820);//进入vs1003的测试模式	    
	while (VS1003_DREQ == 0);     //等待DREQ为高
 	//向vs1003发送正弦测试命令：0x53 0xef 0x6e n 0x00 0x00 0x00 0x00
 	//其中n = 0x24, 设定vs1003所产生的正弦波的频率值，具体计算方法见vs1003的datasheet
    MP3_DCS_SET(0);//选中数据传输
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
    //退出正弦测试
    MP3_DCS_SET(0);//选中数据传输
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

    //再次进入正弦测试并设置n值为0x44，即将正弦波的频率设置为另外的值
    MP3_DCS_SET(0);//选中数据传输      
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
    //退出正弦测试
    MP3_DCS_SET(0);//选中数据传输
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
//ram 测试 																				 
void vs1003_RamTest(void)
{
	//unsigned short regvalue ;	   
	//if(touch_enter_flag==TRUE) return;
	vs1003_enter_flag= TRUE;
	vs1003_Reset();     
 	vs1003_CMDWrite(SPI_MODE,0x0820);// 进入vs1003的测试模式
	while (VS1003_DREQ==0); // 等待DREQ为高
 	MP3_DCS_SET(0);	       			  // xDCS = 1，选择vs1003的数据接口
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
	/*regvalue=*/vs1003_REGRead(SPI_HDAT0); // 如果得到的值为0x807F，则表明完好。
	//printf("regvalueH:%x\n",regvalue>>8);//输出结果 
	//printf("regvalueL:%x\n",regvalue&0xff);//输出结果 
	vs1003_enter_flag= FALSE;
}     
//向VS1003写命令
//address:命令地址
//data:命令数据
void vs1003_CMDWrite(unsigned char address,unsigned short data)
{  
	//if(touch_enter_flag==TRUE) return;
	vs1003_enter_flag= TRUE;
    	while(VS1003_DREQ==0);//等待空闲
	SPI2_SetSpeed(0);//低速 
	 
	MP3_DCS_SET(1); //MP3_DATA_CS=1;
	MP3_CCS_SET(0); //MP3_CMD_CS=0; 
	
	SPI2_ReadWriteByte(VS_WRITE_COMMAND);//发送VS1003的写命令
	SPI2_ReadWriteByte(address); //地址
	SPI2_ReadWriteByte(data>>8); //发送高八位
	SPI2_ReadWriteByte(data);	 //第八位
	MP3_CCS_SET(1);          //MP3_CMD_CS=1; 
	SPI2_SetSpeed(1);//高速
	vs1003_enter_flag= FALSE;
} 
//向VS1003写数据
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
/*  函数名称 :  vs1003_WriteData                                */
/*  函数功能 ： 通过SPI发送len个字节的数据                 */
/*  参数     :  待发送的字节数据                          */
/*  返回值   :  无                                        */
/*--------------------------------------------------------*/
void vs1003_WriteData(unsigned char *buf,int len)
{
	MP3_DCS_SET(0);

	while(len--)
		SPI2_ReadWriteByte(*buf++);

	MP3_DCS_SET(1);
}

//向VS1003写数据
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
//读VS1003的寄存器           
//读VS1003
//注意不要用倍速读取,会出错
unsigned short vs1003_REGRead(unsigned char address)
{ 
	unsigned short temp=0; 
	//if(touch_enter_flag==TRUE) return 0;
	vs1003_enter_flag= TRUE;
	while(VS1003_DREQ==0);//非等待空闲状态 
	SPI2_SetSpeed(0);//低速 
	MP3_DCS_SET(1);       //MP3_DATA_CS=1;
	MP3_CCS_SET(0);       //MP3_CMD_CS=0;
	SPI2_ReadWriteByte(VS_READ_COMMAND);//发送VS1003的读命令
	SPI2_ReadWriteByte(address);        //地址
	temp=SPI2_ReadWriteByte(0xff);		//读取高字节
	temp=temp<<8;
	temp+=SPI2_ReadWriteByte(0xff); 	//读取低字节
	MP3_CCS_SET(1);      //MP3_CMD_CS=1; 
	SPI2_SetSpeed(1);//高速
	vs1003_enter_flag= FALSE;
    	return temp; 
}  

// *************************************************************************************
// 连续发送2048个0到vs1003 以清除缓冲
void vs1003_ClearDataBuf()
{
	u16	n;
	unsigned char i,j;
	for(i=0;i<64;i++)
	{
	    	MP3_DCS_SET(0);
		n=65535;
		while ((VS1003_DREQ == 0)&&(n--)); //等待软件复位结束
		for(j=0;j<32;j++)
		{
			SPI2_ReadWriteByte(0x00);
		}
		MP3_DCS_SET(1);
	}
}
// *************************************************************************************

// ***************************************************************************************************
// 关闭模拟部分电源
void vs1003_AnalogPwrdown(BOOL pwrdown)
{
	static u16 mp3_oldvol= 0;  // 默认为最大音量
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
//比特率预定值
const unsigned short bitrate[2][16]=
{ 
	{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0}, 
	{0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0}
};
//返回Kbps的大小
//得到mp3&wma的波特率
unsigned short vs1003_GetHeadInfo(void)
{
	unsigned int HEAD0;
	unsigned int HEAD1;            
	HEAD0=vs1003_REGRead(SPI_HDAT0); 
	HEAD1=vs1003_REGRead(SPI_HDAT1);
	switch(HEAD1)
	{        
	case 0x7665:return 0;//WAV格式
	case 0X4D54:return 1;//MIDI格式 
	case 0X574D://WMA格式
		{
		HEAD1=HEAD0*2/25;
		if((HEAD1%10)>5)
			return HEAD1/10+1;
		else 
			return HEAD1/10;
        	}
	default://MP3格式
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
//重设解码时间                          
void vs1003_ResetDecodeTime(void)
{
	vs1003_CMDWrite(SPI_DECODE_TIME,0x0000);
	vs1003_CMDWrite(SPI_DECODE_TIME,0x0000);//操作两次
}
//得到mp3的播放时间n sec
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
//加载频谱分析的代码到VS1003
/*
void LoadPatch(void)
{
	unsigned short i;
	for (i=0;i<943;i++)Vs1003_CMD_Write(atab[i],dtab[i]); 
	SysTickDelay(10);
}
//得到频谱数据
void GetSpec(unsigned char *p)
{
	unsigned char byteIndex=0;
	unsigned char temp;
	Vs1003_CMD_Write(SPI_WRAMADDR,0x1804);                                                                                             
	for (byteIndex=0;byteIndex<14;byteIndex++) 
	{                                                                               
		temp=Vs1003_REG_Read(SPI_WRAM)&0x63;//取小于100的数    
		*p++=temp;
	} 
}
*/

void SPI2_RST(void)
{
	RCC->APB2RSTR|=1<<12;//复位SPI2
	SysTickDelay(10); 	
	RCC->APB2RSTR&=~(1<<12);//结束复位SPI2
	SysTickDelay(10); 
	SPI2->CR1|=0<<10;//全双工模式	
	SPI2->CR1|=1<<9; //软件nss管理
	SPI2->CR1|=1<<8;  

	SPI2->CR1|=1<<2; //SPI主机
	SPI2->CR1|=0<<11;//8bit数据格式	
	SPI2->CR1|=1<<1; //空闲模式下SCK为1 CPOL=1
	SPI2->CR1|=1<<0; //数据采样从第二个时间边沿开始,CPHA=1  
	SPI2->CR1|=6<<3; //Fsck=Fpclk/128 =562.5khz
	SPI2->CR1|=0<<7; //MSBfirst 
	
	SPI2->CR1|=1<<6; //SPI设备使能
}	  
//设定vs1003播放的音量和高低音 
void set1003(void)
{
	unsigned char t;
	unsigned short bass=0; //暂存音调寄存器值
	unsigned short volt=0; //暂存音量值
	unsigned char vset=0;  //暂存音量值 	 
	vset=255-vs1003ram[4];//取反一下,得到最大值,表示最大的表示 
	volt=vset;
	volt<<=8;
	volt+=vset;//得到音量设置后大小
	//0,henh.1,hfreq.2,lenh.3,lfreq        
	for(t=0;t<4;t++)
	{
		bass<<=4;
		bass+=vs1003ram[t]; 
	}     
	vs1003_CMDWrite(SPI_BASS,bass);//BASS   
	vs1003_CMDWrite(SPI_VOL,volt); //设音量 
}    

//初始化VS1003的IO口	 
void vs1003_Init()
{
	// SPI_InitTypeDef  SPI_InitStructure;
	// GPIO_InitTypeDef GPIO_InitStructure;
#if VS1003_INT_FLAG_EN	
	// EXTI_InitTypeDef EXTI_InitStructure;
#endif
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOE, ENABLE);
	// RCC->APB2ENR|=1<<12;      //SPI2时钟使能	 
#if (GC_ZR&GC_ZR_GPIO)  
	GPIO_SetBits(GPIOB, GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_12);//预置为高
	GPIO_SetBits(GPIOE, GPIO_Pin_7);//预置为高
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//推挽
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//50M时钟速度
	GPIO_Init(GPIOB, &GPIO_InitStructure); 

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		//上拉输入
	GPIO_Init(GPIOE, &GPIO_InitStructure);
#else
	GPIO_SetBits(GPIOB, GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12);//预置为高
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_11|GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//推挽
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//50M时钟速度
	GPIO_Init(GPIOB, &GPIO_InitStructure); 
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		//上拉输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);

	/* PB15-MOSI2,PB13-SCK2*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 |GPIO_Pin_14 | GPIO_Pin_15;
    	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    	GPIO_Init(GPIOB, &GPIO_InitStructure);

    	/* SPI2 configuration */
	SPI_Cmd(SPI2, DISABLE); 												//必须先禁能,才能改变MODE
    	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;		//两线全双工
    	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;							//主
    	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;						// 8位
    	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;								//CPOL=1 时钟悬空高
    	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;							//CPHA=1 数据捕获第2个
    	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;								//软件NSS
    	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;		// 2分频
    	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;						//高位在前
    	SPI_InitStructure.SPI_CRCPolynomial = 7;								//CRC7
    
	SPI_Init(SPI2, &SPI_InitStructure);
	//SPI_SSOutputCmd(SPI2, ENABLE); //使能NSS脚可用
    	SPI_Cmd(SPI2, ENABLE); 

#if VS1003_INT_FLAG_EN	

	/* 连接VS1003_DREQ 接到GPD2口设置为中断线 */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10);
	/*配置为下降沿触发*/
	EXTI_InitStructure.EXTI_Line = EXTI_Line10;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	vs1003_IntEnable(DISABLE);
#endif

#if 0
// ************************************************************
//  函数名称 :  RecorInit                                    
//  函数功能 ： VS1033录音初始化                             
//  参数     :  无                      				     
//  返回值   :  无                                           
// 开始激活ADPCM录音之前，用户应该写一个时钟分频值到SCI_AICTRL0，采样频率的计算公式如下：Fs = Fc / 256 * d，
// Fc是内部的时钟频率（CLKZ），d是写入SCI_AICTRL0的分频值。d最小的数值为4，如果SCI_AICTRL0为0，则使用默认值12.
// 例如：Fc = 2.0 * 12.288MHZ，d = 12，则：Fs= （2.0 * 12288000） / （256 * 12） = 8000HZ
// -----------------------------------------------------------
#define CALCSAMPLE(x)	((u16)((3*12288000)/ 256 / x))
void vs1003_RecordInit(u32 isline, u32 SampleFrequency)
{
	unsigned int i;
	unsigned short tmp=0x1800 | 0x2000;
	if(isline)tmp|=0x4000;

	// Mp3AnalogPwrdown(FALSE); 
	vs1003_CMDWrite(SPI_CLOCKF, 0x9800);//设置vs1003的时钟,4.5倍频  0x98为3倍频
	vs1003_CMDWrite(SPI_AICTRL0, CALCSAMPLE(SampleFrequency)); //采样率8k
	//vs1003_CMDWrite(SPI_AICTRL0, 0x0012); //采样率8k
	//Mp3WriteRegister (SPI_AICTRL0, 0x0009); //采样率16k

	// vs1003_CMDWrite(SPI_AUDATA, 0x1f40); //采样率8k，单声道
	
	vs1003_CMDWrite(SPI_AICTRL1, 0x0); // AutoGain On
	//vs1003_CMDWrite(SPI_AICTRL1, 0x1000); // AutoGain OFF, reclevel 0x1000
	vs1003_CMDWrite(SPI_MODE, tmp | 0x04);		//软复位VS1003

	i=655350;
	while ((VS1003_DREQ == 0)&&(i--)); //等待软件复位结束
	vs1003_CMDWrite(SPI_CLOCKF, 0x9800);//设置vs1003的时钟,3倍频

   	SysTickDelay(1); //延时1ms
    	
}
#endif
// ***********************************************************************************

}


