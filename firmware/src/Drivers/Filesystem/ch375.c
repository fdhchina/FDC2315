/**************************************************************
模块名:ch375.c 
功能实现:CH375硬件底层的移植部分,另外还包括软件层的改进,由于CH375的库函数书写比较不够规范, 
 		故此模块负责把库函数重新封装成文件读写的通用格式.经封装后读写速度会有所下降.如果需要
 		较高速的操作,请自己去看原库函数
 		***一旦调用,则不应再使用原库的函数,否则会有数据错误
作者:电子白菜




版本号:V1.0 
版本说明:	1.CH375硬件驱动函数.基层IO指令需要有延迟,这里用了一个硬delay函数,可自己根据CPU的速度,降低函数速度,使系统更快
			2.读写部分使用通用格式,并且所有操作都以扇区为单位,故不适合51之流使用,中间使用了缓冲隔离,优点是增加通用性,缺点是降低了速度. 
		 	3.增加了创建目录函数  
日期:2009-4-10
***************************************************************/

#include "hal.h"
#include "my375lib.h"

bool usb_insert_flag=FALSE;
bool usb_disk_Isready=FALSE;

//这几个定义需要在375的头文件之前

#define LIB_CFG_INT_EN			CH375_INT_MODE
#if(GC_ZR&GC_ZR_GPIO) 
#define CH375_INT_WIRE (GPIOD->IDR & GPIO_Pin_10)
#else
#define CH375_INT_WIRE (GPIOE->IDR & GPIO_Pin_13)
#endif
#define FILE_DATA_BUF_LEN		MY_FILE_DATA_BUF_LEN
#define DISK_BASE_BUF_LEN		MY_DISK_BASE_BUF_LEN

u8  DISK_BASE_BUF[ DISK_BASE_BUF_LEN ];	/* 外部RAM的磁盘数据缓冲区,缓冲区长度为一个扇区的长度 */

u8  FILE_DATA_BUF[ FILE_DATA_BUF_LEN ];	/* 外部RAM的文件数据缓冲区,缓冲区长度不小于一次读写的数据长度 */

#define NO_DEFAULT_CH375_F_ENUM		1		/* 不用CH375FileEnumer*/
#define NO_DEFAULT_CH375_F_QUERY	1		/* 不用CH375FileQuery*/
#define NO_DEFAULT_DELAY_100US		1		/* 不用默认的延时100uS子程序 */
void xDelay100uS( void )
{
	delay_us(100);
}
u8 xReadCH375Cmd(void);


#include <string.h>
#include <stdio.h>

/**************************************************************
** 函数名:Ch375_Configuration
** 功能:硬件配置CH375
** 注意事项:
***************************************************************/
void CH375_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
#if(GC_ZR&GC_ZR_GPIO)
	// E
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD, ENABLE);

	//PE12-RST,PE9-WE, PE8-OE, PE10-A0 	//PE11-CS
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//50M时钟速度
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_11;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	//PE13-INT,
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//上拉输入
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	//E口因为使用了IO,所以不在这里配置
#else
	// E
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

	//PE12-RST,PE9-WE, PE8-OE, PE10-A0 	//PE11-CS
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//50M时钟速度
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	//PE13-INT,
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//上拉输入
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	//E口因为使用了IO,所以不在这里配置
#endif
}

#if(GC_ZR&GC_ZR_GPIO)
//虚拟总线宏定义-为了使操作速度加快,这里没有使用库
#define RST375_H	GPIOD->BSRR=GPIO_Pin_9
#define RST375_L    	GPIOD->BRR=GPIO_Pin_9

#define CS375_H		GPIOD->BSRR=GPIO_Pin_11
#define CS375_L		GPIOD->BRR=GPIO_Pin_11

#define ADD375_H	GPIOB->BSRR=GPIO_Pin_11
#define ADD375_L	GPIOB->BRR=GPIO_Pin_11

#define WR375_H		GPIOD->BSRR=GPIO_Pin_8
#define WR375_L		GPIOD->BRR=GPIO_Pin_8  

#define RD375_H		GPIOB->BSRR=GPIO_Pin_10
#define RD375_L		GPIOB->BRR=GPIO_Pin_10 
#else
//虚拟总线宏定义-为了使操作速度加快,这里没有使用库
#define RST375_H	GPIOE->BSRR=GPIO_Pin_12
#define RST375_L    	GPIOE->BRR=GPIO_Pin_12

#define CS375_H		GPIOE->BSRR=GPIO_Pin_11
#define CS375_L		GPIOE->BRR=GPIO_Pin_11

#define ADD375_H	GPIOE->BSRR=GPIO_Pin_10
#define ADD375_L	GPIOE->BRR=GPIO_Pin_10

#define WR375_H		GPIOE->BSRR=GPIO_Pin_9
#define WR375_L		GPIOE->BRR=GPIO_Pin_9  

#define RD375_H		GPIOE->BSRR=GPIO_Pin_8
#define RD375_L		GPIOE->BRR=GPIO_Pin_8  
#endif
//#define GET_INT		(GPIOE->IDR & GPIO_Pin_13)	/*获取INT脚状态*/
#if (GC_ZR&GC_ZR_GPIO)
/**************************************************************
以下两个宏定义用于改变IO口的输入输出状态,为了提高速度,这里采用直接改变寄存器的办法,具体应用根据 
使用的IO脚的不同而会有所差别,这里使用PE0-7作为DATA脚 ,故操作的寄存器为GPIOA->CRL
***************************************************************/
#define SET_DATA_OUT		GPIOE->CRH=0x33333333	/*推挽输出50MHZ速度?*/
#define SET_DATA_IN			GPIOE->CRH=0x44444444	/*浮空输入*/

//#define GET_DATA375()		(GPIOE->IDR&0xff00)
//#define SET_DATA375(dat)	GPIOE->BSRR=(((dat<<8)&0xff00)|(((~dat)<<24)&0xFF000000))//这个可用ODR寄存器
#define GET_DATA375()		((GPIOE->IDR>>8)&0xff)
#define SET_DATA375(dat)	GPIOE->BSRR=((dat<<8)|((~dat)<<24))	// 这个可用ODR寄存器
#else
/**************************************************************
以下两个宏定义用于改变IO口的输入输出状态,为了提高速度,这里采用直接改变寄存器的办法,具体应用根据 
使用的IO脚的不同而会有所差别,这里使用PE0-7作为DATA脚 ,故操作的寄存器为GPIOA->CRL
***************************************************************/
#define SET_DATA_OUT		GPIOE->CRL=0x33333333	/*推挽输出50MHZ速度?*/
#define SET_DATA_IN			GPIOE->CRL=0x44444444	/*浮空输入*/
	
#define GET_DATA375()		(GPIOE->IDR&0xff)
#define SET_DATA375(dat)	GPIOE->BSRR=(dat|(((~dat)<<16)&0x00FF0000))	//这个可用ODR寄存器
#endif
//硬延迟函数,可根据CPU速度逐步调整到最高速度的临界点
#if 1
void ch375_Delay(uint16_t t)
{
	u8 i;
	while(t--)
	{
		for(i=0;i<6;i++);
	}
}
#else
#define delay	delay_us
#endif

#if CH375_INT_MODE
void ch375_INTConfig()
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
	/* Enable the EXTI15_10 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

  	/* 连接IO口到中断线 */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource13);
	/*设置为下降沿触发*/
	EXTI_InitStructure.EXTI_Line = EXTI_Line13;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	EXTI_GenerateSWInterrupt(EXTI_Line13);
}
#endif
/***************************************
** 函数名:xWriteCH375Cmd 
** 功能 :写命令
** 注意事项: ADD=1为命令,锁存时间至少要有4US
***************************************/
void xWriteCH375Cmd(u8 cmd)
{
	//延迟2US
	ch375_Delay(1);
	SET_DATA_OUT;
	ADD375_H;
	WR375_L;
	SET_DATA375(cmd);
	CS375_L;
	//等待至少2US
	ch375_Delay(1);
	WR375_H;
	CS375_H;
}
/**************************************************************
** 函数名:xWriteCH375Data
** 功能:写数据
** 注意事项:ADD=0时为写数据
***************************************************************/
void xWriteCH375Data(u8 dat)
{
	//延迟2US
	ch375_Delay(1);
	SET_DATA_OUT;
	ADD375_L;
	WR375_L;
	SET_DATA375(dat);
	CS375_L;
	//等待至少2US
	ch375_Delay(1);
	WR375_H;
	CS375_H;
}

/**************************************************************
** 函数名:xReadCH375Data
** 功能:读数据
** 注意事项:ADD=0时为数据
***************************************************************/
u8 xReadCH375Data(void)
{
	u8 tmp;
	//延迟2US
	ch375_Delay(1);
	SET_DATA_IN;
	ADD375_L;
	RD375_L;
	CS375_L;
	//等待至少2US
	ch375_Delay(1);
	tmp=GET_DATA375();
	RD375_H;
	CS375_H;
	return tmp;
}

/**************************************************************
** 函数名:xReadCH375Cmd
** 功能:读数据
** 注意事项:ADD=1时为命令
***************************************************************/
u8 xReadCH375Cmd(void)
{
	u8 tmp;
	//延迟2US
	ch375_Delay(1);
	SET_DATA_IN;
	ADD375_H;
	RD375_L;
	CS375_L;
	//等待至少2US
	ch375_Delay(1);
	tmp=GET_DATA375();
	RD375_H;
	CS375_H;
	ADD375_L;
	return tmp;
}

#include "U_HOST\ch375hfm.h"

/****************************************
** 函数名:Init375 
** 功能:初始化375
** 注意事项:RST拉高为复位,然后把控制脚都置为无效,IO脚为高阻输入
****************************************/
BOOL ch375_Reset()
{
	xWriteCH375Cmd( CMD_RESET_ALL);  /* 测试单片机与CH376之间的通讯接口 */
	SysTickDelay(50);
	return (BOOL)(CH375LibInit()==ERR_SUCCESS);
}

BOOL ch375_Init()
{
	u8 res;

	CH375_Configuration();
	RST375_H;
	CS375_H;
	RD375_H;
	WR375_H;
	SysTickDelay(40);
	RST375_L;
	SysTickDelay(100);

	xWriteCH375Cmd( 0x06 );  /* 测试单片机与CH376之间的通讯接口 */
	xWriteCH375Data( 0x65 );
	res = xReadCH375Data( );
	//	xEndCH376Cmd( );  // 并口方式不需要
	if(res==0x9A)	 /* 通讯接口不正常,可能原因有:接口连接异常,其它设备影响(片选不唯一),串口波特率,一直在复位,晶振不工作 */
	{
		res = CH375LibInit( ); 		//375软件初始化
#if CH375_INT_MODE
		ch375_INTConfig();
#endif
		return (BOOL)(res==ERR_SUCCESS);
	}
	return FALSE; 

}

u8 ch375_GetUSBStatus(void)
{
#if (CH375_INT_MODE==0)
	// if(usb_insert_flag==FALSE)
		// if ( CH375_INT_WIRE ==0)
		{
			xWriteCH375Cmd( CMD_GET_STATUS );  /* 获取当前中断状态,发出命令后至少延时2uS */
			ch375_Delay(1);
			return  xReadCH375Data( );  /* 获取中断状态 */
		}
#else
	return CH375IntStatus;	
#endif
}

typedef struct // 内部文件函数变量
{
	UINT32 nFileSize;
	UINT32 nFilePos;
	UINT32 nSectorPos;
	UINT32 nSectorSize;
	u8  nFileState; // 文件状态
} _tFileState;

_tFileState c375_FileState;

/**************************************************************
** 函数名:InitUDisk
** 功能:初始化U盘
** 注意事项:调用硬件部分初始化375,再调用软件初始化U盘-注意函数内有一个200MS的延迟
***************************************************************/
BOOL ch375_DiskIsReady()
{
	u8 rtn,i;

	rtn=CH375DiskConnect();		//检查是否连接
	if(rtn!=ERR_SUCCESS)
		return FALSE;
	
	for(i=0;i<5;i++)
	{
//		IWDG_ReloadCounter();
		SysTickDelay(500);
		//检查U盘是否准备好-加上这一步骤,则可兼容大多数U盘
		rtn=CH375DiskReady();
		if(rtn==ERR_SUCCESS)
		{
			c375_FileState.nFileState = 0;
			return TRUE;
		}																		
	}

	c375_FileState.nFileState = 0;
	return TRUE;
}

#if 1		// 使用读扇区的模式来操作U盘
#include "ff.h"
#include "diskio.h"

static volatile DSTATUS usb_Stat = STA_NOINIT;	/* Disk status */

u8 ch375_GetIntStatus(void) // 等待操作结束并返回状态
{
	while(CH375_INT_WIRE);
	xWriteCH375Cmd( CMD_GET_STATUS );  /* 获取当前中断状态,发出命令后至少延时2uS */
	ch375_Delay(1);
	return  xReadCH375Data( );  /* 获取中断状态 */
}

struct _CH375DATA_TAG
{
	u8 	*buf;		/*数据缓冲区*/
	u32		buf_Count;		//待传的数据总数
	u32		buf_Index;		/*传输的数据在一个扇区中的索引*/
}ch375data;// TCH375Data,*PCH375Data;

/* 数据读写, 单片机读写CH375芯片中的数据缓冲区 */
u8 rd_usb_data(  u8 *buf , u8 bySize) {  /* 从CH37X读出数据块 */
	unsigned char len,i;
	
	xWriteCH375Cmd( CMD_RD_USB_DATA );  /* 从CH375的端点缓冲区读取接收到的数据 */
	len=xReadCH375Data();  /* 后续数据长度 */
	// len= len<bySize?len:bySize;
	if(len<=64)
	{
		ch375_Delay(1);
		for(i=0;i<len;i++)
		{
			if(i<bySize)
				*buf++=xReadCH375Data();
			else
				xReadCH375Data();;
		}
	}
	return( len );
}

void wr_usb_data( unsigned char len, u8 *buf ) {  /* 向CH37X写入数据块 */
	xWriteCH375Cmd( CMD_WR_USB_DATA7 );  /* 向CH375的端点缓冲区写入准备发送的数据 */
	xWriteCH375Data( len );  /* 后续数据长度, len不能大于64 */
	ch375_Delay(1);
	while( len-- )
	{
		xWriteCH375Data( *buf++ );
	}
}

int ch375_ReadSec(u32 sec_start, unsigned char * buf, u8 nSecCnt)
{
	TSize size;
	int err;
	u8 cnt;

	size.l= sec_start;
	ch375data.buf= buf;
	ch375data.buf_Index=0;
	ch375data.buf_Count= (u32)nSecCnt<<9;

	xWriteCH375Cmd(CMD_DISK_READ);
	xWriteCH375Data(size.b[0]);
	xWriteCH375Data(size.b[1]);
	xWriteCH375Data(size.b[2]);
	xWriteCH375Data(size.b[3]);
	xWriteCH375Data(nSecCnt);

	err= 0;
	while(err==0){
		switch(ch375_GetIntStatus())
		{
		case USB_INT_DISK_READ:
			cnt=rd_usb_data(&ch375data.buf[ch375data.buf_Index], 64);
			ch375data.buf_Index+=cnt;
			xWriteCH375Cmd(CMD_DISK_RD_GO);
			break;
		case USB_INT_SUCCESS:
			err= 1;
			break;
		default:
			err= 2;
			break;
		}
	}
	return !err;
}

int ch375_WriteSec(u32 sec_start, unsigned char * buf, u8 nSecCnt)
{
	TSize size;
	int err;
	u8 cnt;

	//while(CH375_INT_WIRE);
	size.l= sec_start;
	ch375data.buf= buf;
	ch375data.buf_Index=0;
	ch375data.buf_Count= (u32)nSecCnt<<9;

	xWriteCH375Cmd(CMD_DISK_WRITE);
	xWriteCH375Data(size.b[0]);
	xWriteCH375Data(size.b[1]);
	xWriteCH375Data(size.b[2]);
	xWriteCH375Data(size.b[3]);
	xWriteCH375Data(nSecCnt);

	err= 0;
	while(err==0)
		switch(ch375_GetIntStatus())
		{
		case USB_INT_DISK_WRITE:			/* USB存储器写数据块, 请求数据写入 */
			cnt= (ch375data.buf_Count>CH375_MAX_DATA_LEN)?CH375_MAX_DATA_LEN : ch375data.buf_Count;
			wr_usb_data(cnt,&ch375data.buf[ch375data.buf_Index]);
			ch375data.buf_Index+=cnt;
			ch375data.buf_Count-= cnt;
			xWriteCH375Cmd(CMD_DISK_WR_GO);
			break;
		case USB_INT_SUCCESS:
			err= 1;
			break;
		default:
			err= 2;
			break;
		}

	return !err;
}

// *************************************************************************************
//  设置当前单元
// *************************************************************************************
BOOL ch375_SetDiskLun(u8 lun)
{
	//ch375_op_status=1;
	xWriteCH375Cmd( CMD_SET_DISK_LUN);  // 设置当前单元
	ch375_Delay(1);
	xWriteCH375Data(0x34);
	ch375_Delay(1);
	xWriteCH375Data(lun);
	ch375_Delay(1);
	return TRUE;
}

BOOL	ch375_GetDiskSize(u8 lun, unsigned long *bytespersec, unsigned long *seccount)
{
	TSize bps,sc;
	u8 buf[64], byTimeOut= 100, c;
	if(ch375_SetDiskLun(lun)==FALSE) return FALSE;
	if(usb_disk_Isready)
	{
		xWriteCH375Cmd( CMD_DISK_SIZE );  /* 获取USB存储器的容量 */
		ch375_Delay(1);
		while((ch375_GetIntStatus()!=USB_INT_SUCCESS)&&(byTimeOut--));  /* 等待中断并获取状态 */
	}
	if (byTimeOut==0) return FALSE;  /* 出现错误 */
	/* 可以由CMD_RD_USB_DATA命令将容量数据读出 */
	c=rd_usb_data(buf, 64);
	if(c<8) return FALSE;

	sc.b[0]= buf[3]; sc.b[1]=buf[2]; sc.b[2]=buf[1]; sc.b[3]=buf[0];
	*seccount= sc.l;
	bps.b[0]= buf[7]; bps.b[1]=buf[6]; bps.b[2]=buf[5]; bps.b[3]=buf[4];
	*bytespersec= bps.l;
	//size= ((buf[0]<<24)+(buf[1]<<16)+(buf[2]<<8)+buf[3])*((buf[4]<<24)+(buf[5]<<16)+(buf[6]<<8)+buf[7]);

	return TRUE;
}

//在auto_mount()内调用
DSTATUS usb_disk_initialize ()
{
	usb_Stat |= STA_NODISK;
	if(usb_disk_Isready) 
		usb_Stat &= ~STA_NODISK;
	usb_Stat &= ~STA_NOINIT;
	return usb_Stat;
}


/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/
DSTATUS usb_disk_status (
					BYTE drv		/* Physical drive nmuber (0) */
					)
{
	if (drv!=USB) return STA_NOINIT;				/* Supports only single drive */
	return usb_Stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT usb_disk_read (
				  BYTE *buff,		  /* Pointer to the data buffer to store read data */
				  DWORD sector,				  /* Start sector number (LBA) */
				  BYTE count		  /* Sector count (1..255) */
				  )
{
	if (!count) return RES_PARERR;
	if (usb_Stat & STA_NOINIT) return RES_NOTRDY;
	if (usb_Stat & STA_NODISK) return RES_NOTRDY;
	return ch375_ReadSec(sector, buff, count) ? RES_ERROR : RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
#if _READONLY == 0
DRESULT usb_disk_write (
				   const BYTE *buff,   /* Pointer to the data to be written */
				   DWORD sector,	   /* Start sector number (LBA) */
				   BYTE count		   /* Sector count (1..255) */
				   )
{
	if (!count) return RES_PARERR;
	if (usb_Stat & STA_NOINIT) return RES_NOTRDY;
	if (usb_Stat & STA_NODISK) return RES_NOTRDY;
	if (usb_Stat & STA_PROTECT)	return RES_WRPRT;

	return ch375_WriteSec(sector, (u8 *)buff, count) ? RES_ERROR : RES_OK;
}
#endif /* _READONLY == 0 */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL != 0
DRESULT usb_disk_ioctl (
				   BYTE ctrl,	   /* Control code */
				   void *buff	   /* Buffer to send/receive control data */
				   )
{
	DRESULT res;
	unsigned long bytespersec, seccount;
	BYTE *ptr = buff;

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
			res = RES_OK;
			break;
		default :
			res = RES_PARERR;
		}
	}
	else
	{
		if (usb_Stat & STA_NOINIT) return RES_NOTRDY;

		switch (ctrl)
		{
		case CTRL_SYNC :		/* Make sure that no pending write process */
			res = RES_OK;
			break;

		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
			if((usb_Stat&STA_NODISK)==0)
			{
				if(ch375_GetDiskSize(0, &bytespersec, &seccount))
				{
					*(DWORD*)buff= seccount;
					res = RES_OK;
				}
			}
			break;

		case GET_SECTOR_SIZE :	/* Get R/W sector size (WORD) */
			if((usb_Stat&STA_NODISK)==0)
			{
				if(ch375_GetDiskSize(0, &bytespersec, &seccount))
				{
					*(DWORD*)buff= bytespersec;
					res = RES_OK;
				}
			}
			break;

		case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
			*(DWORD*)buff = 512;
			res = RES_OK;
			break;

		default:
			res = RES_PARERR;
		}

	}

	return res;
}
#endif /* _USE_IOCTL != 0 */


#else
/**************************************************************
    以下函数体均调用375库函数,之所以要从新编写这些OPENFILE之类的函数,
 	是因为375的库写的不是很规范---一些读写参数要通过直接操作结构体后才能调用功能函数,
 	所以以下函数把375的库从新封装成标准的FAT32函数,这样做会让代码稍微慢了点,但对于STM32 72M的运行速度来说,可以接受
	v1.0版-只为ISP100系列而做的简易版本
***************************************************************/
#include "U_Host\filelong.h"

u8 *FILE_DATA_BUF_ADDR= FILE_DATA_BUF;
u8 *FILE_DATA_BUF_ADDR1= FILE_DATA_BUF+(FILE_DATA_BUF_LEN >>1);

/*计算短文件名的校验和，*/
u8 ChkSum(P_FAT_DIR_INFO pDir1)
{
	u8 FcbNameLen;
	u8 Sum;
	Sum = 0;
	for (FcbNameLen=0; FcbNameLen!=11; FcbNameLen++) {
		//if(pDir1->DIR_Name[FcbNameLen]==0x20)continue;
		Sum = ((Sum & 1) ? 0x80 : 0) + (Sum >> 1) + pDir1->DIR_Name[FcbNameLen];
	}
	return (Sum);
}

/*检查上级子目录并打开*/
u8 mChkName(u8 *pJ){
		u8 i,j;
	j = 0xFF;
	for ( i = 0; i != sizeof( mCmdParam.Create.mPathName ); i ++ ) {  /* 检查目录路径 */
		if ( mCmdParam.Create.mPathName[ i ] == 0 ) break;
		if ( mCmdParam.Create.mPathName[ i ] == PATH_SEPAR_CHAR1 || mCmdParam.Create.mPathName[ i ] == PATH_SEPAR_CHAR2 ) j = i;  /* 记录上级目录 */
	}
	i = ERR_SUCCESS;
	if ( j == 0 || j == 2 && mCmdParam.Create.mPathName[1] == ':' ) {  /* 在根目录下创建 */
		mCmdParam.Open.mPathName[ 0]='/';
		mCmdParam.Open.mPathName[ 1]=0;
		i=CH375FileOpen();			/*打开根目录*/
		if ( i == ERR_OPEN_DIR ) i = ERR_SUCCESS;  /* 成功打开上级目录 */	
	}
	else {
		if ( j != 0xFF ) {  /* 对于绝对路径应该获取上级目录的起始簇号 */
			mCmdParam.Create.mPathName[ j ] = 0;
			i = CH375FileOpen( );  /* 打开上级目录 */
			if ( i == ERR_SUCCESS ) i = ERR_MISS_DIR;  /* 是文件而非目录 */
			else if ( i == ERR_OPEN_DIR ) i = ERR_SUCCESS;  /* 成功打开上级目录 */
			mCmdParam.Create.mPathName[ j ] = PATH_SEPAR_CHAR1;  /* 恢复目录分隔符 */
		}		
	}
	*pJ=j;										/*指针中返回一组数据*/
	return i;
}

/*读取指定短文件名的长文件名,返回长文件名在长文件名空间*/
u8  mLookUpLName( char *lfname)
{
	// u8  BlockSer1;				/*定义两个扇区块内记数*/
	u8   ParData[MAX_PATH_LEN];			/**/
	UINT16	tempSec;						/*扇区偏移*/
	u8 a,i,j,x,k,sum, separPos;
	F_LONG_NAME     *pLDirName; 
	P_FAT_DIR_INFO  pDirName;
	u8  FBuf;
	u8   *pBuf;
	UINT16   c;
	for(k=0;k!=MAX_PATH_LEN;k++)
		ParData[k]=mCmdParam.Other.mBuffer[k];      
	i=mChkName(&separPos);	
	separPos++;
	if ( i == ERR_SUCCESS ) {  /* 成功获取上级目录的起始簇号 */
		//BlockSer1=0;
		FBuf=0;					/*初始化*/	
		tempSec=0;				
		while(1){							/*下面是读取并分析目录项*/
			pDirName=(P_FAT_DIR_INFO)(FBuf?FILE_DATA_BUF_ADDR1:FILE_DATA_BUF_ADDR);		/*短文件名指针指向缓冲区*/		
			mCmdParam.ReadX.mSectorCount=1;				/*读取一扇区数据*/
			mCmdParam.ReadX.mDataBuffer=FBuf?FILE_DATA_BUF_ADDR1:FILE_DATA_BUF_ADDR;  /*当前处理的文件缓冲区,这里使用双向缓冲区，去处理文件名*/
			FBuf=!FBuf;												/*缓冲区标志翻转*/
			i=CH375FileReadX( );
			if(i!=ERR_SUCCESS)goto XFF;
			if(mCmdParam.ReadX.mSectorCount==0){k=16;break;}			/*表示没有数据读出*/
			tempSec+=1;												/*扇区记数加一*/
			for(k=0;k!=16;k++){																	
				pBuf=&ParData[separPos];						
				if(pDirName->DIR_Name[0]==0){k=15;a=1;continue;}		/*第一个字节为0，表示以后没有有效的目录项了*/
				if(pDirName->DIR_Name[0]==0xe5){pDirName++;continue;}			/*第一个字节为0XE5表示此项被删除*/
				if(pDirName->DIR_Attr==ATTR_VOLUME_ID){pDirName++;continue;}		/*为卷标，不分析*/
				if(pDirName->DIR_Attr==ATTR_LONG_NAME){pDirName++;continue;}		/*为长文件名，不分析*/
				for(i=0;i!=8;i++){									/*分析文件名是否相同*/
					if(pDirName->DIR_Name[i]==0x20)continue;		/*为20不分析*/        
					if(pDirName->DIR_Name[i]!=*pBuf)break;
					else pBuf++;
				}
				if(i!=8){pDirName++;continue;}						/*没有找到匹配的短文件名*/
				if(*pBuf=='.')pBuf++;
				for(;i!=11;i++){
					if(pDirName->DIR_Name[i]==0x20)continue;				
					if(pDirName->DIR_Name[i]!=*pBuf)break;		
					else pBuf++;
				}
				if(i==11){break;}								/*表示找到文件名*/
				pDirName++;
			}
			if(i==11)break;								/*找到*/
			if(a==1){k=16;break;}
		}
		if(k!=16){
	 		 // pBuf1=LongFileName;
			 x=0;
			 sum=ChkSum(pDirName); 					       /*计算和*/				
			 pLDirName=(F_LONG_NAME *)(FBuf?FILE_DATA_BUF_ADDR:FILE_DATA_BUF_ADDR1);	/*长文件名指针指向缓冲区*/
			 pLDirName+=(k-1);
			 if(k==0){
			 		pLDirName=(F_LONG_NAME *)(FBuf?FILE_DATA_BUF_ADDR1:FILE_DATA_BUF_ADDR);  //如果短文件名是丛第一组开始的，长文件名就要往起移动
					pLDirName+=15;						
					k=15;					
			}
			while(1){	
				if(pLDirName->LDIR_Attr==0x0f&pLDirName->LDIR_Chksum==sum&pLDirName->LDIR_Ord!=0xe5){
					for(j=0;j!=5;j++){
						c = ff_convert((UINT16)((UINT16)(pLDirName->LDIR_Name1[j*2+1]<<8)+(UINT16)pLDirName->LDIR_Name1[j*2]), 0);			/* Unicode -> OEM conversion */
						if (!c) { i = 0; break; }		/* Could not convert, no LFN */
						if(c>0xA0)
							*lfname++=c>>8;
						*lfname++=c;
					}
					for(;j!=11;j++){	
						c = ff_convert(pLDirName->LDIR_Name2[j-5], 0);			/* Unicode -> OEM conversion */
						if (!c) { i = 0; break; }		/* Could not convert, no LFN */
						if(c>0xA0)
							*lfname++=c>>8;
						*lfname++=c;
					}
					for(;j!=13;j++){	
						c = ff_convert(pLDirName->LDIR_Name3[j-11], 0);			/* Unicode -> OEM conversion */
						if (!c) { i = 0; break; }		/* Could not convert, no LFN */
						if(c>0xA0)
							*lfname++=c>>8;
						*lfname++=c;
					}
						/*这里将长文件名复制出去，最大16*13个长文件名*/					
				   		x++;	 
				}
				else break;							/*没有长文件名则条出*/
				if(x==15)break;						/*最大限制长文件名为16*13字节*/
				if(k==0){						/*首先移动文件指针*/					
					pLDirName=(F_LONG_NAME *)(FBuf?FILE_DATA_BUF_ADDR1:FILE_DATA_BUF_ADDR);
					pLDirName+=15;														
				}
				else {pLDirName-=1; k-=1;}
			}
		}					
	}
	*lfname=0;
	i=CH375FileClose( );	
	return 0;					
XFF:  return i;
}
/*
u8 LongFileNameTostr(UINT16 *ptr,u8 *FileName)
{
	u8 tmp,count;
	UINT16	Index;
	
	while(*ptr!=0)
	{
		tmp= *ptr >>8;
		if(tmp)
		{
			for(Index=0;Index<7445;Index++)
				if(GB2UNICODE[Index][1]==*ptr)
				{
					*FileName=(u8)(0x80+(u8)((GB2UNICODE[Index][0])>>8));
					FileName++;
					*FileName=(u8)(GB2UNICODE[Index][0]+0x80);
					count++;
					break;
				}
			if(Index==7445)
				{
					*FileName='?';
					FileName++;
					*FileName='?';
					count++;
				}
		}else
			*FileName=(u8)*ptr;
			
		FileName++;
		count++;
		ptr++;
	}
	*FileName=0;
	FileName++;
	return(count);
}
*/

//以下几个变量是为ISP100系统而存在
//static UINT32 Read_File_Left_Size;		//文件剩下多大没读取
//static UINT32 Read_Sector_Cnt;			//每次需要读取的扇区数
//static UINT32 Read_Each_Size;			//每次可以读取的大小
static UINT32 File_Size;				//文件大小,此值实时更新在打开文件后确定,在WRITE后改变
//static UINT32 File_Offset;				//文件指针偏移,此值实时更新


/**************************************************************
** 函数名:OpenFile,CloseFile
** 功能:打开文件
** 注意事项:输入文件名必须为大写,这是375库的规定~~, 另外如果要读文件,则文件大小CH375vFileSize必须加大(一个扇区-1)字节 
** 				这些均通过函数内部解决了. 
** 返回:错误代码 
** 版本:V1.0 建议加入文件小写判断,自动改小写为大写,这样本函数就不受文件名的大小写限制 
***************************************************************/
FRESULT ch375_f_open(FIL * fp, const TCHAR * path, BYTE mode)
{
	u8 rtn;

	if ( c375_FileState.nFileState!=0 )
		ch375_f_close(fp);

	strcpy( (char *)mCmdParam.Open.mPathName, path);
	rtn=CH375FileOpen();

	File_Size=CH375vFileSize;
	if ( rtn != ERR_SUCCESS )
	  return FR_NO_FILE;

	fp->fsize= CH375vFileSize;
	fp->fptr= 0;
	fp->dsect= 0;
	//c375_FileState.nFileState= 1;
	c375_FileState.nFileSize = File_Size;
	c375_FileState.nFilePos  = 0;
	c375_FileState.nSectorPos = 0;
	c375_FileState.nSectorSize = 0;
  
	CH375vFileSize += CH375vSectorSize-1;	//CH375系统特别部分,文件大小要加大一点,
	return FR_OK;
}

#if 0
/**************************************************************
** 函数名:CreateFile
** 功能:创建文件
** 注意事项:创建文件后,同时也打开了文件,直接关闭,则文件的大小会是1个字节.
** 版本:
***************************************************************/

u8 CreateFile(const char* fil_name)
{
	u8 rtn;
	strcpy( (char *)mCmdParam.Open.mPathName, fil_name);
	rtn=CH375FileCreate();
	File_Size=1;
//	File_Offset=0;
	return rtn;
}
#endif

//关闭文件*库不支持两个文件同时操作,故无需输入文件名
FRESULT ch375_f_close(FIL * fp)
{
	c375_FileState.nFileState = 0;
	if (CH375FileClose()==ERR_SUCCESS)
		return FR_OK;
  return FR_NO_FILE;
}

#if 0
/**************************************************************
** 函数名:MkDir
** 功能:创建目录
** 注意事项:CH375的函数库本身没有创建目录的函数,这个是官方后期补漏的函数,内部涉及了一些FAT的底层操作
***************************************************************/
const char* DirConstData=".          \x10\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x21\x30\x0\x0\x0\x0\x0\x0..         \x10\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x21\x30\x0\x0\x0\x0\x0\x0";
u8	MkDir( char* dir_name )
{
	u8	i, j;
	UINT16	count;
	UINT32	UpDirCluster;
	u8*	DirXramBuf;
	
	strcpy((char*)mCmdParam.Create.mPathName,(const char*)dir_name);

	j = 0xFF;
	for (i = 0; i != sizeof( mCmdParam.Create.mPathName ); i ++) 
	{  /* 检查目录路径 */
		if (mCmdParam.Create.mPathName[ i ] == 0) 
			break;
		if (mCmdParam.Create.mPathName[ i ] == PATH_SEPAR_CHAR1 || mCmdParam.Create.mPathName[ i ] == PATH_SEPAR_CHAR2) 
			j = i;  /* 记录上级目录 */
	}
	i = ERR_SUCCESS;
	if( j == 0 || j == 2 && mCmdParam.Create.mPathName[1] == ':' ) 
		UpDirCluster = 0;  /* 在根目录下创建子目录 */
	else 
	{
		if (j != 0xFF) 
		{  /* 对于绝对路径应该获取上级目录的起始簇号 */
			mCmdParam.Create.mPathName[ j ] = 0;
			i = CH375FileOpen( );  /* 打开上级目录 */
			if (i == ERR_SUCCESS) i = ERR_MISS_DIR;  /* 是文件而非目录 */
			else if (i == ERR_OPEN_DIR) i = ERR_SUCCESS;  /* 成功打开上级目录 */
			mCmdParam.Create.mPathName[ j ] = PATH_SEPAR_CHAR1;  /* 恢复目录分隔符 */
		}
		UpDirCluster = CH375vStartCluster;  /* 保存上级目录的起始簇号 */
	}
	if (i == ERR_SUCCESS) 
	{  /* 成功获取上级目录的起始簇号 */
		i = CH375FileOpen();  /* 打开本级子目录 */
		if (i == ERR_SUCCESS) 
			i = ERR_FOUND_NAME;  /* 是文件而非目录 */
		else if (i == ERR_OPEN_DIR) 
			i = ERR_SUCCESS;  /* 目录已经存在 */
		else if (i == ERR_MISS_FILE) 
		{  /* 目录不存在,可以新建 */
			i = CH375FileCreate();  /* 以创建文件的方法创建目录 */
			if (i == ERR_SUCCESS) 
			{
//				if ( &FILE_DATA_BUF[0] == &DISK_BASE_BUF[0] ) CH375DirtyBuffer( );  /* 如果FILE_DATA_BUF与DISK_BASE_BUF合用则必须清除磁盘缓冲区 */
				DirXramBuf = &FILE_DATA_BUF[0];  /* 文件数据缓冲区 */
				for ( i = 0x40; i != 0; i -- ) 
				{  /* 目录的保留单元,分别指向自身和上级目录 */
					*DirXramBuf = *DirConstData;
					DirXramBuf ++;
					DirConstData ++;
				}

				FILE_DATA_BUF[0x1A] = (u8)CH375vStartCluster;  /* 自身的起始簇号 */
				FILE_DATA_BUF[0x1B] = (u8)(CH375vStartCluster>>8);
				FILE_DATA_BUF[0x14] = (u8)(CH375vStartCluster>>16);
				FILE_DATA_BUF[0x15] = (u8)(CH375vStartCluster>>24);
				FILE_DATA_BUF[0x20+0x1A] = (u8)UpDirCluster;  /* 上级目录的起始簇号 */
				FILE_DATA_BUF[0x20+0x1B] = (u8)(UpDirCluster>>8);
				FILE_DATA_BUF[0x20+0x14] = (u8)(UpDirCluster>>16);
				FILE_DATA_BUF[0x20+0x15] = (u8)(UpDirCluster>>24);

				for ( count = 0x40; count != CH375vSectorSize; count ++ ) 
				{  /* 清空目录区剩余部分 */
					*DirXramBuf = 0;
					DirXramBuf ++;
				}
				mCmdParam.Write.mSectorCount = 1;
				i = CH375FileWrite( );  /* 写目录的第一个扇区 */
				if ( i == ERR_SUCCESS ) 
				{
					DirXramBuf = &FILE_DATA_BUF[0];
					for ( i = 0x40; i != 0; i -- ) 
					{  /* 清空目录区 */
						*DirXramBuf = 0;
						DirXramBuf ++;
					}
					for ( j = 1; j != CH375vSecPerClus; j ++ ) 
					{
//						if ( &FILE_DATA_BUF[0] == &DISK_BASE_BUF[0] ) CH375DirtyBuffer( );  /* 如果FILE_DATA_BUF与DISK_BASE_BUF合用则必须清除磁盘缓冲区 */
						mCmdParam.Write.mSectorCount = 1;
						i = CH375FileWrite( );  /* 清空目录的剩余扇区 */
						if ( i != ERR_SUCCESS ) break;
					}
					if ( j == CH375vSecPerClus ) 
					{  /* 成功清空目录 */
						mCmdParam.Modify.mFileSize = 0;  /* 目录的长度总是0 */
						mCmdParam.Modify.mFileDate = 0xFFFF;
						mCmdParam.Modify.mFileTime = 0xFFFF;
						mCmdParam.Modify.mFileAttr = 0x10;  /* 置目录属性 */
						i = CH375FileModify( );  /* 将文件信息修改为目录 */
					}
				}
			}
		}
	}
	return( i );
}
#endif
/**************************************************************
** 函数名:ReadFile
** 功能:读取文件到指定缓冲区
** 注意事项:需要打开文件后才能调用, 
** 返回值为错误代码,buf:缓冲指针,len 要读取的值,cnt已读取的值
** 这里使用读扇区的方式读取文件,如果文件的偏移是跨扇区的,则程序 
** 内部有对应的处理方式 
**		这个没空写,迟点再说~
***************************************************************/
u8 ReadSector( u8* buf )
{
	u8 rtn = ERR_SUCCESS;
	mCmdParam.ReadX.mSectorCount = 1;
	mCmdParam.ReadX.mDataBuffer = buf;
	rtn = CH375FileReadX();
	if ( rtn == ERR_SUCCESS && mCmdParam.ReadX.mSectorCount != 0 )
		return rtn;
	return 1;
}
UINT32 ReadBuffer( u8* buf, UINT32 len)
{
	UINT32 i;
	UINT32 siz;
	UINT32 fsiz;

	if ( c375_FileState.nFileSize < c375_FileState.nFilePos )
		return 1;

	siz = c375_FileState.nSectorSize - c375_FileState.nSectorPos;
	fsiz = c375_FileState.nFileSize - c375_FileState.nFilePos;

	siz = siz<fsiz?siz:fsiz;
	siz = len<siz?len:siz;

	for ( i = 0; i < siz; i++, c375_FileState.nFilePos++ )
		*buf++ =  FILE_DATA_BUF[c375_FileState.nSectorPos++];

	return siz;
}

FRESULT ch375_f_read(FIL * fp, void * buff, UINT btr, UINT * br)
{
	u8 rtn;
	UINT32 i, nsec, nrealsec;
	u8* tmp_buf;

	*br = 0; //清除返回值
	if ( btr == 0 )
		return FR_OK;
	tmp_buf = buff;

	// 是否有剩余缓冲
		if ( c375_FileState.nSectorPos < c375_FileState.nSectorSize )
		{
			i = ReadBuffer( tmp_buf, btr);
			btr -= i;
			*br += i;
			if ( btr == 0 || i == 0 ) return FR_OK;		// i = 0 可能是文件到结尾了
			tmp_buf += i;
		}

	// 每次读出一个扇区
	nsec= btr >> 9;	// 取得可读出的最大扇区数
	//nrealsec= (c375_FileState.nFileSize - c375_FileState.nFilePos)>>9;
	nrealsec= (CH375vFileSize- c375_FileState.nFilePos)>>9;
	nsec= (nsec<nrealsec)?nsec:nrealsec;
	mCmdParam.ReadX.mSectorCount = nsec;
	mCmdParam.ReadX.mDataBuffer = tmp_buf;
	rtn = CH375FileReadX();
	if ( rtn == ERR_SUCCESS && mCmdParam.ReadX.mSectorCount != 0 )
	{
		i= mCmdParam.ReadX.mDataBuffer - tmp_buf;
		// c375_FileState.nFilePos += i;
		btr &= 0x1ff;
		*br+= i;
	} else
		return FR_DISK_ERR;
	// 零碎读出
	if(btr)
	{
		rtn = ReadSector( FILE_DATA_BUF );
		if ( rtn != ERR_SUCCESS )
		{
			if ( *br > 0 )
				return FR_OK; //在前面已经读出一部分内容了
			else
				return FR_DISK_ERR;
		}
		c375_FileState.nSectorSize = CH375vSectorSize;
		c375_FileState.nSectorPos  = 0;

		i = ReadBuffer( tmp_buf, btr);
		*br += i;
	}
	return FR_OK;
}

/**************************************************************
** 函数名:WriteFile
** 功能:写文件
** 注意事项:1.采用了过渡的缓冲区,虽然这样速度会有所下降,但可以使函数写的更规范更容易用于移植 
**  		
**  		2.为了能加快速度,过渡缓冲只用于处理写入缓冲少于一个扇区的情况,也就是说,如果len大于一个扇区,则扇区
**  		采用直接写入方式
			3.此函数的效率还有待提高
***************************************************************/
FRESULT ch375_f_write(FIL * fp, const void * buff, UINT btw, UINT * bw)
{
	UINT32 i;
	u8* tmp_buf;
	u8 rtn;
	
	if(btw==0)
	{
		return FR_OK;
	}

	//每次只写入一个扇区
	i=0;
	tmp_buf=(u8 *)buff;
	mCmdParam.Write.mSectorCount=1;
	//整片扇区写入
	while(btw>=CH375vSectorSize)
	{
		btw-=CH375vSectorSize;
		mCmdParam.WriteX.mDataBuffer =tmp_buf;
		i++;
		tmp_buf=(u8 *)buff+i*CH375vSectorSize;
		rtn=CH375FileWriteX();
		if(rtn!=ERR_SUCCESS)
		{
			return FR_DISK_ERR;
		}
		//实时修改文件大小
		File_Size+=CH375vSectorSize;
	}
	//零碎写入
	if(btw)
	{
		//搬运到过渡缓冲中
		for(i=0;i<btw;i++)
		{
			FILE_DATA_BUF[i]=*tmp_buf++;
		}
		mCmdParam.WriteX.mDataBuffer=FILE_DATA_BUF;
		rtn=CH375FileWriteX();
		if(rtn!=ERR_SUCCESS)
		{
			return FR_DISK_ERR;
		}
		//实时修改文件大小
		File_Size+=btw;

		//零碎写入需修改FAT底层部分,使其大小为真实大小
		mCmdParam.Modify.mFileSize = File_Size;   /* 输入参数: 新的文件长度,扇区模式下涉及到零头数据不便自动更新长度 */
		mCmdParam.Modify.mFileAttr = 0xff;  /* 输入参数: 新的文件属性,为0FFH则不修改 */
		mCmdParam.Modify.mFileTime = 0xffff;  /* 输入参数: 新的文件时间,为0FFH则不修改 */
		mCmdParam.Modify.mFileDate = 0xffff;  /* 输入参数: 新的文件日期,为0FFH则不修改 */
		rtn=CH375FileModify();   /* 修改当前文件的信息,修改文件长度 */
	}
  if(rtn==ERR_SUCCESS)
    return FR_OK;
	return FR_DISK_ERR;
}

/**************************************************************
** 函数名:LocatFile
** 功能:定位文件指针
** 注意事项:0xffffffff为移动到文件末尾
***************************************************************/
FRESULT ch375_f_lseek(FIL * fp, DWORD ofs)
{
	mCmdParam.Locate.mSectorOffset = ofs;
  if(CH375FileLocate()==ERR_SUCCESS)
    return FR_OK;
  return FR_DISK_ERR;
}

UINT32 ch375_GetLocat(void)
{
	return 0;
}

extern BOOL IsSongFile(char *fn);
BOOL ch375_GetDirectoryInfo(u16 *pFlCnt)
{
	u8	c, nRes;
	*pFlCnt= 0;
	for ( c = 0; c < 255; c ++ ) {  /* 最多搜索前255个文件 */
		strcpy( (char *)mCmdParam.Open.mPathName, "/*");  /* 搜索文件名,*为通配符,适用于所有文件或者子目录 */
		mCmdParam.Open.mPathName[ 2 ] = c;  /* 根据字符串长度将结束符替换为搜索的序号,从0到255 */
		nRes=CH375FileOpen( );  /* 打开文件,如果文件名中含有通配符*,则为搜索文件而不打开 */
		if ( nRes== ERR_MISS_FILE ) return FALSE;  /* 再也搜索不到匹配的文件,已经没有匹配的文件名 */
		if ( nRes== ERR_FOUND_NAME ) {  /* 搜索到与通配符相匹配的文件名,文件名及其完整路径在命令缓冲区中 */
			if ( IsSongFile((char *)mCmdParam.Open.mPathName))	// 是乐曲文件
			{
				if(pFlCnt)
					(*pFlCnt)++;
			}
			continue;  /* 继续搜索下一个匹配的文件名,下次搜索时序号会加1 */
		}
		else {  /* 出错 */
			// mStopIfError( i );
			break;
		}
	}
	return TRUE;
}

#include "player.h"

BOOL ch375_FindSongIndex(const char *szPath, char *szFile, UINT16 *index, struct _TAG_PLAYER *pPlayer, char *lfname)
{
	UINT16	pathLen;
	u8	c, pCodeStr[MAX_PATH_LEN], nRes;
	u16 nCnt=0xFFFF;
	char sLastFile[14], sFirstFile[14], lfn[51], lfnfirst[51];

  for(pathLen=0; pathLen<MAX_PATH_LEN-2; pathLen++)
  {
    pCodeStr[pathLen]= szPath[pathLen];
		if(szPath[pathLen]==0) break;
  }
  if(szPath[pathLen]) return FALSE; // 路径太长
  if((pCodeStr[pathLen-1]!=(u8)'/')&&(pCodeStr[pathLen-1]!=(u8)'\\'))
    pCodeStr[pathLen++]= '/';
	pCodeStr[pathLen++]= '*';
	for ( c = 0; c < 255; c ++ ) {  /* 最多搜索前255个文件 */
		strcpy( (char *)mCmdParam.Open.mPathName, (char *)pCodeStr );  /* 搜索文件名,*为通配符,适用于所有文件或者子目录 */
		mCmdParam.Open.mPathName[ pathLen ] = c;  /* 根据字符串长度将结束符替换为搜索的序号,从0到255 */
		nRes=CH375FileOpen( );  /* 打开文件,如果文件名中含有通配符*,则为搜索文件而不打开 */
		if ( nRes== ERR_MISS_FILE ) break;  /* 再也搜索不到匹配的文件,已经没有匹配的文件名 */
		if ( nRes== ERR_FOUND_NAME ) {  /* 搜索到与通配符相匹配的文件名,文件名及其完整路径在命令缓冲区中 */
			if ( IsSongFile((char *)mCmdParam.Open.mPathName))	// 是乐曲文件
			{
				nCnt++;
				if(nCnt==0)
				{
					strncpy(sFirstFile, (char *)mCmdParam.Open.mPathName, 13);
	  				mLookUpLName(lfn);
					if(lfname)
						strncpy(lfnfirst, lfn, 50);
				}
				if(*index==0xFFFF)
				{
					strncpy(sLastFile, (char *)mCmdParam.Open.mPathName, 13);
	  				mLookUpLName(lfn);
					if(lfname)
						strncpy(lfname, lfn, 50);
				}else
					if(*index==nCnt)
					{
						pPlayer->IndexOfDir= nCnt;
						if(nCnt==0)
						{							
							strncpy(szFile-1, sFirstFile, 13);
							if(lfname)
								strncpy(lfname, lfnfirst, 50);
						}else
						{
							strncpy(szFile-1, (char *)mCmdParam.Open.mPathName, 13);
		  				mLookUpLName(lfn);
							if(lfname)
								strncpy(lfname, lfn, 50);
						}
						szFile[12]= 0;
						return TRUE;
					}	
			}
			continue;  /* 继续搜索下一个匹配的文件名,下次搜索时序号会加1 */
		}
		else {  /* 出错 */
			// mStopIfError( i );
			break;
		}
	}
	if((*index==0xFFFF)&&(nCnt!=0xFFFF))
	{
		pPlayer->IndexOfDir= nCnt;
		strncpy(szFile-1, sLastFile, 13);
		szFile[12]= 0;
		return TRUE;
	}
	if((*index>=nCnt)&&(pPlayer->RepeatMode>REPEAT_ONE))// index超出歌曲总数且是循环全部模式
	{
		pPlayer->IndexOfDir= 0;
		strncpy(szFile-1, sFirstFile, 13);
		if(lfname)
			strncpy(lfname, lfnfirst, 50);
		szFile[12]= 0;
		return TRUE;
	}
	return FALSE;
}
#endif

