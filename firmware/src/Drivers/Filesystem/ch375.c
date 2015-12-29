/**************************************************************
ģ����:ch375.c 
����ʵ��:CH375Ӳ���ײ����ֲ����,���⻹���������ĸĽ�,����CH375�Ŀ⺯����д�Ƚϲ����淶, 
 		�ʴ�ģ�鸺��ѿ⺯�����·�װ���ļ���д��ͨ�ø�ʽ.����װ���д�ٶȻ������½�.�����Ҫ
 		�ϸ��ٵĲ���,���Լ�ȥ��ԭ�⺯��
 		***һ������,��Ӧ��ʹ��ԭ��ĺ���,����������ݴ���
����:���Ӱײ�




�汾��:V1.0 
�汾˵��:	1.CH375Ӳ����������.����IOָ����Ҫ���ӳ�,��������һ��Ӳdelay����,���Լ�����CPU���ٶ�,���ͺ����ٶ�,ʹϵͳ����
			2.��д����ʹ��ͨ�ø�ʽ,�������в�����������Ϊ��λ,�ʲ��ʺ�51֮��ʹ��,�м�ʹ���˻������,�ŵ�������ͨ����,ȱ���ǽ������ٶ�. 
		 	3.�����˴���Ŀ¼����  
����:2009-4-10
***************************************************************/

#include "hal.h"
#include "my375lib.h"

bool usb_insert_flag=FALSE;
bool usb_disk_Isready=FALSE;

//�⼸��������Ҫ��375��ͷ�ļ�֮ǰ

#define LIB_CFG_INT_EN			CH375_INT_MODE
#if(GC_ZR&GC_ZR_GPIO) 
#define CH375_INT_WIRE (GPIOD->IDR & GPIO_Pin_10)
#else
#define CH375_INT_WIRE (GPIOE->IDR & GPIO_Pin_13)
#endif
#define FILE_DATA_BUF_LEN		MY_FILE_DATA_BUF_LEN
#define DISK_BASE_BUF_LEN		MY_DISK_BASE_BUF_LEN

u8  DISK_BASE_BUF[ DISK_BASE_BUF_LEN ];	/* �ⲿRAM�Ĵ������ݻ�����,����������Ϊһ�������ĳ��� */

u8  FILE_DATA_BUF[ FILE_DATA_BUF_LEN ];	/* �ⲿRAM���ļ����ݻ�����,���������Ȳ�С��һ�ζ�д�����ݳ��� */

#define NO_DEFAULT_CH375_F_ENUM		1		/* ����CH375FileEnumer*/
#define NO_DEFAULT_CH375_F_QUERY	1		/* ����CH375FileQuery*/
#define NO_DEFAULT_DELAY_100US		1		/* ����Ĭ�ϵ���ʱ100uS�ӳ��� */
void xDelay100uS( void )
{
	delay_us(100);
}
u8 xReadCH375Cmd(void);


#include <string.h>
#include <stdio.h>

/**************************************************************
** ������:Ch375_Configuration
** ����:Ӳ������CH375
** ע������:
***************************************************************/
void CH375_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
#if(GC_ZR&GC_ZR_GPIO)
	// E
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD, ENABLE);

	//PE12-RST,PE9-WE, PE8-OE, PE10-A0 	//PE11-CS
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//50Mʱ���ٶ�
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_11;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	//PE13-INT,
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//��������
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	//E����Ϊʹ����IO,���Բ�����������
#else
	// E
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

	//PE12-RST,PE9-WE, PE8-OE, PE10-A0 	//PE11-CS
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//50Mʱ���ٶ�
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	//PE13-INT,
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//��������
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	//E����Ϊʹ����IO,���Բ�����������
#endif
}

#if(GC_ZR&GC_ZR_GPIO)
//�������ߺ궨��-Ϊ��ʹ�����ٶȼӿ�,����û��ʹ�ÿ�
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
//�������ߺ궨��-Ϊ��ʹ�����ٶȼӿ�,����û��ʹ�ÿ�
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
//#define GET_INT		(GPIOE->IDR & GPIO_Pin_13)	/*��ȡINT��״̬*/
#if (GC_ZR&GC_ZR_GPIO)
/**************************************************************
���������궨�����ڸı�IO�ڵ��������״̬,Ϊ������ٶ�,�������ֱ�Ӹı�Ĵ����İ취,����Ӧ�ø��� 
ʹ�õ�IO�ŵĲ�ͬ�����������,����ʹ��PE0-7��ΪDATA�� ,�ʲ����ļĴ���ΪGPIOA->CRL
***************************************************************/
#define SET_DATA_OUT		GPIOE->CRH=0x33333333	/*�������50MHZ�ٶ�?*/
#define SET_DATA_IN			GPIOE->CRH=0x44444444	/*��������*/

//#define GET_DATA375()		(GPIOE->IDR&0xff00)
//#define SET_DATA375(dat)	GPIOE->BSRR=(((dat<<8)&0xff00)|(((~dat)<<24)&0xFF000000))//�������ODR�Ĵ���
#define GET_DATA375()		((GPIOE->IDR>>8)&0xff)
#define SET_DATA375(dat)	GPIOE->BSRR=((dat<<8)|((~dat)<<24))	// �������ODR�Ĵ���
#else
/**************************************************************
���������궨�����ڸı�IO�ڵ��������״̬,Ϊ������ٶ�,�������ֱ�Ӹı�Ĵ����İ취,����Ӧ�ø��� 
ʹ�õ�IO�ŵĲ�ͬ�����������,����ʹ��PE0-7��ΪDATA�� ,�ʲ����ļĴ���ΪGPIOA->CRL
***************************************************************/
#define SET_DATA_OUT		GPIOE->CRL=0x33333333	/*�������50MHZ�ٶ�?*/
#define SET_DATA_IN			GPIOE->CRL=0x44444444	/*��������*/
	
#define GET_DATA375()		(GPIOE->IDR&0xff)
#define SET_DATA375(dat)	GPIOE->BSRR=(dat|(((~dat)<<16)&0x00FF0000))	//�������ODR�Ĵ���
#endif
//Ӳ�ӳٺ���,�ɸ���CPU�ٶ��𲽵���������ٶȵ��ٽ��
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

  	/* ����IO�ڵ��ж��� */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource13);
	/*����Ϊ�½��ش���*/
	EXTI_InitStructure.EXTI_Line = EXTI_Line13;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	EXTI_GenerateSWInterrupt(EXTI_Line13);
}
#endif
/***************************************
** ������:xWriteCH375Cmd 
** ���� :д����
** ע������: ADD=1Ϊ����,����ʱ������Ҫ��4US
***************************************/
void xWriteCH375Cmd(u8 cmd)
{
	//�ӳ�2US
	ch375_Delay(1);
	SET_DATA_OUT;
	ADD375_H;
	WR375_L;
	SET_DATA375(cmd);
	CS375_L;
	//�ȴ�����2US
	ch375_Delay(1);
	WR375_H;
	CS375_H;
}
/**************************************************************
** ������:xWriteCH375Data
** ����:д����
** ע������:ADD=0ʱΪд����
***************************************************************/
void xWriteCH375Data(u8 dat)
{
	//�ӳ�2US
	ch375_Delay(1);
	SET_DATA_OUT;
	ADD375_L;
	WR375_L;
	SET_DATA375(dat);
	CS375_L;
	//�ȴ�����2US
	ch375_Delay(1);
	WR375_H;
	CS375_H;
}

/**************************************************************
** ������:xReadCH375Data
** ����:������
** ע������:ADD=0ʱΪ����
***************************************************************/
u8 xReadCH375Data(void)
{
	u8 tmp;
	//�ӳ�2US
	ch375_Delay(1);
	SET_DATA_IN;
	ADD375_L;
	RD375_L;
	CS375_L;
	//�ȴ�����2US
	ch375_Delay(1);
	tmp=GET_DATA375();
	RD375_H;
	CS375_H;
	return tmp;
}

/**************************************************************
** ������:xReadCH375Cmd
** ����:������
** ע������:ADD=1ʱΪ����
***************************************************************/
u8 xReadCH375Cmd(void)
{
	u8 tmp;
	//�ӳ�2US
	ch375_Delay(1);
	SET_DATA_IN;
	ADD375_H;
	RD375_L;
	CS375_L;
	//�ȴ�����2US
	ch375_Delay(1);
	tmp=GET_DATA375();
	RD375_H;
	CS375_H;
	ADD375_L;
	return tmp;
}

#include "U_HOST\ch375hfm.h"

/****************************************
** ������:Init375 
** ����:��ʼ��375
** ע������:RST����Ϊ��λ,Ȼ��ѿ��ƽŶ���Ϊ��Ч,IO��Ϊ��������
****************************************/
BOOL ch375_Reset()
{
	xWriteCH375Cmd( CMD_RESET_ALL);  /* ���Ե�Ƭ����CH376֮���ͨѶ�ӿ� */
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

	xWriteCH375Cmd( 0x06 );  /* ���Ե�Ƭ����CH376֮���ͨѶ�ӿ� */
	xWriteCH375Data( 0x65 );
	res = xReadCH375Data( );
	//	xEndCH376Cmd( );  // ���ڷ�ʽ����Ҫ
	if(res==0x9A)	 /* ͨѶ�ӿڲ�����,����ԭ����:�ӿ������쳣,�����豸Ӱ��(Ƭѡ��Ψһ),���ڲ�����,һֱ�ڸ�λ,���񲻹��� */
	{
		res = CH375LibInit( ); 		//375�����ʼ��
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
			xWriteCH375Cmd( CMD_GET_STATUS );  /* ��ȡ��ǰ�ж�״̬,���������������ʱ2uS */
			ch375_Delay(1);
			return  xReadCH375Data( );  /* ��ȡ�ж�״̬ */
		}
#else
	return CH375IntStatus;	
#endif
}

typedef struct // �ڲ��ļ���������
{
	UINT32 nFileSize;
	UINT32 nFilePos;
	UINT32 nSectorPos;
	UINT32 nSectorSize;
	u8  nFileState; // �ļ�״̬
} _tFileState;

_tFileState c375_FileState;

/**************************************************************
** ������:InitUDisk
** ����:��ʼ��U��
** ע������:����Ӳ�����ֳ�ʼ��375,�ٵ��������ʼ��U��-ע�⺯������һ��200MS���ӳ�
***************************************************************/
BOOL ch375_DiskIsReady()
{
	u8 rtn,i;

	rtn=CH375DiskConnect();		//����Ƿ�����
	if(rtn!=ERR_SUCCESS)
		return FALSE;
	
	for(i=0;i<5;i++)
	{
//		IWDG_ReloadCounter();
		SysTickDelay(500);
		//���U���Ƿ�׼����-������һ����,��ɼ��ݴ����U��
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

#if 1		// ʹ�ö�������ģʽ������U��
#include "ff.h"
#include "diskio.h"

static volatile DSTATUS usb_Stat = STA_NOINIT;	/* Disk status */

u8 ch375_GetIntStatus(void) // �ȴ���������������״̬
{
	while(CH375_INT_WIRE);
	xWriteCH375Cmd( CMD_GET_STATUS );  /* ��ȡ��ǰ�ж�״̬,���������������ʱ2uS */
	ch375_Delay(1);
	return  xReadCH375Data( );  /* ��ȡ�ж�״̬ */
}

struct _CH375DATA_TAG
{
	u8 	*buf;		/*���ݻ�����*/
	u32		buf_Count;		//��������������
	u32		buf_Index;		/*�����������һ�������е�����*/
}ch375data;// TCH375Data,*PCH375Data;

/* ���ݶ�д, ��Ƭ����дCH375оƬ�е����ݻ����� */
u8 rd_usb_data(  u8 *buf , u8 bySize) {  /* ��CH37X�������ݿ� */
	unsigned char len,i;
	
	xWriteCH375Cmd( CMD_RD_USB_DATA );  /* ��CH375�Ķ˵㻺������ȡ���յ������� */
	len=xReadCH375Data();  /* �������ݳ��� */
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

void wr_usb_data( unsigned char len, u8 *buf ) {  /* ��CH37Xд�����ݿ� */
	xWriteCH375Cmd( CMD_WR_USB_DATA7 );  /* ��CH375�Ķ˵㻺����д��׼�����͵����� */
	xWriteCH375Data( len );  /* �������ݳ���, len���ܴ���64 */
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
		case USB_INT_DISK_WRITE:			/* USB�洢��д���ݿ�, ��������д�� */
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
//  ���õ�ǰ��Ԫ
// *************************************************************************************
BOOL ch375_SetDiskLun(u8 lun)
{
	//ch375_op_status=1;
	xWriteCH375Cmd( CMD_SET_DISK_LUN);  // ���õ�ǰ��Ԫ
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
		xWriteCH375Cmd( CMD_DISK_SIZE );  /* ��ȡUSB�洢�������� */
		ch375_Delay(1);
		while((ch375_GetIntStatus()!=USB_INT_SUCCESS)&&(byTimeOut--));  /* �ȴ��жϲ���ȡ״̬ */
	}
	if (byTimeOut==0) return FALSE;  /* ���ִ��� */
	/* ������CMD_RD_USB_DATA����������ݶ��� */
	c=rd_usb_data(buf, 64);
	if(c<8) return FALSE;

	sc.b[0]= buf[3]; sc.b[1]=buf[2]; sc.b[2]=buf[1]; sc.b[3]=buf[0];
	*seccount= sc.l;
	bps.b[0]= buf[7]; bps.b[1]=buf[6]; bps.b[2]=buf[5]; bps.b[3]=buf[4];
	*bytespersec= bps.l;
	//size= ((buf[0]<<24)+(buf[1]<<16)+(buf[2]<<8)+buf[3])*((buf[4]<<24)+(buf[5]<<16)+(buf[6]<<8)+buf[7]);

	return TRUE;
}

//��auto_mount()�ڵ���
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
    ���º����������375�⺯��,֮����Ҫ���±�д��ЩOPENFILE֮��ĺ���,
 	����Ϊ375�Ŀ�д�Ĳ��Ǻܹ淶---һЩ��д����Ҫͨ��ֱ�Ӳ����ṹ�����ܵ��ù��ܺ���,
 	�������º�����375�Ŀ���·�װ�ɱ�׼��FAT32����,���������ô�����΢���˵�,������STM32 72M�������ٶ���˵,���Խ���
	v1.0��-ֻΪISP100ϵ�ж����ļ��װ汾
***************************************************************/
#include "U_Host\filelong.h"

u8 *FILE_DATA_BUF_ADDR= FILE_DATA_BUF;
u8 *FILE_DATA_BUF_ADDR1= FILE_DATA_BUF+(FILE_DATA_BUF_LEN >>1);

/*������ļ�����У��ͣ�*/
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

/*����ϼ���Ŀ¼����*/
u8 mChkName(u8 *pJ){
		u8 i,j;
	j = 0xFF;
	for ( i = 0; i != sizeof( mCmdParam.Create.mPathName ); i ++ ) {  /* ���Ŀ¼·�� */
		if ( mCmdParam.Create.mPathName[ i ] == 0 ) break;
		if ( mCmdParam.Create.mPathName[ i ] == PATH_SEPAR_CHAR1 || mCmdParam.Create.mPathName[ i ] == PATH_SEPAR_CHAR2 ) j = i;  /* ��¼�ϼ�Ŀ¼ */
	}
	i = ERR_SUCCESS;
	if ( j == 0 || j == 2 && mCmdParam.Create.mPathName[1] == ':' ) {  /* �ڸ�Ŀ¼�´��� */
		mCmdParam.Open.mPathName[ 0]='/';
		mCmdParam.Open.mPathName[ 1]=0;
		i=CH375FileOpen();			/*�򿪸�Ŀ¼*/
		if ( i == ERR_OPEN_DIR ) i = ERR_SUCCESS;  /* �ɹ����ϼ�Ŀ¼ */	
	}
	else {
		if ( j != 0xFF ) {  /* ���ھ���·��Ӧ�û�ȡ�ϼ�Ŀ¼����ʼ�غ� */
			mCmdParam.Create.mPathName[ j ] = 0;
			i = CH375FileOpen( );  /* ���ϼ�Ŀ¼ */
			if ( i == ERR_SUCCESS ) i = ERR_MISS_DIR;  /* ���ļ�����Ŀ¼ */
			else if ( i == ERR_OPEN_DIR ) i = ERR_SUCCESS;  /* �ɹ����ϼ�Ŀ¼ */
			mCmdParam.Create.mPathName[ j ] = PATH_SEPAR_CHAR1;  /* �ָ�Ŀ¼�ָ��� */
		}		
	}
	*pJ=j;										/*ָ���з���һ������*/
	return i;
}

/*��ȡָ�����ļ����ĳ��ļ���,���س��ļ����ڳ��ļ����ռ�*/
u8  mLookUpLName( char *lfname)
{
	// u8  BlockSer1;				/*���������������ڼ���*/
	u8   ParData[MAX_PATH_LEN];			/**/
	UINT16	tempSec;						/*����ƫ��*/
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
	if ( i == ERR_SUCCESS ) {  /* �ɹ���ȡ�ϼ�Ŀ¼����ʼ�غ� */
		//BlockSer1=0;
		FBuf=0;					/*��ʼ��*/	
		tempSec=0;				
		while(1){							/*�����Ƕ�ȡ������Ŀ¼��*/
			pDirName=(P_FAT_DIR_INFO)(FBuf?FILE_DATA_BUF_ADDR1:FILE_DATA_BUF_ADDR);		/*���ļ���ָ��ָ�򻺳���*/		
			mCmdParam.ReadX.mSectorCount=1;				/*��ȡһ��������*/
			mCmdParam.ReadX.mDataBuffer=FBuf?FILE_DATA_BUF_ADDR1:FILE_DATA_BUF_ADDR;  /*��ǰ������ļ�������,����ʹ��˫�򻺳�����ȥ�����ļ���*/
			FBuf=!FBuf;												/*��������־��ת*/
			i=CH375FileReadX( );
			if(i!=ERR_SUCCESS)goto XFF;
			if(mCmdParam.ReadX.mSectorCount==0){k=16;break;}			/*��ʾû�����ݶ���*/
			tempSec+=1;												/*����������һ*/
			for(k=0;k!=16;k++){																	
				pBuf=&ParData[separPos];						
				if(pDirName->DIR_Name[0]==0){k=15;a=1;continue;}		/*��һ���ֽ�Ϊ0����ʾ�Ժ�û����Ч��Ŀ¼����*/
				if(pDirName->DIR_Name[0]==0xe5){pDirName++;continue;}			/*��һ���ֽ�Ϊ0XE5��ʾ���ɾ��*/
				if(pDirName->DIR_Attr==ATTR_VOLUME_ID){pDirName++;continue;}		/*Ϊ��꣬������*/
				if(pDirName->DIR_Attr==ATTR_LONG_NAME){pDirName++;continue;}		/*Ϊ���ļ�����������*/
				for(i=0;i!=8;i++){									/*�����ļ����Ƿ���ͬ*/
					if(pDirName->DIR_Name[i]==0x20)continue;		/*Ϊ20������*/        
					if(pDirName->DIR_Name[i]!=*pBuf)break;
					else pBuf++;
				}
				if(i!=8){pDirName++;continue;}						/*û���ҵ�ƥ��Ķ��ļ���*/
				if(*pBuf=='.')pBuf++;
				for(;i!=11;i++){
					if(pDirName->DIR_Name[i]==0x20)continue;				
					if(pDirName->DIR_Name[i]!=*pBuf)break;		
					else pBuf++;
				}
				if(i==11){break;}								/*��ʾ�ҵ��ļ���*/
				pDirName++;
			}
			if(i==11)break;								/*�ҵ�*/
			if(a==1){k=16;break;}
		}
		if(k!=16){
	 		 // pBuf1=LongFileName;
			 x=0;
			 sum=ChkSum(pDirName); 					       /*�����*/				
			 pLDirName=(F_LONG_NAME *)(FBuf?FILE_DATA_BUF_ADDR:FILE_DATA_BUF_ADDR1);	/*���ļ���ָ��ָ�򻺳���*/
			 pLDirName+=(k-1);
			 if(k==0){
			 		pLDirName=(F_LONG_NAME *)(FBuf?FILE_DATA_BUF_ADDR1:FILE_DATA_BUF_ADDR);  //������ļ����ǴԵ�һ�鿪ʼ�ģ����ļ�����Ҫ�����ƶ�
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
						/*���ｫ���ļ������Ƴ�ȥ�����16*13�����ļ���*/					
				   		x++;	 
				}
				else break;							/*û�г��ļ���������*/
				if(x==15)break;						/*������Ƴ��ļ���Ϊ16*13�ֽ�*/
				if(k==0){						/*�����ƶ��ļ�ָ��*/					
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

//���¼���������ΪISP100ϵͳ������
//static UINT32 Read_File_Left_Size;		//�ļ�ʣ�¶��û��ȡ
//static UINT32 Read_Sector_Cnt;			//ÿ����Ҫ��ȡ��������
//static UINT32 Read_Each_Size;			//ÿ�ο��Զ�ȡ�Ĵ�С
static UINT32 File_Size;				//�ļ���С,��ֵʵʱ�����ڴ��ļ���ȷ��,��WRITE��ı�
//static UINT32 File_Offset;				//�ļ�ָ��ƫ��,��ֵʵʱ����


/**************************************************************
** ������:OpenFile,CloseFile
** ����:���ļ�
** ע������:�����ļ�������Ϊ��д,����375��Ĺ涨~~, �������Ҫ���ļ�,���ļ���СCH375vFileSize����Ӵ�(һ������-1)�ֽ� 
** 				��Щ��ͨ�������ڲ������. 
** ����:������� 
** �汾:V1.0 ��������ļ�Сд�ж�,�Զ���СдΪ��д,�����������Ͳ����ļ����Ĵ�Сд���� 
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
  
	CH375vFileSize += CH375vSectorSize-1;	//CH375ϵͳ�ر𲿷�,�ļ���СҪ�Ӵ�һ��,
	return FR_OK;
}

#if 0
/**************************************************************
** ������:CreateFile
** ����:�����ļ�
** ע������:�����ļ���,ͬʱҲ�����ļ�,ֱ�ӹر�,���ļ��Ĵ�С����1���ֽ�.
** �汾:
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

//�ر��ļ�*�ⲻ֧�������ļ�ͬʱ����,�����������ļ���
FRESULT ch375_f_close(FIL * fp)
{
	c375_FileState.nFileState = 0;
	if (CH375FileClose()==ERR_SUCCESS)
		return FR_OK;
  return FR_NO_FILE;
}

#if 0
/**************************************************************
** ������:MkDir
** ����:����Ŀ¼
** ע������:CH375�ĺ����Ȿ��û�д���Ŀ¼�ĺ���,����ǹٷ����ڲ�©�ĺ���,�ڲ��漰��һЩFAT�ĵײ����
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
	{  /* ���Ŀ¼·�� */
		if (mCmdParam.Create.mPathName[ i ] == 0) 
			break;
		if (mCmdParam.Create.mPathName[ i ] == PATH_SEPAR_CHAR1 || mCmdParam.Create.mPathName[ i ] == PATH_SEPAR_CHAR2) 
			j = i;  /* ��¼�ϼ�Ŀ¼ */
	}
	i = ERR_SUCCESS;
	if( j == 0 || j == 2 && mCmdParam.Create.mPathName[1] == ':' ) 
		UpDirCluster = 0;  /* �ڸ�Ŀ¼�´�����Ŀ¼ */
	else 
	{
		if (j != 0xFF) 
		{  /* ���ھ���·��Ӧ�û�ȡ�ϼ�Ŀ¼����ʼ�غ� */
			mCmdParam.Create.mPathName[ j ] = 0;
			i = CH375FileOpen( );  /* ���ϼ�Ŀ¼ */
			if (i == ERR_SUCCESS) i = ERR_MISS_DIR;  /* ���ļ�����Ŀ¼ */
			else if (i == ERR_OPEN_DIR) i = ERR_SUCCESS;  /* �ɹ����ϼ�Ŀ¼ */
			mCmdParam.Create.mPathName[ j ] = PATH_SEPAR_CHAR1;  /* �ָ�Ŀ¼�ָ��� */
		}
		UpDirCluster = CH375vStartCluster;  /* �����ϼ�Ŀ¼����ʼ�غ� */
	}
	if (i == ERR_SUCCESS) 
	{  /* �ɹ���ȡ�ϼ�Ŀ¼����ʼ�غ� */
		i = CH375FileOpen();  /* �򿪱�����Ŀ¼ */
		if (i == ERR_SUCCESS) 
			i = ERR_FOUND_NAME;  /* ���ļ�����Ŀ¼ */
		else if (i == ERR_OPEN_DIR) 
			i = ERR_SUCCESS;  /* Ŀ¼�Ѿ����� */
		else if (i == ERR_MISS_FILE) 
		{  /* Ŀ¼������,�����½� */
			i = CH375FileCreate();  /* �Դ����ļ��ķ�������Ŀ¼ */
			if (i == ERR_SUCCESS) 
			{
//				if ( &FILE_DATA_BUF[0] == &DISK_BASE_BUF[0] ) CH375DirtyBuffer( );  /* ���FILE_DATA_BUF��DISK_BASE_BUF���������������̻����� */
				DirXramBuf = &FILE_DATA_BUF[0];  /* �ļ����ݻ����� */
				for ( i = 0x40; i != 0; i -- ) 
				{  /* Ŀ¼�ı�����Ԫ,�ֱ�ָ��������ϼ�Ŀ¼ */
					*DirXramBuf = *DirConstData;
					DirXramBuf ++;
					DirConstData ++;
				}

				FILE_DATA_BUF[0x1A] = (u8)CH375vStartCluster;  /* �������ʼ�غ� */
				FILE_DATA_BUF[0x1B] = (u8)(CH375vStartCluster>>8);
				FILE_DATA_BUF[0x14] = (u8)(CH375vStartCluster>>16);
				FILE_DATA_BUF[0x15] = (u8)(CH375vStartCluster>>24);
				FILE_DATA_BUF[0x20+0x1A] = (u8)UpDirCluster;  /* �ϼ�Ŀ¼����ʼ�غ� */
				FILE_DATA_BUF[0x20+0x1B] = (u8)(UpDirCluster>>8);
				FILE_DATA_BUF[0x20+0x14] = (u8)(UpDirCluster>>16);
				FILE_DATA_BUF[0x20+0x15] = (u8)(UpDirCluster>>24);

				for ( count = 0x40; count != CH375vSectorSize; count ++ ) 
				{  /* ���Ŀ¼��ʣ�ಿ�� */
					*DirXramBuf = 0;
					DirXramBuf ++;
				}
				mCmdParam.Write.mSectorCount = 1;
				i = CH375FileWrite( );  /* дĿ¼�ĵ�һ������ */
				if ( i == ERR_SUCCESS ) 
				{
					DirXramBuf = &FILE_DATA_BUF[0];
					for ( i = 0x40; i != 0; i -- ) 
					{  /* ���Ŀ¼�� */
						*DirXramBuf = 0;
						DirXramBuf ++;
					}
					for ( j = 1; j != CH375vSecPerClus; j ++ ) 
					{
//						if ( &FILE_DATA_BUF[0] == &DISK_BASE_BUF[0] ) CH375DirtyBuffer( );  /* ���FILE_DATA_BUF��DISK_BASE_BUF���������������̻����� */
						mCmdParam.Write.mSectorCount = 1;
						i = CH375FileWrite( );  /* ���Ŀ¼��ʣ������ */
						if ( i != ERR_SUCCESS ) break;
					}
					if ( j == CH375vSecPerClus ) 
					{  /* �ɹ����Ŀ¼ */
						mCmdParam.Modify.mFileSize = 0;  /* Ŀ¼�ĳ�������0 */
						mCmdParam.Modify.mFileDate = 0xFFFF;
						mCmdParam.Modify.mFileTime = 0xFFFF;
						mCmdParam.Modify.mFileAttr = 0x10;  /* ��Ŀ¼���� */
						i = CH375FileModify( );  /* ���ļ���Ϣ�޸�ΪĿ¼ */
					}
				}
			}
		}
	}
	return( i );
}
#endif
/**************************************************************
** ������:ReadFile
** ����:��ȡ�ļ���ָ��������
** ע������:��Ҫ���ļ�����ܵ���, 
** ����ֵΪ�������,buf:����ָ��,len Ҫ��ȡ��ֵ,cnt�Ѷ�ȡ��ֵ
** ����ʹ�ö������ķ�ʽ��ȡ�ļ�,����ļ���ƫ���ǿ�������,����� 
** �ڲ��ж�Ӧ�Ĵ���ʽ 
**		���û��д,�ٵ���˵~
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

	*br = 0; //�������ֵ
	if ( btr == 0 )
		return FR_OK;
	tmp_buf = buff;

	// �Ƿ���ʣ�໺��
		if ( c375_FileState.nSectorPos < c375_FileState.nSectorSize )
		{
			i = ReadBuffer( tmp_buf, btr);
			btr -= i;
			*br += i;
			if ( btr == 0 || i == 0 ) return FR_OK;		// i = 0 �������ļ�����β��
			tmp_buf += i;
		}

	// ÿ�ζ���һ������
	nsec= btr >> 9;	// ȡ�ÿɶ��������������
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
	// �������
	if(btr)
	{
		rtn = ReadSector( FILE_DATA_BUF );
		if ( rtn != ERR_SUCCESS )
		{
			if ( *br > 0 )
				return FR_OK; //��ǰ���Ѿ�����һ����������
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
** ������:WriteFile
** ����:д�ļ�
** ע������:1.�����˹��ɵĻ�����,��Ȼ�����ٶȻ������½�,������ʹ����д�ĸ��淶������������ֲ 
**  		
**  		2.Ϊ���ܼӿ��ٶ�,���ɻ���ֻ���ڴ���д�뻺������һ�����������,Ҳ����˵,���len����һ������,������
**  		����ֱ��д�뷽ʽ
			3.�˺�����Ч�ʻ��д����
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

	//ÿ��ֻд��һ������
	i=0;
	tmp_buf=(u8 *)buff;
	mCmdParam.Write.mSectorCount=1;
	//��Ƭ����д��
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
		//ʵʱ�޸��ļ���С
		File_Size+=CH375vSectorSize;
	}
	//����д��
	if(btw)
	{
		//���˵����ɻ�����
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
		//ʵʱ�޸��ļ���С
		File_Size+=btw;

		//����д�����޸�FAT�ײ㲿��,ʹ���СΪ��ʵ��С
		mCmdParam.Modify.mFileSize = File_Size;   /* �������: �µ��ļ�����,����ģʽ���漰����ͷ���ݲ����Զ����³��� */
		mCmdParam.Modify.mFileAttr = 0xff;  /* �������: �µ��ļ�����,Ϊ0FFH���޸� */
		mCmdParam.Modify.mFileTime = 0xffff;  /* �������: �µ��ļ�ʱ��,Ϊ0FFH���޸� */
		mCmdParam.Modify.mFileDate = 0xffff;  /* �������: �µ��ļ�����,Ϊ0FFH���޸� */
		rtn=CH375FileModify();   /* �޸ĵ�ǰ�ļ�����Ϣ,�޸��ļ����� */
	}
  if(rtn==ERR_SUCCESS)
    return FR_OK;
	return FR_DISK_ERR;
}

/**************************************************************
** ������:LocatFile
** ����:��λ�ļ�ָ��
** ע������:0xffffffffΪ�ƶ����ļ�ĩβ
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
	for ( c = 0; c < 255; c ++ ) {  /* �������ǰ255���ļ� */
		strcpy( (char *)mCmdParam.Open.mPathName, "/*");  /* �����ļ���,*Ϊͨ���,�����������ļ�������Ŀ¼ */
		mCmdParam.Open.mPathName[ 2 ] = c;  /* �����ַ������Ƚ��������滻Ϊ���������,��0��255 */
		nRes=CH375FileOpen( );  /* ���ļ�,����ļ����к���ͨ���*,��Ϊ�����ļ������� */
		if ( nRes== ERR_MISS_FILE ) return FALSE;  /* ��Ҳ��������ƥ����ļ�,�Ѿ�û��ƥ����ļ��� */
		if ( nRes== ERR_FOUND_NAME ) {  /* ��������ͨ�����ƥ����ļ���,�ļ�����������·������������� */
			if ( IsSongFile((char *)mCmdParam.Open.mPathName))	// �������ļ�
			{
				if(pFlCnt)
					(*pFlCnt)++;
			}
			continue;  /* ����������һ��ƥ����ļ���,�´�����ʱ��Ż��1 */
		}
		else {  /* ���� */
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
  if(szPath[pathLen]) return FALSE; // ·��̫��
  if((pCodeStr[pathLen-1]!=(u8)'/')&&(pCodeStr[pathLen-1]!=(u8)'\\'))
    pCodeStr[pathLen++]= '/';
	pCodeStr[pathLen++]= '*';
	for ( c = 0; c < 255; c ++ ) {  /* �������ǰ255���ļ� */
		strcpy( (char *)mCmdParam.Open.mPathName, (char *)pCodeStr );  /* �����ļ���,*Ϊͨ���,�����������ļ�������Ŀ¼ */
		mCmdParam.Open.mPathName[ pathLen ] = c;  /* �����ַ������Ƚ��������滻Ϊ���������,��0��255 */
		nRes=CH375FileOpen( );  /* ���ļ�,����ļ����к���ͨ���*,��Ϊ�����ļ������� */
		if ( nRes== ERR_MISS_FILE ) break;  /* ��Ҳ��������ƥ����ļ�,�Ѿ�û��ƥ����ļ��� */
		if ( nRes== ERR_FOUND_NAME ) {  /* ��������ͨ�����ƥ����ļ���,�ļ�����������·������������� */
			if ( IsSongFile((char *)mCmdParam.Open.mPathName))	// �������ļ�
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
			continue;  /* ����������һ��ƥ����ļ���,�´�����ʱ��Ż��1 */
		}
		else {  /* ���� */
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
	if((*index>=nCnt)&&(pPlayer->RepeatMode>REPEAT_ONE))// index����������������ѭ��ȫ��ģʽ
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

