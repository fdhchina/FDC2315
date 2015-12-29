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


//�⼸��������Ҫ��375��ͷ�ļ�֮ǰ
#define CH375_INT_WIRE (GPIOD->IDR & GPIO_Pin_10)
#define FILE_DATA_BUF_LEN		MY_FILE_DATA_BUF_LEN
#define DISK_BASE_BUF_LEN		MY_DISK_BASE_BUF_LEN

uint8_t  DISK_BASE_BUF[ DISK_BASE_BUF_LEN ];	/* �ⲿRAM�Ĵ������ݻ�����,����������Ϊһ�������ĳ��� */

uint8_t  FILE_DATA_BUF[ FILE_DATA_BUF_LEN ];	/* �ⲿRAM���ļ����ݻ�����,���������Ȳ�С��һ�ζ�д�����ݳ��� */

#define NO_DEFAULT_CH375_F_ENUM		1		/* ����CH375FileEnumer*/
#define NO_DEFAULT_CH375_F_QUERY	1		/* ����CH375FileQuery*/
#define NO_DEFAULT_DELAY_100US		1		/* ����Ĭ�ϵ���ʱ100uS�ӳ��� */
void xDelay100uS( void )
{
	delay_us(100);
}
u8 xReadCH375Cmd(void);

#include "U_HOST\\CH375HFM.h"
#include "hal.h"

#include <string.h>
#include <stdio.h>

typedef struct // �ڲ��ļ���������
{
	u32 nFileSize;
	u32 nFilePos;
	u32 nSectorPos;
	u32 nSectorSize;
	u8  nFileState; // �ļ�״̬
} _tFileState;

_tFileState c375_FileState;

/**************************************************************
** ������:Ch375_Configuration
** ����:Ӳ������CH375
** ע������:
***************************************************************/
void CH375_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
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
}


//�������ߺ궨��-Ϊ��ʹ�����ٶȼӿ�,����û��ʹ�ÿ�
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

//#define GET_INT		(GPIOE->IDR & GPIO_Pin_13)	/*��ȡINT��״̬*/

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
//Ӳ�ӳٺ���,�ɸ���CPU�ٶ��𲽵���������ٶȵ��ٽ��
#if 1
void delay(u16 t)
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

/****************************************
** ������:Init375 
** ����:��ʼ��375
** ע������:RST����Ϊ��λ,Ȼ��ѿ��ƽŶ���Ϊ��Ч,IO��Ϊ��������
****************************************/
void Init375(void)
{
	u8 res;
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
	if ( res != 0x9A ) return;//( ERR_USB_UNKNOWN );  /* ͨѶ�ӿڲ�����,����ԭ����:�ӿ������쳣,�����豸Ӱ��(Ƭѡ��Ψһ),���ڲ�����,һֱ�ڸ�λ,���񲻹��� */

}
/***************************************
** ������:xWriteCH375Cmd 
** ���� :д����
** ע������: ADD=1Ϊ����,����ʱ������Ҫ��4US
***************************************/
void xWriteCH375Cmd(u8 cmd)
{
	//�ӳ�2US
	delay(1);
	SET_DATA_OUT;
	ADD375_H;
	WR375_L;
	SET_DATA375(cmd);
	CS375_L;
	//�ȴ�����2US
	delay(1);
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
	delay(1);
	SET_DATA_OUT;
	ADD375_L;
	WR375_L;
	SET_DATA375(dat);
	CS375_L;
	//�ȴ�����2US
	delay(1);
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
	delay(1);
	SET_DATA_IN;
	ADD375_L;
	RD375_L;
	CS375_L;
	//�ȴ�����2US
	delay(1);
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
	delay(1);
	SET_DATA_IN;
	ADD375_H;
	RD375_L;
	CS375_L;
	//�ȴ�����2US
	delay(1);
	tmp=GET_DATA375();
	RD375_H;
	CS375_H;
	ADD375_L;
	return tmp;
}

/**************************************************************
** ������:InitUDisk
** ����:��ʼ��U��
** ע������:����Ӳ�����ֳ�ʼ��375,�ٵ��������ʼ��U��-ע�⺯������һ��200MS���ӳ�
***************************************************************/
u8 InitUDisk(void)
{
	u8 rtn,i;
	Init375();					//375Ӳ����ʼ��

	rtn = CH375LibInit( ); 		//375�����ʼ��
	if(rtn!=ERR_SUCCESS)
		return(rtn);

	rtn=CH375DiskConnect();		//����Ƿ�����
	if(rtn!=ERR_SUCCESS)
		return(rtn);
	
	for(i=0;i<5;i++)
	{
		SysTickDelay(500);
		//���U���Ƿ�׼����-������һ����,��ɼ��ݴ����U��
		rtn=CH375DiskReady();
		if(rtn==ERR_SUCCESS)
		{
			return rtn;
		}																		
	}

	c375_FileState.nFileState = 0;
	return ERR_SUCCESS;//(rtn);
}

/**************************************************************
    ���º����������375�⺯��,֮����Ҫ���±�д��ЩOPENFILE֮��ĺ���,
 	����Ϊ375�Ŀ�д�Ĳ��Ǻܹ淶---һЩ��д����Ҫͨ��ֱ�Ӳ����ṹ�����ܵ��ù��ܺ���,
 	�������º�����375�Ŀ���·�װ�ɱ�׼��FAT32����,���������ô�����΢���˵�,������STM32 72M�������ٶ���˵,���Խ���
	v1.0��-ֻΪISP100ϵ�ж����ļ��װ汾
***************************************************************/

//���¼���������ΪISP100ϵͳ������
//static u32 Read_File_Left_Size;		//�ļ�ʣ�¶��û��ȡ
//static u32 Read_Sector_Cnt;			//ÿ����Ҫ��ȡ��������
//static u32 Read_Each_Size;			//ÿ�ο��Զ�ȡ�Ĵ�С
static u32 File_Size;				//�ļ���С,��ֵʵʱ�����ڴ��ļ���ȷ��,��WRITE��ı�
//static u32 File_Offset;				//�ļ�ָ��ƫ��,��ֵʵʱ����


/**************************************************************
** ������:OpenFile,CloseFile
** ����:���ļ�
** ע������:�����ļ�������Ϊ��д,����375��Ĺ涨~~, �������Ҫ���ļ�,���ļ���СCH375vFileSize����Ӵ�(һ������-1)�ֽ� 
** 				��Щ��ͨ�������ڲ������. 
** ����:������� 
** �汾:V1.0 ��������ļ�Сд�ж�,�Զ���СдΪ��д,�����������Ͳ����ļ����Ĵ�Сд���� 
***************************************************************/
u8 OpenFile(const char* fil_name)
{
	u8 rtn;

	if ( c375_FileState.nFileState!=0 )
		CloseFile();

	strcpy( (char *)mCmdParam.Open.mPathName, fil_name);
	rtn=CH375FileOpen();

	File_Size=CH375vFileSize;
	if ( rtn == ERR_SUCCESS )
	{
		c375_FileState.nFileSize = File_Size;
		c375_FileState.nFilePos  = 0;
		c375_FileState.nSectorPos = 0;
		c375_FileState.nSectorSize = 0;
	}
	//	File_Offset=0;
	CH375vFileSize += CH375vSectorSize-1;	//CH375ϵͳ�ر𲿷�,�ļ���СҪ�Ӵ�һ��,
	return(rtn);
}

//�ر��ļ�*�ⲻ֧�������ļ�ͬʱ����,�����������ļ���
u8 CloseFile(void)
{
	c375_FileState.nFileState = 0;
	return(CH375FileClose());
}

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
	u16	count;
	u32	UpDirCluster;
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
u32 ReadBuffer( u8* buf, u32 len)
{
	u32 i;
	u32 siz;
	u32 fsiz;

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
u8 ReadFile(u8* buf,u32 len,u32* cnt)
{
	u8 rtn, nsec, nrealsec;
	u32 i;
	u8* tmp_buf;

	*cnt = 0; //�������ֵ
	if ( len == 0 )
		return ERR_SUCCESS;
	tmp_buf = buf;

	// �Ƿ���ʣ�໺��
		if ( c375_FileState.nSectorPos < c375_FileState.nSectorSize )
		{
			i = ReadBuffer( tmp_buf, len );
			len -= i;
			*cnt += i;
			if ( len == 0 || i == 0 ) return ERR_SUCCESS;		// i = 0 �������ļ�����β��
			tmp_buf += i;
		}

	// ÿ�ζ���һ������
	nsec= len >> 9;	// ȡ�ÿɶ��������������
	nrealsec= (c375_FileState.nFileSize - c375_FileState.nFilePos)>>9;
	nsec= (nsec<nrealsec)?nsec:nrealsec;
	mCmdParam.ReadX.mSectorCount = nsec;
	mCmdParam.ReadX.mDataBuffer = tmp_buf;
	rtn = CH375FileReadX();
	if ( rtn == ERR_SUCCESS && mCmdParam.ReadX.mSectorCount != 0 )
	{
		len &= 0x1ff;
		*cnt+= nsec<<9;
	} else
		return rtn;
	/*
	while ( len >= CH375vSectorSize && c375_FileState.nFileSize - c375_FileState.nFilePos >= CH375vSectorSize )
	{
		len -= CH375vSectorSize;
		rtn = ReadSector( tmp_buf );
		if ( rtn != ERR_SUCCESS )
			break;

		tmp_buf += CH375vSectorSize;
		c375_FileState.nFilePos += CH375vSectorSize;
		*cnt += CH375vSectorSize;
	}*/

	// �������
	if(len)
	{
		rtn = ReadSector( FILE_DATA_BUF );
		if ( rtn != ERR_SUCCESS )
		{
			if ( *cnt > 0 )
				return ERR_SUCCESS; //��ǰ���Ѿ�����һ����������
			else
				return rtn;
		}
		c375_FileState.nSectorSize = CH375vSectorSize;
		c375_FileState.nSectorPos  = 0;

		i = ReadBuffer( tmp_buf, len );
		*cnt += i;
	}
	return ERR_SUCCESS;
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
u8 WriteFile(u8* buf,u32 len)
{
	u32 i;
	u8* tmp_buf;
	u8 rtn;
	
	if(len==0)
	{
		return ERR_SUCCESS;
	}

	//ÿ��ֻд��һ������
	i=0;
	tmp_buf=buf;
	mCmdParam.Write.mSectorCount=1;
	//��Ƭ����д��
	while(len>=CH375vSectorSize)
	{
		len-=CH375vSectorSize;
		mCmdParam.WriteX.mDataBuffer =tmp_buf;
		i++;
		tmp_buf=buf+i*CH375vSectorSize;
		rtn=CH375FileWriteX();
		if(rtn!=ERR_SUCCESS)
		{
			return rtn;
		}
		//ʵʱ�޸��ļ���С
		File_Size+=CH375vSectorSize;
	}
	//����д��
	if(len)
	{
		//���˵����ɻ�����
		for(i=0;i<len;i++)
		{
			FILE_DATA_BUF[i]=*tmp_buf++;
		}
		mCmdParam.WriteX.mDataBuffer=FILE_DATA_BUF;
		rtn=CH375FileWriteX();
		if(rtn!=ERR_SUCCESS)
		{
			return rtn;
		}
		//ʵʱ�޸��ļ���С
		File_Size+=len;

		//����д�����޸�FAT�ײ㲿��,ʹ���СΪ��ʵ��С
		mCmdParam.Modify.mFileSize = File_Size;   /* �������: �µ��ļ�����,����ģʽ���漰����ͷ���ݲ����Զ����³��� */
		mCmdParam.Modify.mFileAttr = 0xff;  /* �������: �µ��ļ�����,Ϊ0FFH���޸� */
		mCmdParam.Modify.mFileTime = 0xffff;  /* �������: �µ��ļ�ʱ��,Ϊ0FFH���޸� */
		mCmdParam.Modify.mFileDate = 0xffff;  /* �������: �µ��ļ�����,Ϊ0FFH���޸� */
		rtn=CH375FileModify();   /* �޸ĵ�ǰ�ļ�����Ϣ,�޸��ļ����� */
	}
	return(rtn);
}

/**************************************************************
** ������:LocatFile
** ����:��λ�ļ�ָ��
** ע������:0xffffffffΪ�ƶ����ļ�ĩβ
***************************************************************/
u8 LocatFile(u32 offest)
{
	mCmdParam.Locate.mSectorOffset = offest;
	return(CH375FileLocate());
}
u32 GetLocat(void)
{
	return 0;
}


