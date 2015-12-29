SYSTEMPARAM 系统参数   收音机存这里面
增加分区 在audio.c中
typedef struct _TAGMENUITEM
{
	u8 byMenuID;			// 菜单编号
	u8 byParentMenuID;		// 父菜单编号	
	u8 Title_xPos;
	u8 Title_len;
	const char *Title_szName;
	void *pParam;				// 菜单参数指针
	// 按键处理函数
	// 参数：1-PMENUITEM， 2- 键值，3-键值掩码
	unsigned int (*KeyDoFunc)(struct _TAGMENUITEM *, unsigned char , unsigned char );
	// 初始化参数
	void (*InitFunc)(void *);
	// 显示函数
	void (*DisplayUIFunc)(struct _TAGMENUITEM *);
}MENUITEM, *PMENUITEM;

typedef struct	{
	DIR	dj;
	u8	bySongPath;				// 路径标识0-SD		1-USB
	u8	byCurRow;				// 当前反显行
	u16	wFileCount;				// 乐曲文件总数
	u16	wFileIndex;				// 当前文件编号
}SONGVIEWPARAM;

修改后的对应模块管脚   
报警按键  PE5 低电平有效
SD卡模块  硬件SPI1(改完)
	SD_CS     PA4
	SD_CLK    PA5
	SD_MISO	  PA6 
	SD_MOSI	  PA7  
	SD_CD     PC4	SD卡插入检测
MP3模块VS1003  硬件SPI2 (改完)
	MP3_DCS	  PB1
	MP3_RST   PB2
	MP3_DREO  PE7
	MP3_CS    PB12 
	MP3_CLK	  PB13
	MP3_MISO  PB14
	MP3_MOSI  PB15
CH375模块 (改完)
	CH375_RD  PB10
	CH375_A0  PB11
	CH375_WR    PD8
	CH375_RST	PD9
	CH375_INT	PD10
	CH375_CS	PD11
	CH375_D0---D7  PE8---PE15
CD4051音源控制模块(改完)
	audio_ctr1	PC0 
	audio_ctr2 	PC1
	audio_ctr3 	PC3
分区控制端口6个分区 低电平开  高电平关 (改完)
	FQ_CTR1     PA8
	FQ_CTR2		PC9
	FQ_CTR3		PC8
	FQ_CTR4		PC7
	FQ_CTR5		PC6 	FQ_CTR6		PD15
外部3路电源  低电平开   高电平关(改完)
	PWR_CRT1    PC12     AUX
	PWR_CRT2	PC11     功放2
	PWR_CRT3	PC10     功放1
按键GPIO (改完)
	KEY_V1      PE2            KEY_H1	PE3
	KEY_V2	   	PE1	           KEY_H2	PE4
	KEY_V3      PC15           KEY_H3   PE6
	KEY_V4      PC13           KEY_H4   PC14
LCD模块(改完)
	LCD_BACKLIGHT   PE0
	LCD_E			PB5
	LCD_RW		    PB6
	LCD	_RS			PB7
	LCD_CSA         PB9
	LCD_CSB			PB8
	LCD_D0--D7      PD0--PD7
收音模块(改完)
	FM_RST			PD14
	SCL				PD12
	SDA             PD13
PC16短路输出    		PA11(改完)
报警输入            PA12
紧急按键    PE5

















