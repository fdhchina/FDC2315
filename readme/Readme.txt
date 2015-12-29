SYSTEMPARAM ϵͳ����   ��������������
���ӷ��� ��audio.c��
typedef struct _TAGMENUITEM
{
	u8 byMenuID;			// �˵����
	u8 byParentMenuID;		// ���˵����	
	u8 Title_xPos;
	u8 Title_len;
	const char *Title_szName;
	void *pParam;				// �˵�����ָ��
	// ����������
	// ������1-PMENUITEM�� 2- ��ֵ��3-��ֵ����
	unsigned int (*KeyDoFunc)(struct _TAGMENUITEM *, unsigned char , unsigned char );
	// ��ʼ������
	void (*InitFunc)(void *);
	// ��ʾ����
	void (*DisplayUIFunc)(struct _TAGMENUITEM *);
}MENUITEM, *PMENUITEM;

typedef struct	{
	DIR	dj;
	u8	bySongPath;				// ·����ʶ0-SD		1-USB
	u8	byCurRow;				// ��ǰ������
	u16	wFileCount;				// �����ļ�����
	u16	wFileIndex;				// ��ǰ�ļ����
}SONGVIEWPARAM;

�޸ĺ�Ķ�Ӧģ��ܽ�   
��������  PE5 �͵�ƽ��Ч
SD��ģ��  Ӳ��SPI1(����)
	SD_CS     PA4
	SD_CLK    PA5
	SD_MISO	  PA6 
	SD_MOSI	  PA7  
	SD_CD     PC4	SD��������
MP3ģ��VS1003  Ӳ��SPI2 (����)
	MP3_DCS	  PB1
	MP3_RST   PB2
	MP3_DREO  PE7
	MP3_CS    PB12 
	MP3_CLK	  PB13
	MP3_MISO  PB14
	MP3_MOSI  PB15
CH375ģ�� (����)
	CH375_RD  PB10
	CH375_A0  PB11
	CH375_WR    PD8
	CH375_RST	PD9
	CH375_INT	PD10
	CH375_CS	PD11
	CH375_D0---D7  PE8---PE15
CD4051��Դ����ģ��(����)
	audio_ctr1	PC0 
	audio_ctr2 	PC1
	audio_ctr3 	PC3
�������ƶ˿�6������ �͵�ƽ��  �ߵ�ƽ�� (����)
	FQ_CTR1     PA8
	FQ_CTR2		PC9
	FQ_CTR3		PC8
	FQ_CTR4		PC7
	FQ_CTR5		PC6 	FQ_CTR6		PD15
�ⲿ3·��Դ  �͵�ƽ��   �ߵ�ƽ��(����)
	PWR_CRT1    PC12     AUX
	PWR_CRT2	PC11     ����2
	PWR_CRT3	PC10     ����1
����GPIO (����)
	KEY_V1      PE2            KEY_H1	PE3
	KEY_V2	   	PE1	           KEY_H2	PE4
	KEY_V3      PC15           KEY_H3   PE6
	KEY_V4      PC13           KEY_H4   PC14
LCDģ��(����)
	LCD_BACKLIGHT   PE0
	LCD_E			PB5
	LCD_RW		    PB6
	LCD	_RS			PB7
	LCD_CSA         PB9
	LCD_CSB			PB8
	LCD_D0--D7      PD0--PD7
����ģ��(����)
	FM_RST			PD14
	SCL				PD12
	SDA             PD13
PC16��·���    		PA11(����)
��������            PA12
��������    PE5

















