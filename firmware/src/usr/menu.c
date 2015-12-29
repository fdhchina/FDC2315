#include "hal.h"

#include "tuner.h"
#include "vs1003.h"
#include "player.h"
#include "ds3231.h"
#include "audio.h"
#include "lcm19264.h"
#include "msd.h"

#include "queue.h"

#include "menu.h"
#include "stringconst.h"
#include "ampMeter.h"
#include "stmflash.h"
#include "log.h"

typedef 	struct	{
		u8 xPos;
		u8 len;
		const char *szName;
}title_t;


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

typedef enum {
	MENUID_DESKTOP= 0,		// 0
	MENUID_PLAYTUNER,
	MENUID_PLAYMEM,
	MENUID_PLAYAUX,
	MENUID_PLAYUSB,
	MENUID_PLAYPHONE,
#if (GC_ZR & GC_ZR_PLAY)
	MENUID_PLAYYJ,
	MENUID_PLAYBJ,
#endif
	MENUID_PASSWORDINPUT,
	MENUID_MAINMENU,
	MENUID_TASKMANAGER,
	MENUID_SONGVIEWMENU,
	MENUID_SYSTEMSET,
	MENUID_SPECIALMODE,
	MENUID_TASKVIEW,
	MENUID_JOBEDIT,
	MENUID_GROUPLIST,
	MENUID_INTERCUTLIST,
#if (GC_ZR & GC_ZR_REMOTE)
	MENUID_YKEDIT,
	MENUID_PHONEEDIT,
#endif
	MENUID_SONGVIEWMEM,
	MENUID_SONGVIEWUSB,
	MENUID_TIMESET,
	MENUID_PASSWORDSET,
#if (GC_ZR & GC_ZR_BJ)
	MENUID_BJSET,
#endif
	MENUID_JOBSONGSET,
	MENUID_JOBTUNERSET,
	MENUID_SPECIALJOBEDIT,
	MENUID_TASKVIEWLIST,
	MENUID_PORTEDIT,
	MENUID_PORTCTRL,
	MENUID_INVALID=0xFF				// 无效菜单ID
}MENUID_ENUM;

extern PMENUITEM g_pCurMenuItem;
extern const MENUITEM  Menu_Tab[];

static U8 m_byAutoCloseBLCounter=AUTOCLOSEBACKLIGHTCOUNTER;			// 自动关闭屏幕倒计时
static U8 m_byCancelMicphonePrioriCounter= CANCELMICPHONEPRIORITYCOUNTER;	// 取消话筒优先倒计时
CTaskManager	g_taskManager;
CTaskJobGroup	*g_pTodayJobGroup;
u16 g_wJobRunCounter=0;	// 事件执行倒计数(秒)
char sCurSongName[51];	// 当前乐曲名
BOOL g_bTaskChange= FALSE;			// 任务数据有改变否
//static BOOL	g_micinsert_flag= FALSE;
// const char sSpaceLine[33]="                                \0";
char str_buf[32]; 

#define	ROLLNAME_DELAY		4

queue_p	Timer_queue;

// ***************************************************************************************************************
typedef struct{
	u8 byValid;
	u8 resver1[3];
	char sPwd[4];
#if (GC_ZR & GC_ZR_BJ)
	CTaskJob job;
	u8 wBjFcType;
#endif
}SYSTEMPARAM;

static SYSTEMPARAM	sysParam;
void SaveSystemParam()
{
	sysParam.byValid= 0x55;
#if(_HAVE_FONTLIB_==FONT_EXTERN)	// 保存在sst25VF中
	sst25_SectorErase(sacSysConfiger);
	sst25_Write(sacSysConfiger, (u8 *)&sysParam, sizeof(SYSTEMPARAM));
#else
	STMFLASH_Write(sacSysConfiger, (u16 *)&sysParam, sizeof(SYSTEMPARAM)>>1);
#endif
}

void LoadSystemParam()
{
#if(_HAVE_FONTLIB_==FONT_EXTERN)	// 保存在sst25VF中
	sst25_Read(sacSysConfiger, (u8 *)&sysParam, sizeof(SYSTEMPARAM));
#else
	STMFLASH_Read(sacSysConfiger, (u16 *)&sysParam, sizeof(SYSTEMPARAM)>>1);
#endif
	if(sysParam.byValid!=0x55)
	{
		sysParam.byValid= 0x55;
		tjInit(&sysParam.job);
		sysParam.job.m_Head.bySourceType = stAUX;
		sysParam.job.m_Head.byTimeType = tttBj;
		sysParam.wBjFcType = 0;
		memset(sysParam.sPwd,  '6', 4);
	}
}

// =================================================================================
extern void dspSysDate(void);
extern void dspTunerFreq(void);
extern void dspTunerSingle(void);
extern void dspPlayTime(void);
extern void dspSysTime(void);
extern void dspPlayerState(void);
extern void DspSongName(void);
extern void dspDesktop(struct _TAGMENUITEM *pItem);
// 显示事件播放时间
extern void dspJobRuntime(void);
	
extern BOOL sd_IsValid(void);
extern void menu_UpdateTodayTask(void);
extern void dspJobRuntime(void);	// 显示事件播放时间
extern void dspDesktopTask(void);
extern void dspUSBDiskStatus(void);
extern void dspMemoryStatus(void);

extern void vs1003_Init(void);
extern void kb_Init(void);

static u8 m_usbStatus_change=FALSE;

// bool usb_disk_Isready=FALSE;

#ifndef	USB_INT_SUCCESS
#define	USB_INT_SUCCESS		0x14			/* USB事务或者传输操作成功 */
#define	USB_INT_CONNECT		0x15			/* 检测到USB设备连接事件 */
#define	USB_INT_DISCONNECT	0x16			/* 检测到USB设备断开事件 */
#define	USB_INT_BUF_OVER	0x17			/* USB控制传输的数据太多, 缓冲区溢出 */
#define	USB_INT_USB_READY	0x18			/* USB设备已经被初始化（已分配USB地址） */
#define	USB_INT_DISK_READ	0x1D			/* USB存储器读数据块, 请求数据读出 */
#define	USB_INT_DISK_WRITE	0x1E			/* USB存储器写数据块, 请求数据写入 */
#define	USB_INT_DISK_ERR	0x1F			/* USB存储器操作失败 */
#endif


//  根据当前任务状态关闭功放电源
void CloseAmpPower()
{
	u32 nCurSec;
	CTaskJob	*pJob;
#if 1 // 2014-08-22 15:42
  pJob= ttGetNextTask(&g_taskManager.m_lpTodayTask);
  if(pJob)
	{
		nCurSec= MyTimeToSecond(&g_SysTime);
		if(MyTimeToSecond(tjGetStartTime(pJob))-nCurSec<1200)
				return;
	}
	audio_AmpPowerOpen(FALSE);
#else
	int nJobID;
	u8 nJobCount;

	// if(g_micinsert_flag) return;
	nJobID= ttGetNextTaskID(&g_taskManager.m_lpTodayTask);
	if(nJobID>=0)
	{
		nJobCount= ttGetCount(&g_taskManager.m_lpTodayTask);
		if(nJobID<=nJobCount)
		{
			if(ttGetJobState(&g_taskManager.m_lpTodayTask, nJobID-1)<tsTaskDone)
			{
				pJob= ttGetNextTask(&g_taskManager.m_lpTodayTask);
				nCurSec= MyTimeToSecond(&g_SysTime);
				if(MyTimeToSecond(tjGetStartTime(pJob))-nCurSec<1200)
					return;
			}
		}
	}
	audio_AmpPowerOpen(FALSE);
#endif
}

void menu_ClrClient()
{
	lcm_ClrLine(3, 4,  133, 1, 0x00);
	lcm_ClrLine(3, 5,  133, 3, 0xC0);
	// lcm_ClrLine(3, 7,  133, 1, 0x0F);
	//lcm_Putstr(3, 40, sSpaceLine, 22, 0);
	//lcm_Putstr(3, 40, sSpaceLine, 22, 0);
	//lcm_Putstr(3, 40, sSpaceLine, 22, 0);
}
// ===========================================================================================
void dspMicValue(u16 wValue)
{
	strfmt(str_buf, "Mic: %d", wValue);
	lcm_Putstr(120, 16, str_buf, 12, 0);
}

// 判断nTime-Src>nTimeout?有数据溢出检测
BOOL IsTimeout(u32 nTime, u32 nSrc, u32 nTimeout)
{
	u32 nTmp;
	if(nTime<nSrc)	// nTime已溢出
	{
		nTmp =nTime+ ((u32)-1)-nSrc;
		return (BOOL)(nTmp-nSrc>nTimeout);
	}
	return (BOOL)(nTime-nSrc>nTimeout);
}

static BOOL m_bySourceMute= FALSE;

#if (!(GC_ZR&GC_ZR_GPIO))
// 功放输出电平检测及显示
void dspAmplifierMeter()
{
	// static u32 dwCloseSourceCounter;
	// if(audio_GetAmpPowerState())
	{
		if(ADC1_Ok)
		{
			meter_SetData(adc_ReadAmpSingle());
			ADC1_Ok= FALSE;
			adc1_Start(TRUE);
			// DMA1ReConfig();
		}
		// if(g_micinsert_flag)
		{
			if(adc_ReadMicSingle())
			{
				if(m_bySourceMute==FALSE)
				{
					audio_setSourceMute(TRUE);
					m_bySourceMute= TRUE;
				}
				m_byCancelMicphonePrioriCounter= CANCELMICPHONEPRIORITYCOUNTER;
			}/*else
				if(m_byOldSrc&&IsTimeout(dwTickCount, dwCloseSourceCounter, 10000))
				{
					audio_setSource(m_byOldSrc);
					m_byOldSrc= srcInvaild;
				} */
		}
	}
}
#endif
static void MicphonePrioriDo()
{
	if(m_byCancelMicphonePrioriCounter)
	{
		m_byCancelMicphonePrioriCounter--;
		if((m_byCancelMicphonePrioriCounter==0)&&m_bySourceMute)
		{
			audio_setSourceMute(FALSE);
			m_bySourceMute= FALSE;
		} 
	}
}

/*
// 话筒插入控制
void MicphoneInsert()
{
	if(mic_IsValid())
	{
		if(g_micinsert_flag==FALSE)
		{
			g_micinsert_flag= TRUE;
		}
	}else
	{
		if(g_micinsert_flag)
		{
			g_micinsert_flag= FALSE;
			if(m_byOldSrc)
			{
				audio_setSource(m_byOldSrc);
				m_byOldSrc= srcInvaild;
			}
		}
	}
}
*/

static void BackLightDo()
{
	if(m_byAutoCloseBLCounter)
	{
		m_byAutoCloseBLCounter--;
		if(m_byAutoCloseBLCounter==0)
			lcm_BackLightCtrl(FALSE);
	}
}

static void JobRunCounterDo()
{
	if(g_wJobRunCounter)
	{
		g_wJobRunCounter--;
		if(g_pCurMenuItem->byMenuID== MENUID_DESKTOP)
		{
			dspJobRuntime();
			 if(g_wJobRunCounter==0)
			{
				CloseAmpPower();
				if(audio_getSource()==srcMP3)
				{
					player_Stop();
					//sCurSongName[0]= '\0';
					//DspSongName();
				}
				audio_setSource(srcInvaild);
				dspDesktopTask();
			}
		}
	}
}

static void AutoTaskDo()
{
	int nJobIndex, i;
	CTaskJob		*pJob;
	TTaskState	tsState;

	nJobIndex= ttGetNextTaskID(&g_taskManager.m_lpTodayTask);
	if(nJobIndex>0)
	{
		for(i=nJobIndex;i<=ttGetCount(&g_taskManager.m_lpTodayTask);i++)
		{ 
			tsState= ttTaskState(&g_taskManager.m_lpTodayTask, MyTimeToSecond(&g_SysTime), i-1);
			if(tsState)
			{
				pJob= ttGetNextTask(&g_taskManager.m_lpTodayTask);
				if(pJob)
				{
					switch(tsState)
					{
					case tsTaskStart:
						g_wJobRunCounter= 0;
						audio_AmpPowerOpen(TRUE);
						break;
					case tsTaskPauseed:
						break;
					case tsTaskRunning:
						audio_setSource(srcInvaild);
						audio_AmpPowerOpen(TRUE);
						switch(tjGetSourceType(pJob))
						{
						case stTuner:
							tuner_setBand(tjGetTunerBand(pJob));
							tuner_setFreq(tjGetTunerFreq(pJob));
							audio_setSource(srcTuner);
							break;
						case stSongFile:
							player_AutoPlayMem(tjGetSongID(pJob), sCurSongName);
							audio_setSource(srcMP3);
							break;
						case stAUX:
							audio_setSource(srcAux);
							break;
						case stUSB:
							player_AutoPlayUSB(sCurSongName);
							audio_setSource(srcMP3);
							break;
						}
						audio_SetAllPortState(pJob->mPortState);
						g_wJobRunCounter= tjGetPlayLength(pJob)+1;
						break;
					case tsTaskDone:
						audio_SetAllPortState(0);	// 关闭所有分区
            if(g_wJobRunCounter)
            {
  						CloseAmpPower();
            }
						g_wJobRunCounter= 0;
						if(audio_getSource()==srcMP3)
						{
							player_Stop();
							sCurSongName[0]= '\0';
							DspSongName();
						}
						audio_setSource(srcInvaild);
						break;
					}
					dspDesktopTask();
					if((tsState==tsTaskRunning)&&(audio_getSource()==srcMP3))
					{
						DspSongName();
					}
				}
			}
		}
	}
}


static void SDChangeDo()
{
	if(SD_changed)
	{
		DelayMS(200);
		if(sd_IsInserted())	// 卡插入
		{
			if(sd_insert_flag==FALSE)
			{
				f_mount(MMC, &fs_sd);
				sd_insert_flag=TRUE;
				if(sd_IsValid())
				{
					sd_valid_flag= TRUE;
					if(tmLoadFromFile(&g_taskManager, sTaskFile))
					{
						menu_UpdateTodayTask();
						if(g_pCurMenuItem->byMenuID==MENUID_DESKTOP)
						{
							dspDesktopTask();
							SD_changed= FALSE;
							return ;
						}
					}
					if(g_pCurMenuItem->byMenuID==MENUID_PLAYMEM)
					{
						// player_UpdateSongDir(MMC);
						player_PlayMem(0, sCurSongName);
						DspSongName();
					}
				}
			}
		}else
		{
			if(sd_insert_flag==TRUE)
			{
				sd_insert_flag= FALSE;
				sd_valid_flag= FALSE;
         		if(player_GetCurDevice()==MMC)
					player_Stop();
				player_RemoveSongDir(MMC);
				tmResetTaskData(&g_taskManager);
				if(g_pCurMenuItem->byMenuID==MENUID_PLAYMEM)
				{
					sCurSongName[0]= '\0';
					DspSongName();
				}
				if(g_pCurMenuItem->byMenuID==MENUID_DESKTOP)
					dspDesktopTask();
				f_mount(MMC, NULL);
			}
		}
		dspMemoryStatus();
		SD_changed= FALSE;
	}
}

static void USBChangeDo()
{
	static u32 nDiskErrCnt=0;

	switch(ch375_GetUSBStatus())
	{
	case USB_INT_CONNECT:
   		nDiskErrCnt=0;
		m_usbStatus_change=(BOOL)(usb_insert_flag==FALSE);
		usb_insert_flag= TRUE;
   		break;
	case USB_INT_DISCONNECT:
   		nDiskErrCnt=0;
		m_usbStatus_change= usb_insert_flag;
		usb_insert_flag= FALSE;
   	  	break;
   	case USB_INT_DISK_ERR:
   		nDiskErrCnt++;
   		if(player_GetCurDevice()==USB)
				player_Stop();
    	if((nDiskErrCnt>5)&&usb_disk_Isready)
    	{
			m_usbStatus_change= TRUE;
			usb_insert_flag= FALSE;
			ch375_Reset();
    	}
		break;
	}
	if(m_usbStatus_change)
	{
		if(usb_disk_Isready)	// USB设备移除
		{
			usb_disk_Isready= FALSE;
	        if(player_GetCurDevice()==USB)
  				player_Stop();
			f_mount(USB, NULL);
			player_RemoveSongDir(USB);
			if(g_pCurMenuItem->byMenuID==MENUID_PLAYUSB)
			{
				dspUSBDiskStatus();
			}
		}else	// USB设备插入
		{
			usb_disk_Isready= ch375_DiskIsReady();
			if(usb_disk_Isready)
			{
				f_mount(USB, &fs_usb);
				if(g_pCurMenuItem->byMenuID==MENUID_PLAYUSB)
				{
					menu_UDiskInsert();
 					dspUSBDiskStatus();
				}
			}else
    		{
         		 // U盘数据错误
          
        	}
		}
		m_usbStatus_change= FALSE;
	}
}

// 相关状态显示
static void ShowStatus()
{
	switch(g_pCurMenuItem->byMenuID)
	{
	case MENUID_PLAYTUNER:
		dspTunerSingle();
		break;
	case MENUID_PLAYUSB:
		if(usb_disk_Isready)
		{
			dspPlayTime();
			dspPlayerState();
		}
		break;
	case MENUID_PLAYMEM:
		if(sd_valid_flag)
		{
			dspPlayTime();
			dspPlayerState();
		}
		break;
	}
	#if(!(GC_ZR&GC_ZR_GPIO))
	dspAmplifierMeter();
	#endif
}

static void PlayerDO()
{
	switch(player_PlayDo())
	{
	case ERR_PLAY_STOPPED:
		if(player_getState()==ERR_PLAY_PLAYING)
			switch(Player.RepeatMode)
			{
			case REPEAT_NONE:	// 播音完毕
				player_Stop();
				sCurSongName[0]= 0;
				DspSongName();
				break;
			default:	
				player_Next(sCurSongName);
				DspSongName();
				break;
			}
		break;
	case ERR_PLAY_ERROR:
		if(0)//player_GetCurDevice()==USB)
		{
			player_Stop();
			f_mount(1, NULL);
			usb_disk_Isready= FALSE;
		}
    	break;
	}
}
// ===========================================================================================

// 写时执行函数列表

void menu_TimerQueueInit()
{
	Timer_queue= add_queue(NULL);
}

void menu_LCMOn()
{
	lcm_BackLightCtrl(TRUE);
	m_byAutoCloseBLCounter= AUTOCLOSEBACKLIGHTCOUNTER;
}


void menu_UpdateTodayTask()
{
	tmUpdateTodayTask(&g_taskManager, &g_SysDate);
	g_pTodayJobGroup= &(g_taskManager.m_lpTodayTask.m_lpTodayTask);
	ttUpdateTaskState(&g_taskManager.m_lpTodayTask, &g_SysTime);
}

// =================================================================================
BOOL sd_IsValid()
{
	DIR dj;

	return (BOOL)(f_opendir(&dj, sSongPaths[0])==FR_OK);
}

void menu_Init()
{
	meter_Init();
	dspWelcome();
	ch375_Init();
	sd_Config(); //初始化IO
	vs1003_Init();
	kb_Init();
	if(usb_insert_flag)
		m_usbStatus_change= TRUE;
	vs1003_SineTest();
	player_Init();
	audio_Init();
	tuner_LoadParam();
	// audio_setSource(srcTuner);
	tuner_Init(tbFM, 0);
	tuner_setMute(TRUE);
	// audio_setSource(srcMP3);
	
	g_SysDate.wYear= 2015;
	g_SysDate.byMonth= 04;
	g_SysDate.byDay= 24;
	g_SysTime.byHour= 14;
	g_SysTime.byMinute= 16;
	g_SysTime.bySecond= 10;
	g_SysWeek= MyDateOfWeek(&g_SysDate);
	rtc_Init();
	// rtc_GetDateTime(&g_SysDate, &g_SysTime, &g_SysWeek);
	LoadSystemParam();

	tmInit(&g_taskManager);
	menu_TimerQueueInit();
	SDChangeDo();
}

void menu_Start()
{
	dspDesktop(NULL);
	// g_pCurMenuItem->DisplayUIFunc(g_pCurMenuItem);
}

// 缺省按键处理函数
unsigned int keydo_Default(struct _TAGMENUITEM *pNewMenu, unsigned char KeyCode, unsigned char keyMask)
{
	// PMENUITEM  pNewMenu= (PMENUITEM)pItem;
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
	case VK_ENTER:
		return TRUE;
	case VK_ESC:	// 按下菜单键，进入菜单
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return TRUE;
	}
	pNewMenu->DisplayUIFunc(pNewMenu);
	return TRUE;
}

static BOOL m_bCanKeyDo=TRUE;		// 允许执行按键
#if(GC_ZR & GC_ZR_PLAY)
void menu_StopPlay(void);
void dspPlayYj(struct _TAGMENUITEM *pItem);
void dspPlayBj(struct _TAGMENUITEM *pItem);
#endif

void menu_KeyPress(unsigned char keyCode, unsigned char keyMask)
{
#if(GC_ZR & GC_ZR_PLAY)//XXX
	
	if(keyCode&0x80) {
	if (keyCode & 0x01  ) {//VK_BJ_IN
		if (g_pCurMenuItem->byMenuID != MENUID_PLAYBJ) {
			if (sd_valid_flag) //存在SD卡
			{
				if (sysParam.job.m_Head.bySourceType == stSongFile) {
					audio_setSource(srcMP3); //设置音源为内存MP3
					player_PlayMemBj(sysParam.job.m_Head.srtSourceState.srtSong.wSongID > 0 ? sysParam.job.m_Head.srtSourceState.srtSong.wSongID - 1 : 1, sCurSongName);	//单曲循环 报警音乐		第一个参数为文件编号
				}
				else if (sysParam.job.m_Head.bySourceType == stAUX) {
					audio_setSource(srcAux);
				}
			}
			else
			{
				Player.byCurDev = MMC;   // 2014-10-21 添加，有可能如果MMC没有则可能出现问题!
				sCurSongName[0] = '\0';
			}
			audio_AmpPowerOpen(TRUE);
			audio_SetBjPortState(0xFFFF, sysParam.wBjFcType > 0 ? TRUE: FALSE);
			dspPlayBj(g_pCurMenuItem);
		}
		m_byAutoCloseBLCounter= AUTOCLOSEBACKLIGHTCOUNTER;
		lcm_BackLightCtrl(TRUE);
		return;
	}
	else if (keyCode&0x04) {//VK_JINJI_KEY
		if (g_pCurMenuItem->byMenuID != MENUID_PLAYYJ) {
			menu_StopPlay();
			audio_setSource(srcInvaild);
			audio_AmpPowerOpen(TRUE);
			audio_SetAllPortState(0xFFFF);
			dspPlayYj(g_pCurMenuItem);
		}
		m_byAutoCloseBLCounter= AUTOCLOSEBACKLIGHTCOUNTER;
		lcm_BackLightCtrl(TRUE);
		return;
	}
	else if (keyCode&0x08) {
		
		if (g_pCurMenuItem->byMenuID == MENUID_PLAYYJ) {
			// audio_setSource(srcInvaild);
			audio_SetAllPortState(0x0);
			audio_AmpPowerOpen(FALSE);
			//任务执行时间为0
			g_wJobRunCounter=0;
			dspDesktop(g_pCurMenuItem);
			return;
		}
	}
	return;
}
#endif

	lcm_BackLightCtrl(TRUE);
	if(tuner_IsSearching())	// 收音自动搜索中
	{
		tuner_SearchStop();
		menu_Repaint();
		return;
	}
	if(m_bCanKeyDo==FALSE)
	{
		m_bCanKeyDo= (BOOL)(keyMask==0);
	}

	if (m_byAutoCloseBLCounter&&m_bCanKeyDo)
		if (g_pCurMenuItem->KeyDoFunc(g_pCurMenuItem, keyCode, keyMask) == FALSE)
			keydo_Default(g_pCurMenuItem, keyCode, keyMask);
	//if(m_byAutoReturnCounter)
	//	m_byAutoReturnCounter= AUTORETURNCOUNTER;

	m_byAutoCloseBLCounter= AUTOCLOSEBACKLIGHTCOUNTER;
}

void menu_KeyRelease()
{
  m_bCanKeyDo= TRUE;
}

void menu_KeyDo(unsigned char keyCode, unsigned char keyMask)
{
  if(keyCode)
    menu_KeyPress(keyCode, keyMask);
  else
    menu_KeyRelease();
}

void menu_UDiskInsert()
{
	// menu_LCMOn();
	menu_ClrClient();
	
	player_PlayUSB(LoadUSBPlayPosition(), sCurSongName);
	DspSongName();
}

#if (GC_ZR & GC_ZR_REMOTE)

static BOOL m_remoteState = FALSE;
static u16 yk_time = 0;

void openRemoteTask(CRemoteTask *remoteTask) {
	CRemoteTask m_remoteTask;
	CTaskJob *pJob;
	rtAssign(&m_remoteTask, remoteTask);
	pJob = &m_remoteTask.m_Job;
	if (m_remoteTask.m_funcValue < 6) {//func
		if (!m_remoteState)
			return;
		switch (m_remoteTask.m_funcValue)
		{
		case 0:
			if (audio_getSource() == srcMP3) {
				if (player_getState() == ERR_PLAY_PLAYING) {
					player_Pause();
					
					yk_time = g_wJobRunCounter;
				}
				else if (player_getState() == ERR_PLAY_PAUSEED) {
					player_Play();
					yk_time = 0;
				}
			}
			break;
		case 1:
			g_wJobRunCounter = 0;
			yk_time = 0;
			break;
		default:
			break;
		}
	}
	else {
		if (m_remoteState)
			return;
		audio_setSource(srcInvaild);
		audio_AmpPowerOpen(TRUE);
		switch (tjGetSourceType(pJob))
		{
		case stTuner:
			tuner_setBand(tjGetTunerBand(pJob));
			tuner_setFreq(tjGetTunerFreq(pJob));
			audio_setSource(srcTuner);
			break;
		case stSongFile:
			player_AutoPlayMem(tjGetSongID(pJob), sCurSongName);
			audio_setSource(srcMP3);
			break;
		case stAUX:
			audio_setSource(srcAux);
			break;
		case stUSB:
			player_AutoPlayUSB(sCurSongName);
			audio_setSource(srcMP3);
			break;
		}
		audio_SetAllPortState(pJob->mPortState);
		g_wJobRunCounter = tjGetPlayLength(pJob) + 1;
		m_remoteState = TRUE;
	}
}

void openRemoteTaskGroup(CRemoteTaskGroup *remoteTaskGroup, int key) {
	int i = 0, count = 0; 
	CRemoteTask *remoteTask;
	count = rtgGetCount(remoteTaskGroup);
	//UART_PutChar(USART2,key);
	for (i = 0; i < count; i++) {
		remoteTask = rtgGetAt(remoteTaskGroup, i);
		//UART_PutChar(USART2,remoteTask->m_Key);
		//UART_PutChar(USART2,key);
		if (NULL != remoteTask) {
			if (remoteTask->m_Key == key) {
				openRemoteTask(remoteTask);
				m_byAutoCloseBLCounter= AUTOCLOSEBACKLIGHTCOUNTER;
				lcm_BackLightCtrl(TRUE);
				break;
			}
		}
	}
	 
	if (key == 1111110) {
			g_wJobRunCounter = 0;
			yk_time = 0;
	}
	if (key == 888888)
	{
		menu_StopPlay();
		audio_SetAllPortState(0xffff);
		audio_AmpPowerOpen(TRUE);
		audio_setSource(srcPSTN);
		m_byAutoCloseBLCounter= AUTOCLOSEBACKLIGHTCOUNTER;
		lcm_BackLightCtrl(TRUE);
		g_wJobRunCounter = 3600 + 1;
		m_remoteState = TRUE;
	}
}
extern u32 DH_Key;
extern u8 YK_Key; //KEY:1111  
void AutoRemoteTaskDo() {
	char buf[20];
	CRemoteTaskGroup *remoteTaskGroup;	
		//UART_PutChar(USART2,YK_Key);	
	sprintf(buf,"cc = %d",DH_Key);
	//UART_PutStr(USART2,buf,20);
	if (YK_Key <12) {
		remoteTaskGroup = &g_taskManager.m_lpYkTasks;		
		openRemoteTaskGroup(remoteTaskGroup, YK_Key);
		YK_Key = 0xff;
	}
	else if (DH_Key <20000000) {
		remoteTaskGroup = &g_taskManager.m_lpPhoneTasks;		
		openRemoteTaskGroup(remoteTaskGroup, DH_Key);
		DH_Key = 0xffffffff;
	}
	if (m_remoteState) {
		if (g_wJobRunCounter == 0) {
			audio_SetAllPortState(0x0);
			audio_setSource(srcInvaild);
			menu_StopPlay();
			CloseAmpPower();
			m_remoteState = FALSE;
			yk_time = 0;
			lcm_Putstr(87,32,"        ",0,0);
			lcm_Putstr(3,32,"        ",0,0);
			lcm_Putstr(3,50,"                      ",0,0);
			dspDesktopTask();
		}
		else {
			if(yk_time>0) {
				g_wJobRunCounter =yk_time;
			}
			lcm_Putstr(3,32,"正在播音",0,0);
			lcm_Putstr(3,50,"正在执行远程控制任务  ",0,0);
		}
	}
	else{
		dspDesktopTask();
	}
}
#endif

void menu_OnTimer()
{
	static TMyDate g_OldDate = { 0, 0, 0 };

	if (Ms_8)
	{
#if (GC_ZR & GC_ZR_REMOTE)
		if (g_pCurMenuItem->byMenuID == MENUID_DESKTOP)  // && ((YK_Key <12) || (DH_Key <20000000)))
		{
			if((YK_Key <12) || (DH_Key <20000000))
			{
				AutoRemoteTaskDo();
			}
		}
		else //其他界面 遥控码 电话码 置为初始值
		{
			YK_Key=0xff;
			DH_Key=0xffffffff;
		}
#endif
		if (GetSysTime())	// 新的1秒了
		{
			rtc_GetDate(&g_SysDate, &g_SysWeek);
			// if((g_SysTime.byHour==0)&&(g_SysTime.byMinute==0)&&(g_SysTime.bySecond==0))	 // 新的一天开始
			if ((g_SysDate.wYear != g_OldDate.wYear) || (g_SysDate.byMonth != g_OldDate.byMonth) || (g_SysDate.byDay != g_OldDate.byDay))// 日期变化
			{	// 此处可以考虑重新启动以利于系统运行
				// 重新获取任务数据
				menu_UpdateTodayTask();
				if (g_pCurMenuItem->byMenuID == MENUID_DESKTOP)
				{
					dspSysDate();
					dspDesktopTask();
				}
				memcpy(&g_OldDate, &g_SysDate, sizeof(TMyDate));
			}
			if (g_pCurMenuItem->byMenuID == MENUID_DESKTOP)
			{
				dspSysTime();
#if (GC_ZR & GC_ZR_REMOTE)
				AutoRemoteTaskDo();
				if (!m_remoteState)
					AutoTaskDo();
#else
				AutoTaskDo();
#endif
				BackLightDo();
			}
#if (GC_ZR & GC_ZR_REMOTE)
			else {
				m_remoteState = FALSE;
			}
#endif
			MicphonePrioriDo();
			JobRunCounterDo();
		}
		Ms_8 = 0;
	}

	if (Ms_128)
	{
		if (tuner_SearchDo())
			dspTunerFreq();
		// MicphoneInsert();
		SDChangeDo();
		USBChangeDo();
		ShowStatus();
		Ms_128 = 0;
		lcm_UpdateLCMData();
	}
	if (Ms_256)
	{
		queue_do(Timer_queue);
		Ms_256 = 0;
	}
	PlayerDO();
}

void menu_Repaint()
{
	g_pCurMenuItem->DisplayUIFunc(g_pCurMenuItem);
}

// ***************************************************************************************************************
void menu_StopPlay()
{
  player_Stop();
  sCurSongName[0]= 0;
	DspSongName();
}
// --------------------------------------------------------------------------------------------------------------
void dspWelcome()
{
	lcm_Putstr(2,0,sProduct,12,0);
	lcm_Putstr(159, 0, PRODUCT_VER, 5, 0);
	lcm_Putstr(54,16, sModal, 14, 0);
	lcm_Putstr(36,32, sSystemIniting,20,1);
	lcm_Putstr(46,48,sCompany,24,0);
	lcm_UpdateLCMData();
}


void dspSysDate()
{
  if(g_pCurMenuItem->byMenuID!=MENUID_DESKTOP) return;
	strfmt(str_buf, sDateFmt, g_SysDate.wYear -2000, g_SysDate.byMonth, g_SysDate.byDay );
	lcm_Putstr(3, 0, str_buf, 8, 0);

	// 显示星期
	strfmt(str_buf, "%s%s", sWeek, sWeekday[g_SysWeek-1]);
	lcm_Putstr(3, 16, str_buf, 6, 0);
}

void dspSysTime()
{
	strfmt(str_buf, sTimeFmt, g_SysTime.byHour, g_SysTime.byMinute, g_SysTime.bySecond);
	lcm_Putstr32(63, 0, str_buf, 8, 0);
}

void dspShortDate(u8 x, u8 y, PShortDate dt)
{
	strfmt(str_buf, "%02d-%02d", dt->byMonth, dt->byDay);
	lcm_Putstr(x, y, str_buf, 5, 0);
}

void dspTime(u8 x, u8 y, PMyTime tm)
{
	strfmt(str_buf, sTimeFmt, tm->byHour, tm->byMinute, tm->bySecond);
	lcm_Putstr(x, y, str_buf, 8, 0);
}

void dspDrawTitle()
{
	lcm_Linexy(1,14,190,14);
	lcm_Linexy(1,13,190,13);
	lcm_Putstr(g_pCurMenuItem->Title_xPos, 0, g_pCurMenuItem->Title_szName, g_pCurMenuItem->Title_len, 0);
}

void dspDrawRect()
{
	lcm_Linexy(0,14,0,63);
	lcm_Linexy(1,63,190,63);
	lcm_Linexy(191,14,191,63);
}

// 显示手动播音界面框架
void dspAmplifierState()		//显示功放状态
{
	lcm_Putstr(141, 32, sAmplifer, 8, 0);
	if(1)//Amp_State==1)
		lcm_Putstr(141, 48, sOpen, 8, 0);
	else
		lcm_Putstr(141, 48, sClose, 8, 0);
}

u8 GetPortStateString(char *str);
void dspManualFrame()
{
	dspDrawTitle();
	dspDrawRect();
	
	lcm_Linexy(1, 30, 190, 30);
	lcm_Linexy(138 , 30, 138, 62);
	lcm_Putstr(3, 16, sSource, 4, 0);
	lcm_Putstr(30, 16, sSourceName[g_pCurMenuItem->byMenuID-MENUID_PLAYTUNER], 0, 0);

	// 分区
#if (GC_ZR & GC_ZR_FC)
	lcm_Putstr(130, 16, sPort, 4, 0);
	lcm_Putstr(154, 16, str_buf, GetPortStateString(str_buf), 1);
#else
	lcm_Putstr(136, 16, sPort, 4, 0);
	lcm_Putstr(166, 16, str_buf, GetPortStateString(str_buf), 1);
#endif
	dspAmplifierState();
}

void dspTunerFreq()
{
	lcm_Putstr(22, 32, tuner_getFreqString(0xFF, 0, str_buf), 10, 0);
}

void dspTunerSingle()
{
	if (tuner_getStatus(NULL)>tuner_getValidSingle(0xFF))
		lcm_PutPlayerdot(123, 32, 0x0B, 0);
	else
		lcm_PutPlayerdot(123, 32, 0x0A, 0);
}

// ---------------------------------------------------------------------------------------------
void dspPlayerState()
{
	static u8 const playerstate_tab[]={2, 2, 2, 1, 0};
	lcm_PutPlayerdot(2, 48, playerstate_tab[player_getState()], 0);
}

void dspPlayTime(void)
{
	TMyTime	tt;
	SecondToMyTime(vs1003_GetDecodeTime(), &tt);
	strfmt(str_buf, sTimeFmt, tt.byHour, tt.byMinute, tt.bySecond);
	lcm_Putstr(87, 48, str_buf, 8, 0);
}

rollstr_t	currollsongname;
queue_p  show_cursong;
void show_curSongName()
{
	if((g_pCurMenuItem->byMenuID==MENUID_DESKTOP)||(g_pCurMenuItem->byMenuID==MENUID_PLAYMEM)||(g_pCurMenuItem->byMenuID==MENUID_PLAYUSB))
		lcm_RollShowStr(&currollsongname);
}

void DspSongName()
{
	U8 i=0, len;
	char name[51]={0};

	remove_queue(Timer_queue, show_cursong);
	len= strlen(sCurSongName);
	 if(g_pCurMenuItem->byMenuID==MENUID_DESKTOP)
	 {
	 	currollsongname.col= 57;
		currollsongname.showlen= 13;
		currollsongname.row=48;
	 }else
	 {
	 	currollsongname.col= 3;
		currollsongname.showlen= 22;
		currollsongname.row=32;
	 }
	 currollsongname.delay= 5;
	strcpy(name, sCurSongName);
	for(i=len;i<currollsongname.showlen;i++) name[i]=' ';			// 左靠齐
	name[i]=0;

	// 从第一个位置开始显示一次
	if((g_pCurMenuItem->byMenuID==MENUID_DESKTOP)||(g_pCurMenuItem->byMenuID==MENUID_PLAYMEM)||(g_pCurMenuItem->byMenuID==MENUID_PLAYUSB))
		lcm_Putstr(currollsongname.col, currollsongname.row, name, currollsongname.showlen, 0);

	// remove_queue(Timer_queue, show_cursong);
	if(len>currollsongname.showlen)	// 名称较长时才需要
	{
		currollsongname.str= sCurSongName;
		currollsongname.instead= FALSE;
		currollsongname.pos=0;
		currollsongname.delay= ROLLNAME_DELAY;
		show_cursong= add_queue(Timer_queue);
		if(show_cursong)
		{
			show_cursong->func= show_curSongName;
		}
	}
}

// --------------------------------------------------------------------------------------------------------------
void dspDefault(struct _TAGMENUITEM *pMenu)
{
	// PMENUITEM pMenu= (PMENUITEM)pItem;
	if(pMenu->byMenuID!= g_pCurMenuItem->byMenuID) 
	{
		lcm_Clr();
		g_pCurMenuItem= pMenu;
		dspDrawTitle();
		dspDrawRect();
	}
	lcm_Putstr(48, 32, sNoFunc, 16, 0);
}

// --------------------------------------------------------------------------------------------------------------
static u8 _DateIsValid(PMyDate dt)
{
	u8 byDays;
	if(dt->wYear<2013)
	{
		dt->wYear= 2013;
		return 1;
	}
	if(dt->wYear>2112)
	{
		dt->wYear= 2112;
		return 1;
	}
	if(dt->byMonth<1)
	{
		dt->byMonth= 1;
		return 2;
	}
	if(dt->byMonth>12)
	{
		dt->byMonth= 12;
		return 2;
	}
	if(dt->byDay<1)
	{
		dt->byDay= 1;
		return 3;
	}
	byDays= GetDayCount(dt->wYear, dt->byMonth);
	if(dt->byDay>byDays)
	{
		dt->byDay= byDays;
		return 3;
	}
	return 0;
}

static u8 _TimeIsValid(PMyTime tm)
{
	if(tm->byHour>23)
	{
		tm->byHour=23;
		return 4;
	}
	if(tm->byMinute>59)
	{
		tm->byMinute= 59;
		return 5;
	}
	if(tm->bySecond>59)
	{
		tm->bySecond= 59;
		return 6;
	}
	return 0;
}

// --------------------------------------------------------------------------------------------------------------
void dspJobRuntime()	// 显示事件播放时间
{
	TMyTime	tm;
	if(g_wJobRunCounter==0) return;
	SecondToMyTime(g_wJobRunCounter, &tm);
	dspTime(87, 32, &tm);
}

const char sTaskDone[]= "今日播音完毕！";

char * GetDesktopSourceString(CTaskJob *tj, char *szRes, u8 len)
{
	char lfn[51];
	u8 nSrc= tjGetSourceType(tj), byBand;
	if((nSrc==stInvalid)||(nSrc> stMicphone))
	{
		szRes[0]= 0;
		return szRes;
	}
	strfmt(szRes, "%s ", sSourceName[nSrc-1]);
	switch(nSrc)
	{
	case stSongFile:
		if(FindSongno(sSongPaths[0], tjGetSongID(tj), TRUE, NULL, lfn, NULL))
		 strncat(szRes, lfn, 17);
		break;
	case stTuner:
		byBand= tjGetTunerBand(tj);
		strfmt(lfn, "     %s", tuner_getBandName(byBand));
		strcat(szRes, lfn);
		strcat(szRes, tuner_getFreqString(byBand, tjGetTunerFreq(tj), lfn));
		break;
	}
	for(nSrc=strlen(szRes); nSrc<len; nSrc++)
		szRes[nSrc]= ' ';
	szRes[len]= 0; 
	return szRes;
}

void dspDesktopTask()
{
	CTaskJob	*pJob;
	u8 nCount= tjgGetCount(g_pTodayJobGroup);
	int nJob;
	menu_ClrClient();
	lcm_Putstr(143, 32, "今日任务", 8, 0);
	nJob= ttGetNextTaskID(&(g_taskManager.m_lpTodayTask));
	if(nJob<1)
		nJob= nCount+1;
	strfmt(str_buf, "%03d/%03d", nJob-1, nCount);
	lcm_Putstr(146, 48, str_buf, 7, 0);
	if(sd_insert_flag)
	{
		if(sd_valid_flag)
		{
			if(nCount==0)
			{
        sCurSongName[0]= 0;
        DspSongName();
				lcm_Putstr(24, 40,  sTodayNoJob, 0, 0);
				return ;
			}
			pJob= ttGetNextTask(&(g_taskManager.m_lpTodayTask));
			if((pJob==NULL)&&(g_wJobRunCounter==0))
			{
        sCurSongName[0]= 0;
        DspSongName();
				lcm_Putstr(30, 40, sTaskDone, 0, 0);
			}else
			{
				switch(g_taskManager.m_lpTodayTask.m_tsTaskState)
				{
				case tsTaskReady:
				case tsTaskStart:
					lcm_Putstr(3, 32, "即将播音", 0, 0);
					dspTime(87, 32, tjGetStartTime(pJob));
					break;
				case tsTaskRunning:
					lcm_Putstr(3, 32, "正在播音", 0, 0);
					dspJobRuntime();
					break;
				case tsTaskDone:
          sCurSongName[0]= 0;
          DspSongName();
					lcm_Putstr(30, 40, sTaskDone, 0, 0);
					return ;
				}
				lcm_Putstr(3, 48, GetDesktopSourceString(pJob, str_buf, 22), 22, 0);
			}
			return ;
		}
	}
	dspMemoryStatus();
}

void dspDesktop(struct _TAGMENUITEM *pItem)
{
	if((!g_pCurMenuItem)||(MENUID_DESKTOP!= g_pCurMenuItem->byMenuID) )
	{
		lcm_Clr();
		lcm_Linexy(0,30,0,63);
		lcm_Linexy(0,30,191,30);
		lcm_Linexy(191,31,191,63);
		lcm_Linexy(1,63,190,63);
		lcm_Linexy(138,31,138,62);
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_DESKTOP];
 		rtc_GetDateTime(&g_SysDate, &g_SysTime, &g_SysWeek);
    if(g_wJobRunCounter==0)
    {
	  	// 更新今日任务
		  menu_UpdateTodayTask();
	  	CloseAmpPower();
    }
 		dspSysDate();
	}
  dspDesktopTask();
	dspSysTime();
}

u32 keydo_Desktop(struct _TAGMENUITEM * pNewMenu, unsigned char KeyCode, unsigned char keyMask)
{
	// PMENUITEM pNewMenu;
	switch(KeyCode)
	{
	case VK_ENTER:	// 按下菜单键，进入菜单
		pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_PASSWORDINPUT];
		*((u16 *)pNewMenu->pParam)= 0;
		break;
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
		if(keyMask&0x01)	// 长按键进入一键播音
		{
			g_wJobRunCounter= 0;
			menu_StopPlay();
			pNewMenu= (PMENUITEM)&Menu_Tab[KeyCode];	// 取得新的菜单
			if(pNewMenu->InitFunc)
				pNewMenu->InitFunc(g_pCurMenuItem);
			audio_AmpPowerOpen(TRUE);
#if (GC_ZR & GC_ZR_FC)
			audio_SetAllPortState(0xFFFF);	
#else
			audio_SetAllPortState(0xFF);
#endif
			m_bCanKeyDo= FALSE;
		}
		else
			return TRUE;
		break;
	default:
		return TRUE;
	}
	pNewMenu->DisplayUIFunc(g_pCurMenuItem);
	return TRUE;
}

// --------------------------------------------------------------------------------------------------------------
#if (GC_ZR)
typedef union 	{
	u32 w;
	struct	{
		u32 wValue:28;
		u8 byInputDig:4;
	}sub;
} NUMINOUTPARAM;
#else
typedef union {
	u16 w;
	struct {
		u16 wValue : 14;
		u8 byInputDig : 2;
	}sub;
} NUMINOUTPARAM;
#endif

typedef struct {
	u8 byInited;
	u8 byRes;
#if (GC_ZR & GC_ZR_TUNER)
	u8 bySetChannel;
#endif
	NUMINOUTPARAM mInput; 
}PLAYTUNERPARAM;

void InitPlayeTuner(void *pItem)
{
	PLAYTUNERPARAM *pSet= (PLAYTUNERPARAM *)Menu_Tab[MENUID_PLAYTUNER].pParam;
	pSet->byInited= 0;
}

void dspChannel(u8 x, u8 y, u16 nChn, u8 instead)
{
	if(nChn==0)
	{
		lcm_Putstr(x, y, "     ", 5, instead);
		return ;
	}
	strfmt(str_buf, "CH %02d", nChn);
	lcm_Putstr(x, y, str_buf, 5, instead);
}

void dspPlayTuner(struct _TAGMENUITEM *pItem)
{
	PLAYTUNERPARAM *pSet= (PLAYTUNERPARAM *)Menu_Tab[MENUID_PLAYTUNER].pParam;
	if((!g_pCurMenuItem)||(MENUID_PLAYTUNER!= g_pCurMenuItem->byMenuID) )
	{
		lcm_Clr();
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_PLAYTUNER];
		if(pSet->byInited==0)
		{
			audio_setSource(srcTuner);
			pSet->mInput.w= 0;//xFFFF;
			pSet->byInited= 1;
			pSet->bySetChannel = 0;
		}
		dspManualFrame();
		dspChannel(64, 48, pSet->mInput.sub.wValue, 0);
	}

	lcm_Putstr(4, 32, tuner_getBandName(0xFF), 2, 0);
	dspTunerFreq();
	if (pSet->bySetChannel > 0) {
		lcm_Putstr(15, 48, sTunerSet, 14, 0);//15+12*7=99
		strfmt(str_buf, "%02d", pSet->bySetChannel);
		lcm_Putstr(105, 48, str_buf, 2, 1);
	}
}

// 数字输入处理,最大可输入4位数字
// byDigtal 0-3 0表示4位
// 输满byDigtal指定位数返回 TRUE, 否则返回FALSE
BOOL	NumInputProc(NUMINOUTPARAM *pInput, u8 byInput, u8 byDigtal)
{
	if(pInput->sub.byInputDig==0)
		pInput->sub.wValue = byInput;
	else
		pInput->sub.wValue = pInput->sub.wValue *10 +byInput;
	pInput->sub.byInputDig++;
	if( pInput->sub.byInputDig==byDigtal)
	{
		pInput->sub.byInputDig= 0;
		return TRUE;
	}	else
		return FALSE;
}

// byDigtal 0-3 0表示4位
void dspInputNum(u8 x, u8 y, NUMINOUTPARAM *pInput, u8 byDigtal, u8 instead)
{
	u8 i, n= pInput->sub.byInputDig==0?byDigtal:pInput->sub.byInputDig;
	char str[10];
	strfmt(str, "%%0%dd", n);
	strfmt(str_buf, str, pInput->sub.wValue);
	for(i=n; i<byDigtal; i++)
		str_buf[i]= '_';
	lcm_Putstr(x, y, str_buf, byDigtal, instead);
}

u32 keydo_PlayTuner(struct _TAGMENUITEM * pMenu, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_PLAYTUNER];
	PLAYTUNERPARAM *pSet= (PLAYTUNERPARAM *)pNewMenu->pParam;
	BOOL bRes;
	switch(KeyCode)
	{
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
		if((pSet->mInput.sub.byInputDig==0)||(pSet->mInput.w==0))
			return TRUE;
		if (pSet->bySetChannel > 0) 
			return TRUE;
	case VK_NUM0:
		if((keyMask&0x01)&&(pSet->mInput.sub.wValue==0))	// 
			return TRUE;
		if (pSet->bySetChannel > 0) 
			return TRUE;
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
		if (pSet->bySetChannel > 0) 
			return TRUE;
		if((pSet->mInput.sub.wValue==4)&&(KeyCode!=VK_NUM0))
			return TRUE;
		if(pSet->mInput.sub.byInputDig==0)	
			pSet->mInput.w= 0;
				
		bRes= NumInputProc(&pSet->mInput, KeyCode-1, 2);
		if((pSet->mInput.sub.byInputDig==1)&&(pSet->mInput.sub.wValue==4))	// 第一位输入4则直接选择CH40
		{
			pSet->mInput.sub.wValue= 40;
			pSet->mInput.sub.byInputDig= 0;
			bRes= TRUE;
		}
		dspInputNum(82, 48,  &pSet->mInput, 2, 0);
		if(bRes)
		{
			if((pSet->mInput.w>0)&&(pSet->mInput.w<=BANDSTATION_COUNT))
			{
				tuner_setCurrentFreq(0xFF, tuner_getStationFreq(0xFF, pSet->mInput.w-1));
				dspChannel(64, 48, pSet->mInput.w, 0);
				// pSet->mInput.w= 0;
				break;
			}
		}
		return TRUE;
	case VK_UP:
		if (pSet->bySetChannel == 0) {
			dspChannel(64, 48, 0, 0);
			if (tuner_FreqInc())
				break;
			else
				return TRUE;
		}
		else {
			if (pSet->bySetChannel < BANDSTATION_COUNT)
				pSet->bySetChannel++;
			else
				pSet->bySetChannel = 1;
			break;
		}
	case VK_DOWN:
		if (pSet->bySetChannel == 0) {
			dspChannel(64, 48, 0, 0);
			if (tuner_FreqDec())
				break;
			else
				return TRUE;
		}
		else {
			if (pSet->bySetChannel > 1) {
				pSet->bySetChannel--;
			}
			else
				pSet->bySetChannel = BANDSTATION_COUNT;
			break;
		}
	case VK_LEFT:
		// tuner_ChangeBand();
		if (pSet->bySetChannel == 0) {//未选台
			pSet->bySetChannel = 1;
		}
		break;
	case VK_RIGHT:
		if(keyMask&0x01)
			tuner_SearchStart();
		break;
	case VK_ENTER:	
		if(pSet->bySetChannel > 0) {
			tuner_saveFreq(pSet->bySetChannel - 1);
			lcm_Putstr(15, 48, "                 ", 0, 0);
			dspChannel(64, 48, pSet->mInput.w, 0);
			pSet->bySetChannel = 0;
			break;
		}
		pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_PORTCTRL];	// 取得新的菜单
		if(pNewMenu->InitFunc)
			pNewMenu->InitFunc(g_pCurMenuItem);
		break;
	case VK_ESC:	
		if(pSet->bySetChannel > 0) {
			lcm_Putstr(15, 48, "                 ", 0, 0);
			dspChannel(64, 48, pSet->mInput.w, 0);
			pSet->bySetChannel = 0;
			break;
		}
		audio_SetAllPortState(0x0);
		audio_setSource(srcInvaild);
#if (GC_ZR & GC_ZR_TUNER)
		tuner_SaveAll();
#endif
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	}
	pNewMenu->DisplayUIFunc(pMenu);
	return TRUE;
}

// --------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------
void dspMemoryStatus()
{
	if((g_pCurMenuItem->byMenuID==MENUID_DESKTOP)||(g_pCurMenuItem->byMenuID==MENUID_PLAYMEM))
	{
		menu_ClrClient();
		if(sd_insert_flag)
		{
			if(sd_valid_flag)
			{
				if(g_pCurMenuItem->byMenuID==MENUID_PLAYMEM)
				{
					dspPlayTime();
					dspPlayerState();
					DspSongName();
				}
			}else
			{
				lcm_Putstr(3, 32, sSDReadError, 0, 0);
				lcm_Putstr(76, 48, sSelfCheck, 0, 0);
			}
		}else
		{
			lcm_Putstr(3, 32, sNoSD, 0, 0);
			lcm_Putstr(76, 48, sSelfCheck, 0, 0);
		}
	}
}

typedef struct {
	u8 byInited;
}PLAYMEMPARAM;

void InitPlayMem(void *pItem)
{
	PLAYMEMPARAM *pSet= (PLAYMEMPARAM *)Menu_Tab[MENUID_PLAYMEM].pParam;
	pSet->byInited= 0;
}

void dspPlayMem(struct _TAGMENUITEM *pItem)
{
	PLAYMEMPARAM *pSet= (PLAYMEMPARAM *)Menu_Tab[MENUID_PLAYMEM].pParam;
	if((!g_pCurMenuItem)||(MENUID_PLAYMEM!= g_pCurMenuItem->byMenuID) )
	{
		lcm_Clr();
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_PLAYMEM];
		dspManualFrame();
		if(pSet->byInited==0)
		{
			audio_setSource(srcMP3);
			if(sd_valid_flag)
			{
				player_PlayMem(0, sCurSongName);
#if (GC_ZR & GC_ZR_PLAY)
				player_Stop();
#endif
			}else
			{
        Player.byCurDev= MMC;   // 2014-10-21 添加，有可能如果MMC没有则可能出现问题!
				sCurSongName[0]= '\0';
			}
			pSet->byInited= 1;
		}
		DspSongName();
	}
	dspMemoryStatus();
}

u32 keydo_PlayMem(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM pNewMenu=(PMENUITEM)&Menu_Tab[MENUID_PLAYMEM];
	// PLAYMEMPARAM *pSet= (PLAYMEMPARAM *)pNewMenu->pParam;
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4: 
		return FALSE;
	case VK_NUM5:
		if(player_getState()<ERR_PLAY_PAUSEED)
			player_OpenFile(NULL, sCurSongName);
		player_Play();
		break;
	case VK_NUM6:
		player_Pause();
		break;
	case VK_NUM7:
		player_Stop();
		break;
	case VK_NUM8:
		player_Prev(sCurSongName);
		// DspSongName();
		break;
	case VK_NUM9:
		player_Next(sCurSongName);
		// DspSongName();
		break;
	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
		return FALSE;
	case VK_ENTER:	
		pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_PORTCTRL];	// 取得新的菜单
		if(pNewMenu->InitFunc)
			pNewMenu->InitFunc(g_pCurMenuItem);
		if(pNewMenu->InitFunc)
			pNewMenu->InitFunc(g_pCurMenuItem);
		break;
	case VK_ESC:	
		audio_SetAllPortState(0x0);
		audio_setSource(srcInvaild);
		menu_StopPlay();
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}

// --------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------
typedef struct {
	u8 byInited;
}PLAYAUXPARAM;

void InitPlayAUX(void *pItem)
{
	PLAYAUXPARAM *pSet= (PLAYAUXPARAM *)Menu_Tab[MENUID_PLAYAUX].pParam;
	pSet->byInited= 0;
}

void dspPlayAUX(struct _TAGMENUITEM *pItem)
{
	PLAYAUXPARAM *pSet= (PLAYAUXPARAM *)Menu_Tab[MENUID_PLAYAUX].pParam;
	if((!g_pCurMenuItem)||(MENUID_PLAYAUX!= g_pCurMenuItem->byMenuID) )
	{
		lcm_Clr();
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_PLAYAUX];
		if(pSet->byInited==0)
		{
			audio_setSource(srcAux);
			pSet->byInited= 1;
		}
		dspManualFrame();
	}
}

u32 keydo_PlayAUX(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM pNewMenu=(PMENUITEM)&Menu_Tab[MENUID_PLAYAUX];
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4: 
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
		return FALSE;
	case VK_ENTER:	
		pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_PORTCTRL];	// 取得新的菜单
		if(pNewMenu->InitFunc)
			pNewMenu->InitFunc(g_pCurMenuItem);
		break;
	case VK_ESC:	
		audio_setSource(srcInvaild);
		audio_SetAllPortState(0x0);
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	
	return TRUE;
}
// --------------------------------------------------------------------------------------------------------------
void dspUSBDiskStatus()
{
	menu_ClrClient();
	if(usb_disk_Isready)
	{
		DspSongName();
	}else
	{
		sCurSongName[0]= '\0';
		DspSongName();
		lcm_Putstr(42, 40, "无Ｕ盘！",  8, 0);
	}
}
// --------------------------------------------------------------------------------------------------------------
typedef struct {
	u8 byInited;
}PLAYUSBPARAM;

void InitPlayUSB(void *pItem)
{
	PLAYUSBPARAM *pSet= (PLAYUSBPARAM *)Menu_Tab[MENUID_PLAYUSB].pParam;
	pSet->byInited= 0;
}

void dspPlayUSB(struct _TAGMENUITEM *pItem)
{
	PLAYUSBPARAM *pSet= (PLAYUSBPARAM *)Menu_Tab[MENUID_PLAYUSB].pParam;
	if((!g_pCurMenuItem)||(MENUID_PLAYUSB!= g_pCurMenuItem->byMenuID) )
	{
		lcm_Clr();
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_PLAYUSB];
		dspManualFrame();
		if(pSet->byInited==0)
		{
			audio_setSource(srcMP3);
			if(usb_disk_Isready)
			{
				player_PlayUSB(LoadUSBPlayPosition(), sCurSongName);
				
#if (GC_ZR & GC_ZR_PLAY)
				player_Stop();
#endif
			}else
			{
				Player.byCurDev= USB;   // 2014-10-02 添加，未插入U盘， 在播放U盘中直接按播放，此时播放的是内存的曲目
				sCurSongName[0]= '\0';
			}
			pSet->byInited= 1;
		}
		dspUSBDiskStatus();
	}
	if(usb_disk_Isready)
	{
		dspPlayTime();
		dspPlayerState();
	}
}

u32 keydo_PlayUSB(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_PLAYUSB];
	// PLAYUSBPARAM *pSet= (PLAYUSBPARAM *)pNewMenu->pParam;
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4: 
		return FALSE;
	case VK_NUM5:
		if(player_getState()<ERR_PLAY_PAUSEED)
    {
			if (player_OpenFile(NULL, sCurSongName))  // 2014-10-02 避免无歌曲刷屏
  			player_Play();
    }else
 			player_Play();
		break;
    
	case VK_NUM6:
		player_Pause();
		break;
	case VK_NUM7:
		player_Stop();
		break;
	case VK_NUM8:
    if (Player.SongDir[USB].nCount) // 2014-10-02 避免无歌曲刷屏
    {
  		player_Prev(sCurSongName);
	  	DspSongName();
    }
		break;
	case VK_NUM9:
    if (Player.SongDir[USB].nCount) // 2014-10-02 避免无歌曲刷屏
    {
  		player_Next(sCurSongName);
	  	DspSongName();
    }
		break;
	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
		return FALSE;
	case VK_ENTER:	
		pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_PORTCTRL];	// 取得新的菜单
		if(pNewMenu->InitFunc)
			pNewMenu->InitFunc(g_pCurMenuItem);
		break;
	case VK_ESC:	
		audio_SetAllPortState(0x0);
		audio_setSource(srcInvaild);
		menu_StopPlay();
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	
	return TRUE;
}

// --------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------
void dspPlayPhone(struct _TAGMENUITEM *pItem)
{
	if((!g_pCurMenuItem)||(MENUID_PLAYPHONE!= g_pCurMenuItem->byMenuID) )
	{
		lcm_Clr();
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_PLAYPHONE];
		dspManualFrame();
		// 只开功放， 不开任务音源
	}
}

u32 keydo_PlayPhone(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM pNewMenu=(PMENUITEM)&Menu_Tab[MENUID_PLAYPHONE];
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4: 
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
		return FALSE;
	case VK_ENTER:	
		pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_PORTCTRL];	// 取得新的菜单
		if(pNewMenu->InitFunc)
			pNewMenu->InitFunc(g_pCurMenuItem);
		break;
	case VK_ESC:	
		// audio_setSource(srcInvaild);
		audio_SetAllPortState(0x0);
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);

	return TRUE;
}
// --------------------------------------------------------------------------------------------------------------
#if (GC_ZR & GC_ZR_PLAY)//XXX
// --------------------------------------------------------------------------------------------------------------

void dspPlayYj(struct _TAGMENUITEM *pItem)
{
	if ((!g_pCurMenuItem) || (MENUID_PLAYYJ != g_pCurMenuItem->byMenuID))
	{
		lcm_Clr();
		g_pCurMenuItem = (PMENUITEM)&Menu_Tab[MENUID_PLAYYJ];
		dspManualFrame();
		lcm_Putstr(30, 40, "正在紧急播音....", 0, 0);
		// 只开功放， 不开任务音源
	}
}

u32 keydo_PlayYj(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_PLAYYJ];
	switch (KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
	case VK_UP:
	case VK_DOWN:
		return TRUE;
	case VK_LEFT:
	case VK_RIGHT:
		return TRUE;
	case VK_ENTER:
		pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_PORTCTRL];	// 取得新的菜单
		if (pNewMenu->InitFunc)
			pNewMenu->InitFunc(g_pCurMenuItem);
		break;
	case VK_ESC:
		return TRUE;
	default:
		return TRUE;
	}
	pNewMenu->DisplayUIFunc(pItem);

	return TRUE;
}

void dspPlayBj(struct _TAGMENUITEM *pItem)
{
	if ((!g_pCurMenuItem) || (MENUID_PLAYBJ != g_pCurMenuItem->byMenuID))
	{
		lcm_Clr();
		g_pCurMenuItem = (PMENUITEM)&Menu_Tab[MENUID_PLAYBJ];
		dspManualFrame();
		lcm_Putstr(30, 40, "正在报警播音....", 0, 0);
		DspSongName();
	}
}

u32 keydo_PlayBj(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_PLAYBJ];
	switch (KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
		return TRUE;
	case VK_NUM5:
		if (player_getState()<ERR_PLAY_PAUSEED)
			player_OpenFile(NULL, sCurSongName);
		player_Play();
		break;
	case VK_NUM6:
		player_Pause();
		break;
	case VK_NUM7:
		player_Stop();
		break;
	case VK_NUM8:
	case VK_NUM9:
	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
		return TRUE;
		//return FALSE;
	case VK_ENTER:
		pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_PORTCTRL];	// 取得新的菜单
		if (pNewMenu->InitFunc)
			pNewMenu->InitFunc(g_pCurMenuItem);
		break;
	case VK_ESC:
			menu_StopPlay();
			audio_setSource(srcInvaild);
			audio_SetAllPortState(0x0);
			audio_AmpPowerOpen(FALSE);
			pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_DESKTOP];	// 取得新的菜单
		break;
	default:
		return TRUE;
	}
	pNewMenu->DisplayUIFunc(pItem);

	return TRUE;
}
// --------------------------------------------------------------------------------------------------------------
#endif
// --------------------------------------------------------------------------------------------------------------
typedef struct {
	char sPwd[5];
	u16	wParam;
}PASSWORDINPUTPARAM;

void dspPasswordInput(struct _TAGMENUITEM *pItem)
{
	u8 i;
	PASSWORDINPUTPARAM *pSet=(PASSWORDINPUTPARAM *) ((PMENUITEM)&Menu_Tab[MENUID_PASSWORDINPUT])->pParam;
	if(g_pCurMenuItem->byMenuID!= MENUID_PASSWORDINPUT)
	{
		lcm_Clr();
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_PASSWORDINPUT];
		pSet->sPwd[0]= 0;
		pSet->wParam= 0;
		dspDrawTitle();
		dspDrawRect();
		lcm_Putstr(3, 24, "请输入系统密码:", 0, 0);
	}
	for(i=0;i<pSet->wParam; i++)
		str_buf[i]= '*';
	for(; i<4; i++)
		str_buf[i]= '_';
	str_buf[i]= 0;
	lcm_Putstr(83, 40, str_buf, 4, 0);
}

u32 keydo_PasswordInput(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM  pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_PASSWORDINPUT];
	PASSWORDINPUTPARAM *pSet= (PASSWORDINPUTPARAM *)pNewMenu->pParam;
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
		pSet->sPwd[pSet->wParam++]= 0x2F+KeyCode;
	case VK_ENTER:	
 		if(pSet->wParam==4)
		{
			if((strncmp(pSet->sPwd, "4989", 4)==0)||(strncmp(pSet->sPwd, sysParam.sPwd, 4)==0))
			{
				pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_MAINMENU];	// 取得新的菜单
				*((u16 *)pNewMenu->pParam)= 0;

        // 停止运行任务
        player_Stop();
				audio_SetAllPortState(0);	// 关闭所有分区
        if(g_wJobRunCounter)
        {
  				CloseAmpPower();
        }
				g_wJobRunCounter= 0;
        
			}
			pSet->wParam=0;
		}
		break;
	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
		return TRUE;
	case VK_ESC:	// 按下菜单键，进入菜单
		// 放弃数据保存，返回上一级
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}
// --------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------
typedef struct {
	u8 byInited;
	u8 byIndex;
}MAINMENUPARAM;

void InitMainMenu(void *pItem)
{
	MAINMENUPARAM *pSet= (MAINMENUPARAM *)Menu_Tab[MENUID_MAINMENU].pParam;
	pSet->byInited= 0;
}

static const u8 menu_item4_pos_tab[4][2]={{21, 24},{113, 24},{21, 40}, {113, 40}};
void dspMainMenu(struct _TAGMENUITEM *pItem)
{
	unsigned int i;
	MAINMENUPARAM *pSet= (MAINMENUPARAM *)Menu_Tab[MENUID_MAINMENU].pParam;
	if((!g_pCurMenuItem)||(MENUID_MAINMENU!= g_pCurMenuItem->byMenuID) )
	{
		lcm_Clr();
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_MAINMENU];
		if(pSet->byInited==0)
		{
      audio_setSource(srcInvaild);
      menu_StopPlay();
			pSet->byIndex= 0;
			pSet->byInited= 1;
		}
		dspDrawTitle();
		dspDrawRect();
	}
	for(i=0;i<4;i++)
	{
		lcm_Putstr(menu_item4_pos_tab[i][0], menu_item4_pos_tab[i][1], sMenuTitles[i], 8, pSet->byIndex==i);
	}
	//if(m)
	//	lcm_Putstr(179, 24, sPrevPage, 2, 0);
	//else
	//	lcm_Putstr(179, 24, sNextPage, 2, 0);
}

u32 keydo_MainMenu(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM  pNewMenu=(PMENUITEM)&Menu_Tab[MENUID_MAINMENU];
	MAINMENUPARAM *pSet= (MAINMENUPARAM *)pNewMenu->pParam;
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
	case VK_UP:
	case VK_DOWN:
		return FALSE;
	case VK_LEFT:
		if(pSet->byIndex)
			pSet->byIndex--;
		else
			pSet->byIndex= 3;
		pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_MAINMENU];
		break;
	case VK_RIGHT:
		if(pSet->byIndex>=3)
			pSet->byIndex=0;
		else
			pSet->byIndex++;
		pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_MAINMENU];
		break;
	case VK_ENTER:	
		if((sd_valid_flag==FALSE)&&(pSet->byIndex<2))
			return TRUE;
		pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_TASKMANAGER]+pSet->byIndex;	// 取得新的菜单
		if(pNewMenu->InitFunc)
			pNewMenu->InitFunc(g_pCurMenuItem);
		break;
	case VK_ESC:	// 按下菜单键，进入菜单
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		if(g_bTaskChange)
		{
			tmSaveToFile(&g_taskManager, sTaskFile);
			// menu_UpdateTodayTask();
			g_bTaskChange= FALSE;
		}
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}

// --------------------------------------------------------------------------------------------------------------
typedef struct {
	u8 byInited;
	u8 byIndex;
}TASKMANAGERMENUPARAM;

void InitTaskMannagerMenu(void *pItem)
{
	TASKMANAGERMENUPARAM *pSet= (TASKMANAGERMENUPARAM *)Menu_Tab[MENUID_TASKMANAGER].pParam;
	pSet->byInited = 0;
	pSet->byIndex = 0;
}
#if (GC_ZR & GC_ZR_MANAGE)
void dspTaskManagerMenu(struct _TAGMENUITEM *pItem)
{
	unsigned int i,j;
	TASKMANAGERMENUPARAM *pSet = (TASKMANAGERMENUPARAM *)Menu_Tab[MENUID_TASKMANAGER].pParam;
	if ((!g_pCurMenuItem) || (MENUID_TASKMANAGER != g_pCurMenuItem->byMenuID))
	{
		pSet->byInited = 0;
		g_pCurMenuItem = (PMENUITEM)&Menu_Tab[MENUID_TASKMANAGER];
	}
	if (pSet->byInited == 0)
	{
		pSet->byInited = 1;
		lcm_Clr();
		dspDrawTitle();
		dspDrawRect();
	}
	if (pSet->byIndex < 4) {		
		for (i = 0; i<4; i++)
		{
			j = i + 0;
			lcm_Putstr(menu_item4_pos_tab[i][0], menu_item4_pos_tab[i][1], sTaskMenuTitles[j], 8, pSet->byIndex == j);
		}
		lcm_Putstr(172,52,nExtPage,2,0);
	}
	else if ((pSet->byIndex >= 4) && ( pSet->byIndex< 6)) {
		for (i = 0; i<2; i++)
		{
			j = i + 4;
			lcm_Putstr(menu_item4_pos_tab[i][0], menu_item4_pos_tab[i][1], sTaskMenuTitles[j], 8, pSet->byIndex == j);
		}
		lcm_Putstr(172,52,pRePage,2,0);
	}
}
#else
void dspTaskManagerMenu(struct _TAGMENUITEM *pItem)
{
	unsigned int i, m;
	TASKMANAGERMENUPARAM *pSet = (TASKMANAGERMENUPARAM *)Menu_Tab[MENUID_TASKMANAGER].pParam;
	if ((!g_pCurMenuItem) || (MENUID_TASKMANAGER != g_pCurMenuItem->byMenuID))
	{
		lcm_Clr();
		if (pSet->byInited == 0)
		{
			pSet->byIndex = 0;
			pSet->byInited = 1;
		}
		g_pCurMenuItem = (PMENUITEM)&Menu_Tab[MENUID_TASKMANAGER];
		dspDrawTitle();
		dspDrawRect();
	}
	m = (pSet->byIndex) & 0x4;
	for (i = 0; i<4; i++)
	{
		lcm_Putstr(menu_item4_pos_tab[i][0], menu_item4_pos_tab[i][1], sTaskMenuTitles[m + i], 8, pSet->byIndex == i);
	}
}
#endif
u32 keydo_TaskManagerMenu(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM  pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_TASKMANAGER];
	TASKMANAGERMENUPARAM *pSet= (TASKMANAGERMENUPARAM *)pNewMenu->pParam;
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
	case VK_UP:
	case VK_DOWN:
		return FALSE;
#if (GC_ZR & GC_ZR_MANAGE)
	case VK_LEFT:
		if(pSet->byIndex)
			pSet->byIndex--;
		else
			pSet->byIndex= 5;
		
		switch(pSet->byIndex) {
			case 3:
			case 5:
				pSet->byInited = 0;
			default:
				break;
		}
		
		break;
	case VK_RIGHT:
		if(pSet->byIndex>=5)
			pSet->byIndex=0;
		else
			pSet->byIndex++;
		
		switch(pSet->byIndex) {
			case 0:
			case 4:
				pSet->byInited = 0;
			default:
				break;
		}
		
		break;
#else
	case VK_LEFT:
		if (pSet->byIndex)
			pSet->byIndex--;
		else
			pSet->byIndex = 3;
		break;
	case VK_RIGHT:
		if (pSet->byIndex >= 3)
			pSet->byIndex = 0;
		else
			pSet->byIndex++;
		break;
#endif
	case VK_ENTER:	
		pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_TASKVIEW]+pSet->byIndex;	// 取得新的菜单
		if(pNewMenu->InitFunc)
			pNewMenu->InitFunc(g_pCurMenuItem);
		break;
	case VK_ESC:	
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}
// --------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------
typedef struct {
	u8 byInited;
	u8 byIndex;
}SONGVIEWMENUPARAM;

void InitSongViewMenu(void *pItem)
{
	SONGVIEWMENUPARAM *pSet= (SONGVIEWMENUPARAM *)Menu_Tab[MENUID_SONGVIEWMENU].pParam;
	pSet->byInited= 0;
}

void dspSongViewMenu(struct _TAGMENUITEM *pItem)
{
	unsigned int i, m;
	SONGVIEWMENUPARAM *pSet= (SONGVIEWMENUPARAM *)Menu_Tab[MENUID_SONGVIEWMENU].pParam;
	if((!g_pCurMenuItem)||(MENUID_SONGVIEWMENU!= g_pCurMenuItem->byMenuID) )
	{
		lcm_Clr();
		if(pSet->byInited==0)
		{
			pSet->byIndex= 0;
			pSet->byInited= 1;
		}
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_SONGVIEWMENU];
		dspDrawTitle();
		dspDrawRect();
	}
	m= pSet->byIndex &0x2;
	for(i=0;i<2;i++)
	{
		lcm_Putstr(menu_item4_pos_tab[i][0], 32, sSongViewMenus[m+i], 8, pSet->byIndex==i);
	}
	
}

u32 keydo_SongViewMenu(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM  pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_SONGVIEWMENU];
	SONGVIEWMENUPARAM *pSet= (SONGVIEWMENUPARAM *)pNewMenu->pParam;
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
	case VK_UP:
	case VK_DOWN:
		return FALSE;
	case VK_LEFT:
		if(pSet->byIndex)
			pSet->byIndex--;
		else
			pSet->byIndex= 1;
		pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_SONGVIEWMENU];
		break;
	case VK_RIGHT:
		if(pSet->byIndex>=1)
			pSet->byIndex=0;
		else
			pSet->byIndex++;
		pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_SONGVIEWMENU];
		break;
	case VK_ENTER:	
		pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_SONGVIEWMEM]+pSet->byIndex;	// 取得新的菜单
		break;
	case VK_ESC:	
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}

// --------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------
typedef struct {
	u8 byInited;
	u8 byIndex;
}SYSTEMSETPARAM;

void InitSystemSet(void *pItem)
{
	SYSTEMSETPARAM *pSet= (SYSTEMSETPARAM *)Menu_Tab[MENUID_SYSTEMSET].pParam;
	pSet->byInited= 0;
}

void dspSystemSet(struct _TAGMENUITEM *pItem)
{
	unsigned int i;
	SYSTEMSETPARAM *pSet= (SYSTEMSETPARAM *)Menu_Tab[MENUID_SYSTEMSET].pParam;
	if((!g_pCurMenuItem)||(MENUID_SYSTEMSET!= g_pCurMenuItem->byMenuID) )
	{
		lcm_Clr();
		if(pSet->byInited==0)
		{
			pSet->byIndex= 0;
			pSet->byInited= 1;
		}

		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_SYSTEMSET];
		dspDrawTitle();
		dspDrawRect();
	}
#if (GC_ZR & GC_ZR_MANAGE)
	for(i=0;i<3;i++)
	{
		lcm_Putstr(menu_item4_pos_tab[i][0], menu_item4_pos_tab[i][1], sSystemSets[i], 8, pSet->byIndex==i);
	}
#else
	m = pSet->byIndex & 0x2;
	for (i = 0; i<2; i++)
	{
		lcm_Putstr(menu_item4_pos_tab[i][0], 32, sSystemSets[m + i], 8, pSet->byIndex == i);
	}
#endif
}

u32 keydo_SystemSet(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM  pNewMenu=(PMENUITEM)&Menu_Tab[MENUID_SYSTEMSET];
	SYSTEMSETPARAM *pSet= (SYSTEMSETPARAM *)pNewMenu->pParam;
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
	case VK_UP:
	case VK_DOWN:
		return FALSE;
#if (GC_ZR & GC_ZR_MANAGE)
	case VK_LEFT:
		if(pSet->byIndex)
			pSet->byIndex--;
		else
			pSet->byIndex= 2;
		break;
	case VK_RIGHT:
		if(pSet->byIndex > 2-1)
			pSet->byIndex=0;
		else
			pSet->byIndex++;
		break;
#else
	case VK_LEFT:
		if (pSet->byIndex)
			pSet->byIndex--;
		else
			pSet->byIndex = 1;
		break;
	case VK_RIGHT:
		if (pSet->byIndex >= 1)
			pSet->byIndex = 0;
		else
			pSet->byIndex++;
		break;
#endif
	case VK_ENTER:	
		pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_TIMESET]+pSet->byIndex;	// 取得新的菜单
		if(pNewMenu->InitFunc)
			pNewMenu->InitFunc(g_pCurMenuItem);
		break;
	case VK_ESC:	
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}

// --------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------
typedef struct {
	u8 byMode;
	u8 byDayCount;
	u16 wParam;
}SPECIALMODESETPARAM;

static void GetSpecialMode(SPECIALMODESETPARAM *pSpec)
{
	int nDays, nStartDays;
	CSpecialMode *pMode= &(g_taskManager.m_lpSpecMode);
	pSpec->byMode= pMode->m_bySpecMode;
	if(pSpec->byMode)
	{
		nDays= MyDateOfDays(&g_SysDate)-MyDateOfDays(&pMode->m_SpecialDate);
		if(nDays>0)		// 需要调整
		{
			nStartDays= pMode->m_bySpecMode-1;
			if(nDays>=nStartDays+pMode->m_bySpecDayCount)
			{
				pSpec->byMode= SPECIALMODE_INVALID;
			} else
			{
				pSpec->byMode= SPECIALMODE_CURRENT;
				pSpec->byDayCount= pMode->m_bySpecDayCount-(nDays-nStartDays);
			}
		} else
			if(nDays==0)
				pSpec->byDayCount= pMode->m_bySpecDayCount;
			else
			{
				pSpec->byMode= SPECIALMODE_INVALID;
			}
	}
}

static SetSpecialMode(SPECIALMODESETPARAM *pSpec)
{
	CSpecialMode *pMode= &g_taskManager.m_lpSpecMode;
	pMode->m_bySpecMode= pSpec->byMode;
	pMode->m_bySpecDayCount= pSpec->byDayCount;
	memcpy(&pMode->m_SpecialDate, &g_SysDate, sizeof(TMyDate));
}

void dspSpecialMode(struct _TAGMENUITEM *pItem)
{
	SPECIALMODESETPARAM *pSpec= (SPECIALMODESETPARAM *)((PMENUITEM)&Menu_Tab[MENUID_SPECIALMODE])->pParam;
	if((!g_pCurMenuItem)||(MENUID_SPECIALMODE!= g_pCurMenuItem->byMenuID) )
	{
		lcm_Clr();
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_SPECIALMODE];
		dspDrawTitle();
		dspDrawRect();
		lcm_Putstr(3, 24, &sMenuTitles[3][4], 4, 0);
		lcm_Putstr(3, 40, "执行天数", 8, 0);
		// 需要从任务数据中获取状态
		GetSpecialMode(pSpec);
		pSpec->wParam= 0;
	}
	
	lcm_Putstr(130, 24, sSpecialModes[pSpec->byMode], 10, pSpec->wParam==0);
	strfmt(str_buf, "%d", pSpec->byDayCount);
	lcm_Putstr(184, 40, str_buf, 1, pSpec->wParam==1);
}

u32 keydo_SpecialMode(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM  pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_SPECIALMODE];
	SPECIALMODESETPARAM *pSpec= (SPECIALMODESETPARAM *)((PMENUITEM)&Menu_Tab[MENUID_SPECIALMODE])->pParam;
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
		return FALSE;
	case VK_UP:
		if(pSpec->wParam==0)
		{
			if(pSpec->byMode<3)
				pSpec->byMode ++;
			else
				pSpec->byMode= 0;
		}else
			if(pSpec->byDayCount<7)
				pSpec->byDayCount++;
			else
				pSpec->byDayCount= 1;
		break;
	case VK_DOWN:
		if(pSpec->wParam==0)
		{
			if(pSpec->byMode)
				pSpec->byMode --;
			else
				pSpec->byMode= 3;
		}else
			if(pSpec->byDayCount>1)
				pSpec->byDayCount--;
			else
				pSpec->byDayCount= 7;
		break;
	case VK_LEFT:
		if(pSpec->byMode)
			if(pSpec->wParam)
				(pSpec->wParam)--;
			else
				pSpec->wParam= 1;
		else
			pSpec->wParam= 0;
		break;
	case VK_RIGHT:
		if(pSpec->byMode)
			if(pSpec->wParam>=1)
				pSpec->wParam= 0;
			else
				pSpec->wParam++;
		else
			pSpec->wParam= 0;
		break;
	case VK_ENTER:	
		// 保存相关数据，返回上一级
		SetSpecialMode(pSpec);
		g_bTaskChange= TRUE;
	case VK_ESC:	
		// 放弃数据保存，返回上一级
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}
// --------------------------------------------------------------------------------------------------------------

// 播音事件编辑中时间各项显示位置
static u8 jobtimecolpos_tab[]={30, 48, 66, 142, 160, 178};
static char const sDig2Fmt[] = "%02d";
static const char sDig4Fmt[] = "%04d";
#if (GC_ZR)
static const char sDig6Fmt[] = "%06d";
#endif
static const u8 intercutdateshowpos_tab[3]={33, 63, 81};

// --------------------------------------------------------------------------------------------------------------
static const u8 taskviewdateshowpos_tab[]={66, 96, 114};

typedef struct	{
	TMyDate	dt;
	u8 byIndex;
	u8 byDig;
}TASKVIEWPARAM;

void InitTaskView(void *pItem)
{
	TASKVIEWPARAM *pSet= (TASKVIEWPARAM *)((PMENUITEM)&Menu_Tab[MENUID_TASKVIEW])->pParam;
	pSet->byIndex= 0;
	pSet->byDig= 0;
	// 取得当前日期
	memcpy(&pSet->dt, &g_SysDate, sizeof(TMyDate));
}

void dspTaskView(struct _TAGMENUITEM *pItem)
{
	TASKVIEWPARAM *pSet= (TASKVIEWPARAM *)((PMENUITEM)&Menu_Tab[MENUID_TASKVIEW])->pParam;
	if((!g_pCurMenuItem)||(MENUID_TASKVIEW!= g_pCurMenuItem->byMenuID) )
	{
		lcm_Clr();
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_TASKVIEW];
		dspDrawTitle();
		dspDrawRect();
		lcm_Putstr(3, 24, sDate, 4, 0);
		lcm_Putstr(taskviewdateshowpos_tab[0], 40, "    -  -  ", 10, 0);
	}
	
	// 日期
	strfmt(str_buf, sDig4Fmt, pSet->dt.wYear);
	lcm_Putstr(taskviewdateshowpos_tab[0], 40, str_buf, 4, pSet->byIndex==0);
	strfmt(str_buf, sDig2Fmt, pSet->dt.byMonth);
	lcm_Putstr(taskviewdateshowpos_tab[1], 40, str_buf, 2, pSet->byIndex==1);
	strfmt(str_buf, sDig2Fmt, pSet->dt.byDay);
	lcm_Putstr(taskviewdateshowpos_tab[2], 40, str_buf, 2, pSet->byIndex==2);
}

u32 keydo_TaskView(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	NUMINOUTPARAM Num;
	u8	byDig;
	PMENUITEM  pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_TASKVIEW];
	TASKVIEWPARAM *pSet= (TASKVIEWPARAM *)((PMENUITEM)&Menu_Tab[MENUID_TASKVIEW])->pParam;
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
		switch(pSet->byIndex)
		{
		case 0:	// year
			Num.sub.wValue= pSet->dt.wYear;
			byDig= 4;
			break;
		case 1:	// month
			Num.sub.wValue= pSet->dt.byMonth;
			byDig=2;
			break;
		case 2:	// day
			Num.sub.wValue= pSet->dt.byDay;
			byDig=2;
			break;
		default:
			return TRUE;
		}
		Num.sub.byInputDig = pSet->byDig;
		if(Num.sub.byInputDig==0)
			Num.sub.wValue= 0;
		NumInputProc(&Num, KeyCode-1,  byDig);
		dspInputNum(taskviewdateshowpos_tab[pSet->byIndex], 40, &Num, byDig, 1);
		pSet->byDig= Num.sub.byInputDig;
		switch(pSet->byIndex)
		{
		case 0:	// year
			pSet->dt.wYear= Num.sub.wValue;
			break;
		case 1:	// month
			pSet->dt.byMonth= Num.sub.wValue;
			break;
		case 2:	// day
			pSet->dt.byDay= Num.sub.wValue;
			break;
		}
		
		return TRUE;
	case VK_UP:
		switch(pSet->byIndex)
		{
		case 0:	// date:year
			if(pSet->dt.wYear<2112)
				pSet->dt.wYear++;
			else
				pSet->dt.wYear= 2013;
			break;
		case 1:	// date:month
			if(pSet->dt.byMonth<12)
				pSet->dt.byMonth++;
			else
				pSet->dt.byMonth= 1;
			break;
		case 2:	// date:day
			if(pSet->dt.byDay<GetDayCount(pSet->dt.wYear, pSet->dt.byMonth))
				pSet->dt.byDay++;
			else
				pSet->dt.byDay= 1;
			break;
 		default:
			return TRUE;
		}
		break;
	case VK_DOWN:
		switch(pSet->byIndex)
		{
		case 0:	// date:year
			if(pSet->dt.wYear>2013)
				pSet->dt.wYear--;
			else
				pSet->dt.wYear= 2112;
			break;
		case 1:	// date:month
			if(pSet->dt.byMonth>1)
				pSet->dt.byMonth--;
			else
				pSet->dt.byMonth= 12;
			break;
		case 2:	// date:day
			if(pSet->dt.byDay>1)
				pSet->dt.byDay--;
			else
				pSet->dt.byDay= GetDayCount(pSet->dt.wYear, pSet->dt.byMonth);
			break;
		default:
			return TRUE;
		}
		break;
	case VK_LEFT:
		if(pSet->byIndex)
			pSet->byIndex--;
		else
			pSet->byIndex= 2;
		break;
	case VK_RIGHT:
		if(pSet->byIndex<2)
			pSet->byIndex++;
		else
			pSet->byIndex= 0;
		break;
	case VK_ENTER:	
		// 保存相关数据，返回上一级
		pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_TASKVIEWLIST];
		if(pNewMenu->InitFunc)
			pNewMenu->InitFunc(g_pCurMenuItem);
		break;
	case VK_ESC:	
		// 放弃数据保存，返回上一级
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}
// --------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------
typedef struct	{
	CTaskJob	job;
	TMyTime	endTime;
}EDITJOB;

typedef struct	{
	u8 byInited;
	u8 byIndex;		// 位置索引
	u8 byGroup;		// 事件组编号
	u8 byJobID;		// 事件编号
	u8 byMode:6;			// 编辑模式, 0--查看，1--添加，2--修改
	u8 byDig:2;			// 输入位数
	u8 byParentMenuID;	// 进入事件编辑前的菜单编号
	EDITJOB Edit;		// 事件数据
}JOBEDITPARAM;

static void  _InitJobParam(JOBEDITPARAM *pSet)
{
	tjInit(&pSet->Edit.job);
	memset(&pSet->Edit.endTime, 0, sizeof(TMyTime));
}

static void _NewJobParam(JOBEDITPARAM *pSet)
{
	tjSetStartMyTime(&pSet->Edit.job, tjgGetNewTaskStartTime(tjglGetAt(&g_taskManager.m_lpJobGroups, pSet->byGroup), &pSet->Edit.endTime));
	tjGetEndTime(&pSet->Edit.job, &pSet->Edit.endTime);
}

static void _loadJobParam(JOBEDITPARAM *pSet, CTaskJob *tj)
{
	tjAssign(&pSet->Edit.job, tj);
	MyTimeAdd(&pSet->Edit.endTime, tjGetStartTime(tj), tjGetPlayLength(tj));
}

void InitJobEdit(void *pItem)
{
	JOBEDITPARAM *pSet= (JOBEDITPARAM *)((PMENUITEM)&Menu_Tab[MENUID_JOBEDIT])->pParam;
	pSet->byInited= 0;
	pSet->byParentMenuID= ((PMENUITEM)pItem)->byMenuID;
}

char * GetJobSourceString(CTaskJob *tj, char *szRes, u8 len)
{
	char lfn[51];
	u8 nSrc= tjGetSourceType(tj), byBand;
	if((nSrc==stInvalid)||(nSrc> stMicphone))
	{
		szRes[0]= 0;
		return szRes;
	}
#if (GC_ZR & GC_ZR_FC)
	strfmt(szRes, "%s", sSourceName[nSrc - 1]);
#else
	strfmt(szRes, "%s ", sSourceName[nSrc - 1]);
#endif
	switch(nSrc)
	{
	case stSongFile:
		if (FindSongno(sSongPaths[0], tjGetSongID(tj), TRUE, NULL, lfn, NULL))
#if (GC_ZR & GC_ZR_FC)
			strncpy(szRes, lfn, len);
#else
			strncpy(szRes, lfn, 17);
#endif
		// strncat(szRes, lfn, 17);
		break;
	case stTuner:
		byBand= tjGetTunerBand(tj);
		strfmt(lfn, "%s", tuner_getBandName(byBand));
		strcat(szRes, lfn);
		strcat(szRes, tuner_getFreqString(byBand, tjGetTunerFreq(tj), lfn));
		break;
	}
	for(nSrc=strlen(szRes); nSrc<len; nSrc++)
		szRes[nSrc]= ' ';
	szRes[len]= 0; 
	return szRes;
}
#if (GC_ZR & GC_ZR_FC)//XXX
u8 GetJobPortStateString(CTaskJob *pJob, char *str)
{
	u8 i;
	for(i=0;i<6;i++)
	{
		if(pJob->mPortState&(1<<i))
			str[i]= i+0x31;
		else
			str[i]= ' ';
	}
	return 6;
}

u8 GetPortStateString(char *str)
{
	u8 i;
	for(i=0;i<6;i++)
	{
		if(audio_GetPortState(i))
			str[i]= i+0x31;
		else
			str[i]= ' ';
	}
	return 6;
}
#else
u8 GetJobPortStateString(CTaskJob *pJob, char *str)
{
	u8 i;
	for (i = 0; i<4; i++)
	{
		if (pJob->mPortState&(1 << i))
			str[i] = i + 0x31;
		else
			str[i] = ' ';
	}
	return 4;
}

u8 GetPortStateString(char *str)
{
	u8 i;
	for (i = 0; i<4; i++)
	{
		if (audio_GetPortState(i))
			str[i] = i + 0x31;
		else
			str[i] = ' ';
	}
	return 4;
}
#endif
void dspJobEndTime(EDITJOB *pSet, u8 startIndex, u8 curIndex)
{
	//  结束时间
	strfmt(str_buf, sDig2Fmt, pSet->endTime.byHour);
	lcm_Putstr(jobtimecolpos_tab[3], 32, str_buf, 2, curIndex==startIndex);
	
	strfmt(str_buf, sDig2Fmt, pSet->endTime.byMinute);
	lcm_Putstr(jobtimecolpos_tab[4], 32, str_buf, 2, curIndex==startIndex+1);
	
	strfmt(str_buf, sDig2Fmt, pSet->endTime.bySecond);
	lcm_Putstr(jobtimecolpos_tab[5], 32, str_buf, 2, curIndex==startIndex+2);
}

void dspJobEdit(struct _TAGMENUITEM *pItem)
{
	JOBEDITPARAM *pSet= (JOBEDITPARAM *)((PMENUITEM)&Menu_Tab[MENUID_JOBEDIT])->pParam;
	CTaskJobGroup *pJobGroup=NULL;
	u8 nJobCount;
	if((!g_pCurMenuItem)||(MENUID_JOBEDIT!= g_pCurMenuItem->byMenuID) )
	{
		lcm_Clr();
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_JOBEDIT];
		dspDrawRect();
		if(pSet->byInited==0)
		{
			memset(pSet, 0, sizeof(JOBEDITPARAM));
			
			pJobGroup= tjglGetAt(&g_taskManager.m_lpJobGroups, pSet->byGroup);
			// tjglGetJobGroup(&g_taskManager.m_lpJobGroups, pSet->byGroup+1);
			/* if(pJobGroup==NULL)
			{
				pJobGroup= tjglNew(&g_taskManager.m_lpJobGroups);
			}*/
			nJobCount= tjgGetCount(pJobGroup);
			if(nJobCount)
				_loadJobParam(pSet, tjgGetAt(pJobGroup, 0));
			else
			{
				tjSetSourceType(&pSet->Edit.job, stTuner);
				tjSetTunerBand(&pSet->Edit.job, tuner_getBand());
				tjSetTunerFreq(&pSet->Edit.job, tuner_getCurrentFreq(0xFF));
#if (GC_ZR & GC_ZR_FC)
				pSet->Edit.job.mPortState = 0xFFFF;
#else
				pSet->Edit.job.mPortState = 0xFF;
#endif
				_NewJobParam(pSet);
			}
			pSet->byInited= 1;
		}
		lcm_Linexy(1, 14, 190, 14);
		lcm_Linexy(1, 13, 190, 13);
		lcm_Putstr(18, 0, "事件组 /8", 9, 0);
		lcm_Putstr(2, 16, sSource, 4, 0);
#if (GC_ZR & GC_ZR_FC)
		lcm_Putstr(130, 16, sPort, 4, 0);
#else
		lcm_Putstr(136, 16, sPort, 4, 0);
#endif
		lcm_Putstr(2, 32, sStart, 4, 0);
		lcm_Putstr(118, 32, sEnd, 4, 0);
		strcpy(str_buf, sZeroTime);
		lcm_Putstr(jobtimecolpos_tab[0], 32, str_buf, 8, 0);
		lcm_Putstr(jobtimecolpos_tab[3], 32, str_buf, 8, 0);
	}
	if(pJobGroup==NULL)
	{
		pJobGroup= tjglGetAt(&g_taskManager.m_lpJobGroups, pSet->byGroup);
		// tjglGetJobGroup(&g_taskManager.m_lpJobGroups, pSet->byGroup+1);
		/* if(pJobGroup==NULL)
		{
			pJobGroup= tjglNew(&g_taskManager.m_lpJobGroups);
		}*/
		nJobCount= tjgGetCount(pJobGroup);
	}
	// 标题	
	str_buf[0]= 0x30+pSet->byGroup+1;
	lcm_Putstr(54, 0, str_buf, 1, pSet->byIndex==0);
	
	strfmt(str_buf, "%s事件%03d/%03d", sEditMode[pSet->byMode], (pSet->byMode==1)?nJobCount+1:(nJobCount?pSet->byJobID+1:0), nJobCount);
	lcm_Putstr(84, 0, str_buf, 15, 0);
	
	// 音源
#if (GC_ZR & GC_ZR_FC)
	lcm_Putstr(30, 16, GetJobSourceString(&pSet->Edit.job, str_buf, 16), 16, pSet->byIndex==1);
#else
	lcm_Putstr(30, 16, GetJobSourceString(&pSet->Edit.job, str_buf, 17), 17, pSet->byIndex==1);
#endif

	// 分区
#if (GC_ZR & GC_ZR_FC)
	lcm_Putstr(154, 16, str_buf, GetJobPortStateString(&pSet->Edit.job, str_buf), pSet->byIndex == 2);
#else
	lcm_Putstr(166, 16, str_buf, GetJobPortStateString(&pSet->Edit.job, str_buf), pSet->byIndex == 2);
#endif
	// 开始时间
	strfmt(str_buf, sDig2Fmt, pSet->Edit.job.m_Head.srtStartTime.byHour);
	lcm_Putstr(jobtimecolpos_tab[0], 32, str_buf, 2, pSet->byIndex==3);
	
	strfmt(str_buf, sDig2Fmt, pSet->Edit.job.m_Head.srtStartTime.byMinute);
	lcm_Putstr(jobtimecolpos_tab[1], 32, str_buf, 2, pSet->byIndex==4);
	
	strfmt(str_buf, sDig2Fmt, pSet->Edit.job.m_Head.srtStartTime.bySecond);
	lcm_Putstr(jobtimecolpos_tab[2], 32, str_buf, 2, pSet->byIndex==5);
	
	dspJobEndTime(&pSet->Edit, 6, pSet->byIndex);	

	// 显示功能菜单
	lcm_Putstr(2, 48, sPrev, 6, pSet->byIndex==9);
	lcm_Putstr(50, 48, sAdd, 4, pSet->byIndex==10);
	
	if(pSet->byMode)
		lcm_Putstr(84, 48, sSave, 4, pSet->byIndex==11);
	else
		lcm_Putstr(84, 48, sEdit, 4, pSet->byIndex==11);
	
	lcm_Putstr(118, 48, sDelete, 4, pSet->byIndex==12);
	lcm_Putstr(154, 48, sNext, 6, pSet->byIndex==13);
	
}

u32 keydo_JobEdit(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	static const u8 jobeditindex_normal_tab[][14]={
		{9, 0, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 0},	   // right
		{13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 10, 11, 12}		// left
	};
	static const u8 jobeditindex_edit_tab[][14]={
		{0, 2, 3, 4, 5, 6, 7, 8, 11, 0, 0, 1, 0, 0},		// right
		{0, 11, 1, 2, 3, 4, 5, 6, 7, 0, 0, 8, 0, 0}		// left
	};
	NUMINOUTPARAM Num;
	u8 *pBuf, index, nJobCount;
	PMENUITEM  pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_JOBEDIT];
	JOBEDITPARAM *pSet= (JOBEDITPARAM *)pNewMenu->pParam;
	CTaskJobGroup *pJobGroup= tjglGetAt(&g_taskManager.m_lpJobGroups, pSet->byGroup);
	CTaskJob		*pJob;
	nJobCount= tjgGetCount(pJobGroup);

	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
		switch(pSet->byIndex)
		{
		case 3:	// start:hour
		case 4:	// start:minute
		case 5:	// start:second
			pBuf= (u8 *)&pSet->Edit.job.m_Head.srtStartTime;
			index= pSet->byIndex-3;
			Num.sub.wValue= pBuf[index];
			break;
		case 6:	// end:hour
		case 7:	// end:minute
		case 8:	// end:second
			pBuf= (u8 *)&pSet->Edit.endTime;
			index= pSet->byIndex-6;
			Num.sub.wValue= pBuf[index];
			index+= 3;
			break;
		default:
			return TRUE;
		}
		Num.sub.byInputDig = pSet->byDig;
		if(Num.sub.byInputDig==0)
			Num.sub.wValue= 0;
		NumInputProc(&Num, KeyCode-1,  2);
		dspInputNum(jobtimecolpos_tab[index], 32, &Num, 2, 1);
		pSet->byDig= Num.sub.byInputDig;
		switch(pSet->byIndex)
		{
		case 3:	// start:hour
		case 4:	// start:minute
		case 5:	// start:second
			pBuf[index]= Num.sub.wValue;
			MyTimeAdd(&pSet->Edit.endTime,  tjGetStartTime(&pSet->Edit.job), tjGetPlayLength(&pSet->Edit.job));
			dspJobEndTime(&pSet->Edit, 6, pSet->byIndex);
			break;
		case 6:	// end:hour
		case 7:	// end:minute
		case 8:	// end:second
			pBuf[pSet->byIndex-6]= Num.sub.wValue;
		}
		return TRUE;
	case VK_UP:
		switch(pSet->byIndex)
		{
		case 0:
			if(pSet->byGroup<tjglGetCount(&g_taskManager.m_lpJobGroups)-1)
				pSet->byGroup++;
			else
				pSet->byGroup= 0;
			pSet->byJobID= 0;
			pJobGroup= tjglGetAt(&g_taskManager.m_lpJobGroups, pSet->byGroup);
			if(tjgGetCount(pJobGroup))
			{
				_loadJobParam(pSet, tjgGetAt(pJobGroup, pSet->byJobID));
			}else
				_InitJobParam(pSet);
			break;
		case 1:
			if(pSet->Edit.job.m_Head.bySourceType>stTuner)
				pSet->Edit.job.m_Head.bySourceType--;
			else
				pSet->Edit.job.m_Head.bySourceType= stMicphone;
			switch(pSet->Edit.job.m_Head.bySourceType)
			{
			case stSongFile:
				tjSetSongType(&pSet->Edit.job, pstSongFile);
				tjSetSongID(&pSet->Edit.job, 1);
				break;
			case stTuner:
				tjSetTunerBand(&pSet->Edit.job, tuner_getBand());
				tjSetTunerFreq(&pSet->Edit.job, tuner_getCurrentFreq(0xFF));
				break;
			}
			break;
		case 3:	// start:hour
			if(pSet->Edit.job.m_Head.srtStartTime.byHour<23)
				pSet->Edit.job.m_Head.srtStartTime.byHour++;
			else
				pSet->Edit.job.m_Head.srtStartTime.byHour= 0;
			MyTimeAdd(&pSet->Edit.endTime,  tjGetStartTime(&pSet->Edit.job), tjGetPlayLength(&pSet->Edit.job));
			dspJobEndTime(&pSet->Edit, 5, pSet->byIndex);
			break;
		case 4:	// start:minute
		case 5:	// start:second
			pBuf= (u8 *)&pSet->Edit.job.m_Head.srtStartTime;
			index= pSet->byIndex-3;
			if(pBuf[index]<59)
				pBuf[index]++;
			else
				pBuf[index]= 0;
			MyTimeAdd(&pSet->Edit.endTime,  tjGetStartTime(&pSet->Edit.job), tjGetPlayLength(&pSet->Edit.job));
			dspJobEndTime(&pSet->Edit, 5, pSet->byIndex);
			break;
		case 6:	// end:hour
			if(pSet->Edit.endTime.byHour<23)
				pSet->Edit.endTime.byHour++;
			else
				pSet->Edit.endTime.byHour= 0;
			break;
		case 7:	// end:minute
		case 8:	// end:second
			pBuf= (u8 *)&pSet->Edit.endTime;
			index= pSet->byIndex-6;
			if(pBuf[index]<59)
				pBuf[index]++;
			else
				pBuf[index]= 0;
			break;
		default:
			return TRUE;
		}
		break;
	case VK_DOWN:
		switch(pSet->byIndex)
		{
		case 0:
			if(pSet->byGroup)
				pSet->byGroup--;
			else
				pSet->byGroup= tjglGetCount(&g_taskManager.m_lpJobGroups)-1;
			pSet->byJobID= 0;
			pJobGroup= tjglGetAt(&g_taskManager.m_lpJobGroups, pSet->byGroup);
			if(tjgGetCount(pJobGroup))
			{
				_loadJobParam(pSet, tjgGetAt(pJobGroup, pSet->byJobID));
			}else
				_InitJobParam(pSet);
			break;
		case 1:
			if(pSet->Edit.job.m_Head.bySourceType<stMicphone)
				pSet->Edit.job.m_Head.bySourceType++;
			else
				pSet->Edit.job.m_Head.bySourceType= stTuner;
			switch(pSet->Edit.job.m_Head.bySourceType)
			{
			case stSongFile:
				tjSetSongType(&pSet->Edit.job, pstSongFile);
				tjSetSongID(&pSet->Edit.job, 1);
				break;
			case stTuner:
				tjSetTunerBand(&pSet->Edit.job, tuner_getBand());
				tjSetTunerFreq(&pSet->Edit.job, tuner_getCurrentFreq(0xFF));
				break;
			}
			break;
		case 3:	// start:hour
			if(pSet->Edit.job.m_Head.srtStartTime.byHour)
				pSet->Edit.job.m_Head.srtStartTime.byHour--;
			else
				pSet->Edit.job.m_Head.srtStartTime.byHour= 23;
			MyTimeAdd(&pSet->Edit.endTime,  tjGetStartTime(&pSet->Edit.job), tjGetPlayLength(&pSet->Edit.job));
			dspJobEndTime(&pSet->Edit, 5, pSet->byIndex);
			break;
		case 4:	// start:minute
		case 5:	// start:second
			pBuf= (u8 *)&pSet->Edit.job.m_Head.srtStartTime;
			index= pSet->byIndex-3;
			if(pBuf[index])
				pBuf[index]--;
			else
				pBuf[index]= 59;
			MyTimeAdd(&pSet->Edit.endTime,  tjGetStartTime(&pSet->Edit.job), tjGetPlayLength(&pSet->Edit.job));
			dspJobEndTime(&pSet->Edit, 5, pSet->byIndex);
			break;
		case 6:	// end:hour
			if(pSet->Edit.endTime.byHour)
				pSet->Edit.endTime.byHour--;
			else
				pSet->Edit.endTime.byHour= 23;
			break;
		case 7:	// end:minute
		case 8:	// end:second
			pBuf= (u8 *)&pSet->Edit.endTime;
			index= pSet->byIndex-6;
			if(pBuf[index])
				pBuf[index]--;
			else
				pBuf[index]= 59;
			break;
		default:
			return TRUE;
		}
		break;
	case VK_LEFT:
	case VK_RIGHT:
		if(pSet->byMode)
		{
			index= 0;
			switch(pSet->byIndex)
			{
			case 3:	// start:hour
			case 4:	// start:minute
			case 5:	// start:second
				index= _TimeIsValid(&pSet->Edit.job.m_Head.srtStartTime);
				if(index)
				{
					pSet->byIndex= index-1;
					nJobCount= 1;
				}
				break;
			case 6:	// end:hour
			case 7:	// end:minute
			case 8:	// end:second
				index= _TimeIsValid(&pSet->Edit.endTime);
				if(index)
				{
					pSet->byIndex= index+2;
					nJobCount= 1;
				}
				break;
			}
			if(index==0)
				pSet->byIndex= jobeditindex_edit_tab[KeyCode==VK_LEFT][pSet->byIndex];
		}else
		{
			if(nJobCount==0)
			{
				if(pSet->byIndex)
					pSet->byIndex= 0;
				else
					pSet->byIndex= 10;
				break;
			}
			pSet->byIndex= jobeditindex_normal_tab[KeyCode==VK_LEFT][pSet->byIndex];
		}
		break;
	case VK_ENTER:
		switch(pSet->byIndex)
		{
		case 0:	// group
		case 3:	// start:hour
		case 4:	// start:minute
		case 5:	// start:second
		case 6:	// end:hour
		case 7:	// end:minute
		case 8:	// end:second
			return TRUE;
		case 1:	// source
			switch(pSet->Edit.job.m_Head.bySourceType)
			{
			case stTuner:
				pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_JOBTUNERSET];	// 取得新的菜单
				break;
			case stSongFile:
				pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_JOBSONGSET];	// 取得新的菜单
				break;
			default:
				return TRUE;
			}		
			if(pNewMenu->InitFunc)
				pNewMenu->InitFunc(&pSet->Edit);
			break;
		case 2:	// port
			pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_PORTEDIT];
			if(pNewMenu->InitFunc)
				pNewMenu->InitFunc(&pSet->Edit.job);
			break;
		case 9:	// prev
			if(nJobCount==1)
				return TRUE;
			if(pSet->byJobID)
				pSet->byJobID--;
			else
				pSet->byJobID= nJobCount-1;
			_loadJobParam(pSet, tjgGetAt(pJobGroup, pSet->byJobID));
			break;
		case 10:	// add
			pSet->byMode= 1;
			_NewJobParam(pSet);
			pSet->byIndex= 1;
			break;
		case 11:	// edit/save
			if(pSet->byMode==0)
			{
				pSet->byMode= 2;
				pSet->byIndex= 1;
			}else
			{
				tjSetPlayLength(&pSet->Edit.job, (WORD)(MyTimeToSecond(&pSet->Edit.endTime)-MyTimeToSecond(tjGetStartTime(&pSet->Edit.job))));
				if(pSet->byMode==1)
				{
					tjAssign(tjgNew(pJobGroup), &pSet->Edit.job);
					pSet->byJobID= tjgGetCount(pJobGroup)-1;
					g_bTaskChange= TRUE;
				}else
				{
					pJob= tjgGetAt(pJobGroup, pSet->byJobID);
					if(memcmp(tjGetHeadBuffer(pJob), tjGetHeadBuffer(&pSet->Edit.job), tjGetHeadBufferSize())!=0)
					{
						tjAssign(pJob, &pSet->Edit.job);
						g_bTaskChange= TRUE;
					}
				}
				pSet->byMode= 0;
			}
			pJobGroup->m_bValid= 1;
			pJobGroup->m_nIndex=  pSet->byGroup+1;
			break;
		case 12:// delete
			g_bTaskChange= TRUE;
			tjgDeleteIndex(pJobGroup, pSet->byJobID);
			nJobCount= tjgGetCount(pJobGroup);
			if(nJobCount)
			{
				if(pSet->byJobID>=nJobCount)
					pSet->byJobID= nJobCount-1;
				_loadJobParam(pSet, tjgGetAt(pJobGroup, pSet->byJobID));
			}else
			{
				_InitJobParam(pSet);
				pSet->byJobID= 0;
				pSet->byIndex= 0;
			}
			break;
		case 13:// next
			if(nJobCount==1)
				return TRUE;
			if(pSet->byJobID<nJobCount-1)
				pSet->byJobID++;
			else
				pSet->byJobID= 0;
			_loadJobParam(pSet, tjgGetAt(pJobGroup, pSet->byJobID));
			break;
		}
		break;
	case VK_ESC:
		if(pSet->byMode)
		{
			if(nJobCount)
			{
				_loadJobParam(pSet, tjgGetAt(pJobGroup, pSet->byJobID));
				pSet->byIndex= 9+pSet->byMode;
			}else
			{
				_InitJobParam(pSet);
				pSet->byIndex= 10;
			}
			pSet->byMode= 0;
			break;
		}
		// 放弃数据保存，返回上一级
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}
// --------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------
typedef struct	{
	u8	byGroupIndex;		// 事件组编号
	u8	byMode;			// 编辑模式, 0--查看，1--添加，2--修改
	u8	byIndex;		// 位置索引
	u8	byDig;			// 输入位数
	CTaskGroup	group;	// 任务组数据
}GROUPLISTPARAM;

void InitGroupList(void *pItem)
{
	GROUPLISTPARAM *pSet= (GROUPLISTPARAM *)((PMENUITEM)&Menu_Tab[MENUID_GROUPLIST])->pParam;
	pSet->byMode= 0;
	pSet->byDig= 0;
	pSet->byGroupIndex=0;
	if(tglGetCount(&g_taskManager.m_lpTaskGroups))
	{
	pSet->byIndex= 12;
		tgAssign(&pSet->group, tglGetAt(&g_taskManager.m_lpTaskGroups, pSet->byGroupIndex));
	}	else
	{
		pSet->byIndex= 13;
		memset(&pSet->group, 0, sizeof(CTaskGroup));
	}
}

void dspGroupList(struct _TAGMENUITEM *pItem)
{
	u8 i;
	GROUPLISTPARAM *pSet= (GROUPLISTPARAM *)((PMENUITEM)&Menu_Tab[MENUID_GROUPLIST])->pParam;
	u8 tgCount= tglGetCount(&g_taskManager.m_lpTaskGroups);
	if(MENUID_GROUPLIST!= g_pCurMenuItem->byMenuID)
	{
		lcm_Clr();
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_GROUPLIST];
		dspDrawTitle();
		dspDrawRect();
		lcm_Putstr(102, 0, "00-00至00-00", 0, 0);
		lcm_Putstr(3, 16, sWeek, 4, 0);
		lcm_Putstr(40, 16, "一 二 三 四 五 六 日 特殊", 25, 0);
		lcm_Putstr(3, 32, "事件组 ", 7, 0);
	}
	strfmt(str_buf, "[%d/%d]", (pSet->byMode==1)?tgCount+1: (tgCount?pSet->byGroupIndex+1:0), tgCount);
	lcm_Putstr(66, 0, str_buf, 5, 0);

	strfmt(str_buf, sDig2Fmt, pSet->group.m_Head.srtStartDate.byMonth);
	lcm_Putstr(102, 0, str_buf, 2, pSet->byIndex==0);
	strfmt(str_buf, sDig2Fmt, pSet->group.m_Head.srtStartDate.byDay);
	lcm_Putstr(120, 0, str_buf, 2, pSet->byIndex==1);
	strfmt(str_buf, sDig2Fmt, pSet->group.m_Head.srtEndDate.byMonth);
	lcm_Putstr(144, 0, str_buf, 2, pSet->byIndex==2);
	strfmt(str_buf, sDig2Fmt, pSet->group.m_Head.srtEndDate.byDay);
	lcm_Putstr(162, 0, str_buf, 2, pSet->byIndex==3);

	for(i=0;i<7;i++)
	{
		str_buf[0]= pSet->group.m_Head.byTaskGroupOfWeek[i]+0x30;
		lcm_Putstr(44+i*18, 32, str_buf, 1, pSet->byIndex==i+4);
	}

	str_buf[0]= pSet->group.m_Head.bySpecialGroup+0x30;
	lcm_Putstr(178, 32, str_buf, 1, pSet->byIndex==11);
	
	// 显示功能菜单
	lcm_Putstr(2, 48, sPrev, 6, pSet->byIndex==12);
	lcm_Putstr(50, 48, sAdd, 4, pSet->byIndex==13);
	
	if(pSet->byMode)
		lcm_Putstr(84, 48, sSave, 4, pSet->byIndex==14);
	else
		lcm_Putstr(84, 48, sEdit, 4, pSet->byIndex==14);
	
	lcm_Putstr(118, 48, sDelete, 4, pSet->byIndex==15);
	lcm_Putstr(154, 48, sNext, 6, pSet->byIndex==16);
}

extern U8 DaysOfMonth[12];
u32 keydo_GroupList(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	static const u8 groupindex_normal_tab[][17]={
	    // 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16
		{0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,   0,  13, 14, 15, 16, 12},	// right
		{0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,   0,  16, 12, 13, 14, 15}		// left
	};
	static const u8 groupindex_edit_tab[][17]={
	    // 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16
		{1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 14,  0,  0,   0,   0,  0},	// right
		{14,0,  1,  2,  3,  4,  5,  6,  7,  8,   9,  10,  0,  0,  11,   0,  0}		// left
	};
	//NUMINOUTPARAM Num;
	PMENUITEM  pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_GROUPLIST];
	GROUPLISTPARAM *pSet= (GROUPLISTPARAM *)((PMENUITEM)&Menu_Tab[MENUID_GROUPLIST])->pParam;
	u8 tgCount= tglGetCount(&g_taskManager.m_lpTaskGroups), byDays;
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
		return FALSE;
	case VK_UP:
		switch(pSet->byIndex)
		{
		case 0:	// start:month
			if(pSet->group.m_Head.srtStartDate.byMonth<12)
				pSet->group.m_Head.srtStartDate.byMonth++;
			else
				pSet->group.m_Head.srtStartDate.byMonth= 1;
			break;
		case 1:	// start:day
			//byDays= DaysOfMonth[pSet->group.m_Head.srtStartDate.byMonth-1];
			if(pSet->group.m_Head.srtStartDate.byDay<DaysOfMonth[pSet->group.m_Head.srtStartDate.byMonth-1])
				pSet->group.m_Head.srtStartDate.byDay++;
			else
				pSet->group.m_Head.srtStartDate.byDay= 1;
			break;
		case 2:	// end:month
			if(pSet->group.m_Head.srtEndDate.byMonth<12)
				pSet->group.m_Head.srtEndDate.byMonth++;
			else
				pSet->group.m_Head.srtEndDate.byMonth= 1;
			break;
		case 3:	// end:day
			if(pSet->group.m_Head.srtEndDate.byDay<DaysOfMonth[pSet->group.m_Head.srtEndDate.byMonth-1])
				pSet->group.m_Head.srtEndDate.byDay++;
			else
				pSet->group.m_Head.srtEndDate.byDay= 1;
			break;
		case 4:	// 星期一事件组
		case 5:	// 星期二事件组
		case 6:	// 星期三事件组
		case 7:	// 星期四事件组
		case 8:	// 星期五事件组
		case 9:	// 星期六事件组
		case 10:	// 星期日事件组
			byDays= pSet->byIndex-4;
			if(pSet->group.m_Head.byTaskGroupOfWeek[byDays]<TASKGROUP_COUNT_OFLIST)
				pSet->group.m_Head.byTaskGroupOfWeek[byDays]++;
			else
				pSet->group.m_Head.byTaskGroupOfWeek[byDays]= 0;
			break;
		case 11:	// 特殊事件组
			if(pSet->group.m_Head.bySpecialGroup<TASKGROUP_COUNT_OFLIST)
				pSet->group.m_Head.bySpecialGroup++;
			else
				pSet->group.m_Head.bySpecialGroup= 1;
			break;
		default:
			return TRUE;
		}
		break;
	case VK_DOWN:
		switch(pSet->byIndex)
		{
		case 0:	// start:month
			if(pSet->group.m_Head.srtStartDate.byMonth>1)
				pSet->group.m_Head.srtStartDate.byMonth--;
			else
				pSet->group.m_Head.srtStartDate.byMonth= 12;
			break;
		case 1:	// start:day
			if(pSet->group.m_Head.srtStartDate.byDay>1)
				pSet->group.m_Head.srtStartDate.byDay--;
			else
				pSet->group.m_Head.srtStartDate.byDay= DaysOfMonth[pSet->group.m_Head.srtStartDate.byMonth-1];
			break;
		case 2:	// end:month
			if(pSet->group.m_Head.srtEndDate.byMonth>1)
				pSet->group.m_Head.srtEndDate.byMonth--;
			else
				pSet->group.m_Head.srtEndDate.byMonth= 12;
			break;
		case 3:	// end:day
			if(pSet->group.m_Head.srtEndDate.byDay>1)
				pSet->group.m_Head.srtEndDate.byDay--;
			else
				pSet->group.m_Head.srtEndDate.byDay= DaysOfMonth[pSet->group.m_Head.srtEndDate.byMonth-1];
			break;
		case 4:	// 星期一事件组
		case 5:	// 星期二事件组
		case 6:	// 星期三事件组
		case 7:	// 星期四事件组
		case 8:	// 星期五事件组
		case 9:	// 星期六事件组
		case 10:	// 星期日事件组
			byDays= pSet->byIndex-4;
			if(pSet->group.m_Head.byTaskGroupOfWeek[byDays]>0)
				pSet->group.m_Head.byTaskGroupOfWeek[byDays]--;
			else
				pSet->group.m_Head.byTaskGroupOfWeek[byDays]= TASKGROUP_COUNT_OFLIST;
			break;
		case 11:	// 特殊事件组
			if(pSet->group.m_Head.bySpecialGroup>1)
				pSet->group.m_Head.bySpecialGroup--;
			else
				pSet->group.m_Head.bySpecialGroup= TASKGROUP_COUNT_OFLIST;
			break;
		default:
			return TRUE;
		}
		break;
	case VK_LEFT:
	case VK_RIGHT:
		if(pSet->byMode)
		{
			tgCount= 0;
			switch(pSet->byIndex)
			{
			case 0:	// start:month
			case 1:	// start:day
				break;
			case 2:	// end:month
			case 3:	// end:day
				break;
			}			
			if(tgCount==0)
				pSet->byIndex= groupindex_edit_tab[KeyCode==VK_LEFT][pSet->byIndex];
		}else
		{
			if(tgCount==0)
			{
				pSet->byIndex= 13;	// add
				break;
			}
			pSet->byIndex= groupindex_normal_tab[KeyCode==VK_LEFT][pSet->byIndex];
		}
		break;
	case VK_ENTER:
		switch(pSet->byIndex)
		{
		case 12:	// prev
			if(pSet->byGroupIndex)
				pSet->byGroupIndex--;
			else
				pSet->byGroupIndex= tgCount-1;
			tgAssign(&pSet->group, tglGetAt(&g_taskManager.m_lpTaskGroups, pSet->byGroupIndex));
			break;
		case 13:// add
			if(tglCanAddNew(&g_taskManager.m_lpTaskGroups))
			{
				pSet->byMode= 1;
				tgInit(&pSet->group);
				pSet->byIndex= 0;
			}
			break;
		case 14:// edit/save
			if(pSet->byMode)	// save
			{
				if(pSet->byMode==1)	// add
					tgAssign(tglNew(&g_taskManager.m_lpTaskGroups), &pSet->group);
				else
					tgAssign(tglGetAt(&g_taskManager.m_lpTaskGroups, pSet->byGroupIndex), &pSet->group);
				pSet->byIndex= 12+pSet->byMode;
				pSet->byMode= 0;
				g_bTaskChange= TRUE;
			}else
			{
				pSet->byMode= 2;
				pSet->byIndex= 0;
			}
			break;
		case 15:// delete
			g_bTaskChange= TRUE;
			tglDeleteIndex(&g_taskManager.m_lpTaskGroups, pSet->byGroupIndex);
			tgCount= tglGetCount(&g_taskManager.m_lpTaskGroups);
			if(tgCount)
			{
				if(tgCount<=pSet->byGroupIndex)
				{
					pSet->byGroupIndex= tgCount-1;
				}
				tgAssign(&pSet->group, tglGetAt(&g_taskManager.m_lpTaskGroups, pSet->byGroupIndex));
			}else
			{
				memset(&pSet->group, 0, sizeof(CTaskGroup));
				pSet->byIndex= 13;
			}
			break;
		case 16:// next
			if(pSet->byGroupIndex<tgCount-1)
				pSet->byGroupIndex++;
			else
				pSet->byGroupIndex= 0;
			tgAssign(&pSet->group, tglGetAt(&g_taskManager.m_lpTaskGroups, pSet->byGroupIndex));
			break;
		default:
			return TRUE;
		}
		break;
	case VK_ESC:	
		if(pSet->byMode)
		{
			tgAssign(&pSet->group, tglGetAt(&g_taskManager.m_lpTaskGroups, pSet->byGroupIndex));
			if(tgCount)
				pSet->byIndex= 12+pSet->byMode;
			else
				pSet->byIndex= 13;
			pSet->byMode= 0;
			break;
		}
		// 放弃数据保存，返回上一级
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}
// --------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------
typedef struct	{
	u8	byTaskIndex;		// 任务编号
	u8	byMode;			// 编辑模式, 0--查看，1--添加，2--修改
	u8	byIndex;		// 位置索引
	u8	byDig;			// 输入位数
	CIntercutTask task;	// 任务组数据
}INTERCUTPARAM;

void InitIntercutTask(void *pItem)
{
	INTERCUTPARAM *pSet= (INTERCUTPARAM *)((PMENUITEM)&Menu_Tab[MENUID_INTERCUTLIST])->pParam;
	pSet->byMode= 0;
	pSet->byDig= 0;
	pSet->byTaskIndex=0;
	if(itgGetCount(&g_taskManager.m_lpIntercutTasks))
	{
		pSet->byIndex= 4;
		itAssign(&pSet->task, itgGetAt(&g_taskManager.m_lpIntercutTasks, pSet->byTaskIndex));
	}	else
	{
		pSet->byIndex= 5;
		// memset(&pSet->task, 0, sizeof(CIntercutTask));
		itInit(&pSet->task, &g_SysDate);
	}
}

void dspIntercutTask(struct _TAGMENUITEM *pItem)
{
	// u8 i;
	INTERCUTPARAM *pSet= (INTERCUTPARAM *)((PMENUITEM)&Menu_Tab[MENUID_INTERCUTLIST])->pParam;
	u8 itCount= itgGetCount(&g_taskManager.m_lpIntercutTasks);
	if(MENUID_INTERCUTLIST!= g_pCurMenuItem->byMenuID)
	{
		lcm_Clr();
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_INTERCUTLIST];
		dspDrawTitle();
		dspDrawRect();
		lcm_Putstr(3, 16, sDate, 4, 0);
		lcm_Putstr(33, 16, "    -  -  ", 10, 0);
		lcm_Putstr(3, 32, sContent, 8, 0);
	}
	// 编号
	strfmt(str_buf, sCountFmt, (pSet->byMode==1)?itCount+1:(itCount?pSet->byTaskIndex+1:0), itCount);
	lcm_Putstr(93, 0, str_buf, 9, 0);

	// 日期
	strfmt(str_buf, sDig4Fmt, pSet->task.m_Date.wYear);
	lcm_Putstr(intercutdateshowpos_tab[0], 16, str_buf, 4, pSet->byIndex==0);
	strfmt(str_buf, sDig2Fmt, pSet->task.m_Date.byMonth);
	lcm_Putstr(intercutdateshowpos_tab[1], 16, str_buf, 2, pSet->byIndex==1);
	strfmt(str_buf, sDig2Fmt, pSet->task.m_Date.byDay);
	lcm_Putstr(intercutdateshowpos_tab[2], 16, str_buf, 2, pSet->byIndex==2);

	// 显示音源
	lcm_Putstr(57, 32,  GetJobSourceString(&pSet->task.m_Job, str_buf, 22), 0, pSet->byIndex==3);
	
	// 显示功能菜单
	lcm_Putstr(2, 48, sPrev, 6, pSet->byIndex==4);
	lcm_Putstr(50, 48, sAdd, 4, pSet->byIndex==5);
	
	if(pSet->byMode)
		lcm_Putstr(84, 48, sSave, 4, pSet->byIndex==6);
	else
		lcm_Putstr(84, 48, sEdit, 4, pSet->byIndex==6);
	
	lcm_Putstr(118, 48, sDelete, 4, pSet->byIndex==7);
	lcm_Putstr(154, 48, sNext, 6, pSet->byIndex==8);
}

u32 keydo_IntercutTask(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	static const u8 intercutindex_normal_tab[][9]={
	    // 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16
		{0,  0,  0,  0,  5,  6,  7,  8,  4},	// right
		{0,  0,  0,  0,  8,  4,  5,  6,  7}		// left
	};
	static const u8 intercutindex_edit_tab[][9]={
	    // 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16
		{1,  2,  3,  6,  0,  0,  0,  0,  0},	// right
		{6,  0,  1,  2,  0,  0,  3,  0,  0}		// left
	};
	NUMINOUTPARAM Num;
	PMENUITEM  pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_INTERCUTLIST];
	INTERCUTPARAM *pSet= (INTERCUTPARAM *)((PMENUITEM)&Menu_Tab[MENUID_INTERCUTLIST])->pParam;
	u8 itCount= itgGetCount(&g_taskManager.m_lpIntercutTasks), byDig;
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
		switch(pSet->byIndex)
		{
		case 0:	// year
			Num.sub.wValue= pSet->task.m_Date.wYear;
			byDig= 4;
			break;
		case 1:	// month
			Num.sub.wValue= pSet->task.m_Date.byMonth;
			byDig=2;
			break;
		case 2:	// day
			Num.sub.wValue= pSet->task.m_Date.byDay;
			byDig=2;
			break;
		default:
			return TRUE;
		}
		Num.sub.byInputDig = pSet->byDig;
		if(Num.sub.byInputDig==0)
			Num.sub.wValue= 0;
		NumInputProc(&Num, KeyCode-1,  byDig);
		dspInputNum(intercutdateshowpos_tab[pSet->byIndex], 16, &Num, byDig, 1);
		pSet->byDig= Num.sub.byInputDig;
		switch(pSet->byIndex)
		{
		case 0:	// year
			pSet->task.m_Date.wYear= Num.sub.wValue;
			break;
		case 1:	// month
			pSet->task.m_Date.byMonth= Num.sub.wValue;
			break;
		case 2:	// day
			pSet->task.m_Date.byDay= Num.sub.wValue;
			break;
		}
		
		return TRUE;
	case VK_UP:
		switch(pSet->byIndex)
		{
		case 0:	// date:year
			if(pSet->task.m_Date.wYear<2112)
				pSet->task.m_Date.wYear++;
			else
				pSet->task.m_Date.wYear= 2013;
			break;
		case 1:	// date:month
			if(pSet->task.m_Date.byMonth<12)
				pSet->task.m_Date.byMonth++;
			else
				pSet->task.m_Date.byMonth= 1;
			break;
		case 2:	// date:day
			if(pSet->task.m_Date.byDay<DaysOfMonth[pSet->task.m_Date.byMonth-1])
				pSet->task.m_Date.byDay++;
			else
				pSet->task.m_Date.byDay= 1;
			break;
 		default:
			return TRUE;
		}
		break;
	case VK_DOWN:
		switch(pSet->byIndex)
		{
		case 0:	// date:year
			if(pSet->task.m_Date.wYear>2013)
				pSet->task.m_Date.wYear--;
			else
				pSet->task.m_Date.wYear= 2112;
			break;
		case 1:	// date:month
			if(pSet->task.m_Date.byMonth>1)
				pSet->task.m_Date.byMonth--;
			else
				pSet->task.m_Date.byMonth= 12;
			break;
		case 2:	// date:day
			if(pSet->task.m_Date.byDay>1)
				pSet->task.m_Date.byDay--;
			else
				pSet->task.m_Date.byDay= DaysOfMonth[pSet->task.m_Date.byMonth-1];
			break;
		default:
			return TRUE;
		}
		break;
	case VK_LEFT:
	case VK_RIGHT:
		if(pSet->byMode)
		{
			itCount= 0;
			switch(pSet->byIndex)
			{
			case 0:	// date:year
			case 1:	// date:month
			case 2:	// date:day
				itCount= _DateIsValid(&pSet->task.m_Date);
				if(itCount)
				{
					pSet->byIndex= itCount-1;
				}
				break;
			}
			if(itCount==0)
				pSet->byIndex= intercutindex_edit_tab[KeyCode==VK_LEFT][pSet->byIndex];
		}else
		{
			if(itCount==0)
			{
				pSet->byIndex= 5;	// add
				break;
			}
			pSet->byIndex= intercutindex_normal_tab[KeyCode==VK_LEFT][pSet->byIndex];
		}
		break;
	case VK_ENTER:
		switch(pSet->byIndex)
		{
		case 3:
			pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_SPECIALJOBEDIT];	// 取得新的菜单
			if(pNewMenu->InitFunc)
				pNewMenu->InitFunc(g_pCurMenuItem);
			break;
		case 4:	// prev
			if(pSet->byTaskIndex)
				pSet->byTaskIndex--;
			else
				pSet->byTaskIndex= itCount-1;
			itAssign(&pSet->task, itgGetAt(&g_taskManager.m_lpIntercutTasks, pSet->byTaskIndex));
			break;
		case 5:// add
			if(itgCanAddNew(&g_taskManager.m_lpIntercutTasks))
			{
				pSet->byMode= 1;
				itInit(&pSet->task, &g_SysDate);
				pSet->byIndex= 0;
			}
			break;
		case 6:// edit/save
			if(pSet->byMode)	// save
			{
				if(pSet->byMode==1)	// add
					itAssign(itgNew(&g_taskManager.m_lpIntercutTasks), &pSet->task);
				else
					itAssign(itgGetAt(&g_taskManager.m_lpIntercutTasks, pSet->byTaskIndex), &pSet->task);
				pSet->byIndex= 4+pSet->byMode;
				pSet->byMode= 0;
				g_bTaskChange= TRUE;
			}else
			{
				pSet->byMode= 2;
				pSet->byIndex= 0;
			}
			break;
		case 7:// delete
			g_bTaskChange= TRUE;
			itgDeleteIndex(&g_taskManager.m_lpIntercutTasks, pSet->byTaskIndex);
			itCount= itgGetCount(&g_taskManager.m_lpIntercutTasks);
			if(itCount)
			{
				if(itCount<=pSet->byTaskIndex)
				{
					pSet->byTaskIndex= itCount-1;
				}
				itAssign(&pSet->task, itgGetAt(&g_taskManager.m_lpIntercutTasks, pSet->byTaskIndex));
			}else
			{
				memset(&pSet->task, 0, sizeof(CIntercutTask));
				pSet->byIndex= 5;
			}
			break;
		case 8:// next
			if(pSet->byTaskIndex<itCount-1)
				pSet->byTaskIndex++;
			else
				pSet->byTaskIndex= 0;
			itAssign(&pSet->task, itgGetAt(&g_taskManager.m_lpIntercutTasks, pSet->byTaskIndex));
			break;
		default:
			return TRUE;
		}
		break;
	case VK_ESC:	
		if(pSet->byMode)
		{
			itAssign(&pSet->task, itgGetAt(&g_taskManager.m_lpIntercutTasks, pSet->byTaskIndex));
			if(itCount)
				pSet->byIndex= 4+pSet->byMode;
			else
				pSet->byIndex= 5;
			pSet->byMode= 0;
			break;
		}
		// 放弃数据保存，返回上一级
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}
// --------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------
//&mYkTask,									// pParam
//keydo_YkTask,								// KeyDoFunc
//InitYkTask,												// InitFunc;
//dspYkTask									// DisplayFunc
#if (GC_ZR & GC_ZR_REMOTE)
typedef struct	{
	u8	byTaskIndex;		// 任务编号
	u8	byMode;			// 编辑模式, 0--查看，1--添加，2--修改
	u8	byIndex;		// 位置索引
	u8	byDig;			// 输入位数
	CRemoteTask task;	// 任务组数据
}YKPARAM;

void InitYkTask(void *pItem)
{
	YKPARAM *pSet = (YKPARAM *)((PMENUITEM)&Menu_Tab[MENUID_YKEDIT])->pParam;
	pSet->byMode = 0;
	pSet->byDig = 0;
	pSet->byTaskIndex = 0;
	if (rtgGetCount(&g_taskManager.m_lpYkTasks))
	{
		pSet->byIndex = 3;
		rtAssign(&pSet->task, rtgGetAt(&g_taskManager.m_lpYkTasks, pSet->byTaskIndex));
	}
	else
	{
		pSet->byIndex = 4;
		// memset(&pSet->task, 0, sizeof(CIntercutTask));
		rtInit(&pSet->task, tttYk);
	}
}

void dspYkTask(struct _TAGMENUITEM *pItem)
{
	u8 i;
	YKPARAM *pSet = (YKPARAM *)((PMENUITEM)&Menu_Tab[MENUID_YKEDIT])->pParam;
	u8 rtCount = rtgGetCount(&g_taskManager.m_lpYkTasks);
	if (MENUID_YKEDIT != g_pCurMenuItem->byMenuID)
	{
		lcm_Clr();
		g_pCurMenuItem = (PMENUITEM)&Menu_Tab[MENUID_YKEDIT];
		dspDrawTitle();
		dspDrawRect();
		lcm_Putstr(3, 16, sKey, 8, 0);
	}
	// 编号
	strfmt(str_buf, sCountFmt, (pSet->byMode == 1) ? rtCount + 1 : (rtCount ? pSet->byTaskIndex + 1 : 0), rtCount);
	lcm_Putstr(93, 0, str_buf, 9, 0);

	// 按键
	strfmt(str_buf, sDig2Fmt, pSet->task.m_Key +1);
	lcm_Putstr(57, 16, str_buf, 2, pSet->byIndex == 0);

	// 类型	
	if (rtGetTaskType(&pSet->task)){//music
		lcm_Putstr(154, 16, sMusic, 4, pSet->byIndex == 1);
		// 显示音源
		lcm_Putstr(3, 32, sContent, 8, 0);
		lcm_Putstr(57, 32, GetJobSourceString(&pSet->task.m_Job, str_buf, 22), 0, pSet->byIndex == 2);
	}
	else {//funcs
		lcm_Putstr(154, 16, sFuncs, 4, pSet->byIndex == 1);
		// 显示按键功能
		lcm_Putstr(3, 32, sKeyFuncs, 8, 0);
		for (i = 0; i<30; i++)
			str_buf[i] = ' ';
		str_buf[30] = 0;
		lcm_Putstr(57, 32, str_buf, 22, 0);
		lcm_Putstr(57, 32, sKeyFuncsValue[rtGetTaskFuncValue(&pSet->task)], 9, pSet->byIndex == 2);
	}

	// 显示功能菜单
	lcm_Putstr(2, 48, sPrev, 6, pSet->byIndex == 3);
	lcm_Putstr(50, 48, sAdd, 4, pSet->byIndex == 4);

	if (pSet->byMode)
		lcm_Putstr(84, 48, sSave, 4, pSet->byIndex == 5);
	else
		lcm_Putstr(84, 48, sEdit, 4, pSet->byIndex == 5);

	lcm_Putstr(118, 48, sDelete, 4, pSet->byIndex == 6);
	lcm_Putstr(154, 48, sNext, 6, pSet->byIndex == 7);
}

u32 keydo_YkTask(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	static const u8 ykindex_normal_tab[][9] = {
		// 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16
		{ 0, 0, 0, 4, 5, 6, 7, 3 },	// right
		{ 0, 0, 0, 7, 3, 4, 5, 6 }		// left
	};
	static const u8 ykindex_edit_tab[][9] = {
		// 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16
		{ 1, 2, 5, 0, 0, 0, 0, 0 },	// right
		{ 5, 0, 1, 0, 0, 2, 0, 0 }		// left
	};
	NUMINOUTPARAM Num;
	PMENUITEM  pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_YKEDIT];
	YKPARAM *pSet = (YKPARAM *)((PMENUITEM)&Menu_Tab[MENUID_YKEDIT])->pParam;
	u8 rtCount = rtgGetCount(&g_taskManager.m_lpYkTasks), byDig;
	switch (KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
		if (pSet->byIndex == 0) {
			Num.sub.wValue = pSet->task.m_Key;
			byDig = 2;
		}
		else
			return TRUE;
		Num.sub.byInputDig = pSet->byDig;
		if (Num.sub.byInputDig == 0)
			Num.sub.wValue = 0;
		NumInputProc(&Num, KeyCode - 1, byDig);
		dspInputNum(57, 16, &Num, byDig, 1);
		pSet->byDig = Num.sub.byInputDig;
		if (pSet->byIndex == 0) {
			pSet->task.m_Key = Num.sub.wValue -1;
		}
		return TRUE;
	case VK_UP:
		switch (pSet->byIndex) {
		case 0: 
			if (pSet->task.m_Key < 12 - 1)
				pSet->task.m_Key++;
			else
				pSet->task.m_Key = 0;
			break;
		case 1:
			if (rtGetTaskType(&pSet->task) == 0){
				rtSetTaskFuncValue(&pSet->task, 6);
			}
			break;
		case 2:
			if (rtGetTaskType(&pSet->task) == 0) {//func
				rtCount = rtGetTaskFuncValue(&pSet->task);
				if (rtCount < 1) {
					rtCount++;
				}
				else
					rtCount = 0;
				rtSetTaskFuncValue(&pSet->task, rtCount);
			}
			else {
			}
			break;
		default:
			return TRUE;
		}
		break;
	case VK_DOWN:
		switch (pSet->byIndex) {
		case 0:
			if (pSet->task.m_Key > 0)
				pSet->task.m_Key--;
			else
				pSet->task.m_Key = 12 - 1;
			break;
		case 1:
			if (rtGetTaskType(&pSet->task) > 0){
				rtSetTaskFuncValue(&pSet->task, 0);
			}
			break;
		case 2:
			if (rtGetTaskType(&pSet->task) == 0) {//func
				rtCount = rtGetTaskFuncValue(&pSet->task);
				if (rtCount > 0) {
					rtCount--;
				}
				else
					rtCount = 1;
				rtSetTaskFuncValue(&pSet->task, rtCount);
			}
			else {

			}
			break;
		default:
			return TRUE;
		}
		break;
	case VK_LEFT:
	case VK_RIGHT:
		if (pSet->byMode)
		{
			rtCount = pSet->task.m_Key;
			if (rtCount > 12 - 1) {
				pSet->byIndex = 0;
			}
			else
				pSet->byIndex = ykindex_edit_tab[KeyCode == VK_LEFT][pSet->byIndex];
		}
		else
		{
			if (rtCount == 0)
			{
				pSet->byIndex = 4;	// add
				break;
			}
			pSet->byIndex = ykindex_normal_tab[KeyCode == VK_LEFT][pSet->byIndex];
		}
		break;
	case VK_ENTER:
		switch (pSet->byIndex)
		{
		case 2:
			if (rtGetTaskType(&pSet->task) > 0) {
				pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_SPECIALJOBEDIT];	// 取得新的菜单
				if (pNewMenu->InitFunc)
					pNewMenu->InitFunc(g_pCurMenuItem);
			}
			break;
		case 3:	// prev
			if (pSet->byTaskIndex)
				pSet->byTaskIndex--;
			else
				pSet->byTaskIndex = rtCount - 1;
			rtAssign(&pSet->task, rtgGetAt(&g_taskManager.m_lpYkTasks, pSet->byTaskIndex));
			break;
		case 4:// add
			if (rtgCanAddNew(&g_taskManager.m_lpYkTasks))
			{
				pSet->byMode = 1;
				rtInit(&pSet->task, tttYk);
				pSet->byIndex = 0;
			}
			break;
		case 5:// edit/save
			if (pSet->byMode)	// save
			{
				if (pSet->byMode == 1)	// add
					rtAssign(rtgNew(&g_taskManager.m_lpYkTasks), &pSet->task);
				else
					rtAssign(rtgGetAt(&g_taskManager.m_lpYkTasks, pSet->byTaskIndex), &pSet->task);
				pSet->byIndex = 3 + pSet->byMode;
				pSet->byMode = 0;
				g_bTaskChange = TRUE;
			}
			else
			{
				pSet->byMode = 2;
				pSet->byIndex = 0;
			}
			break;
		case 6:// delete
			g_bTaskChange = TRUE;
			rtgDeleteIndex(&g_taskManager.m_lpYkTasks, pSet->byTaskIndex);
			rtCount = rtgGetCount(&g_taskManager.m_lpYkTasks);
			if (rtCount)
			{
				if (rtCount <= pSet->byTaskIndex)
				{
					pSet->byTaskIndex = rtCount - 1;
				}
				rtAssign(&pSet->task, rtgGetAt(&g_taskManager.m_lpYkTasks, pSet->byTaskIndex));
			}
			else
			{
				memset(&pSet->task, 0, sizeof(CRemoteTask));
				pSet->byIndex = 4;
			}
			break;
		case 7:// next
			if (pSet->byTaskIndex<rtCount - 1)
				pSet->byTaskIndex++;
			else
				pSet->byTaskIndex = 0;
			rtAssign(&pSet->task, rtgGetAt(&g_taskManager.m_lpYkTasks, pSet->byTaskIndex));
			break;
		default:
			return TRUE;
		}
		break;
	case VK_ESC:
		if (pSet->byMode)
		{
			rtAssign(&pSet->task, rtgGetAt(&g_taskManager.m_lpYkTasks, pSet->byTaskIndex));
			if (rtCount)
				pSet->byIndex = 3 + pSet->byMode;
			else
				pSet->byIndex = 4;
			pSet->byMode = 0;
			break;
		}
		// 放弃数据保存，返回上一级
		pNewMenu = (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}

// --------------------------------------------------------------------------------------------------------------
typedef struct	{
	u8	byTaskIndex;		// 任务编号
	u8	byMode;			// 编辑模式, 0--查看，1--添加，2--修改
	u8	byIndex;		// 位置索引
	u8	byDig;			// 输入位数
	CRemoteTask task;	// 任务组数据
}PHONEPARAM;

void InitPhoneTask(void *pItem){

	PHONEPARAM *pSet = (PHONEPARAM *)((PMENUITEM)&Menu_Tab[MENUID_PHONEEDIT])->pParam;
	pSet->byMode = 0;
	pSet->byDig = 0;
	pSet->byTaskIndex = 0;
	if (rtgGetCount(&g_taskManager.m_lpPhoneTasks))
	{
		pSet->byIndex = 3;
		rtAssign(&pSet->task, rtgGetAt(&g_taskManager.m_lpPhoneTasks, pSet->byTaskIndex));
	}
	else
	{
		pSet->byIndex = 4;
		// memset(&pSet->task, 0, sizeof(CIntercutTask));
		rtInit(&pSet->task, tttPhone);
	}
}

void dspPhoneTask(struct _TAGMENUITEM *pItem){

	u8 i;
	PHONEPARAM *pSet = (PHONEPARAM *)((PMENUITEM)&Menu_Tab[MENUID_PHONEEDIT])->pParam;
	u8 rtCount = rtgGetCount(&g_taskManager.m_lpPhoneTasks);
	if (MENUID_PHONEEDIT != g_pCurMenuItem->byMenuID)
	{
		lcm_Clr();
		g_pCurMenuItem = (PMENUITEM)&Menu_Tab[MENUID_PHONEEDIT];
		dspDrawTitle();
		dspDrawRect();
		lcm_Putstr(3, 16, sKey, 8, 0);
	}
	// 编号
	strfmt(str_buf, sCountFmt, (pSet->byMode == 1) ? rtCount + 1 : (rtCount ? pSet->byTaskIndex + 1 : 0), rtCount);
	lcm_Putstr(93, 0, str_buf, 9, 0);

	// 按键
	strfmt(str_buf, sDig6Fmt, pSet->task.m_Key);
	lcm_Putstr(57, 16, str_buf, 6, pSet->byIndex == 0);

	// 类型	
	if (rtGetTaskType(&pSet->task)) {//music
		lcm_Putstr(154, 16, sMusic, 4, pSet->byIndex == 1);
		// 显示音源
		lcm_Putstr(3, 32, sContent, 8, 0);
		lcm_Putstr(57, 32, GetJobSourceString(&pSet->task.m_Job, str_buf, 22), 0, pSet->byIndex == 2);
	}
	else {//funcs
		lcm_Putstr(154, 16, sFuncs, 4, pSet->byIndex == 1);
		// 显示按键功能
		lcm_Putstr(3, 32, sKeyFuncs, 8, 0);
		for (i = 0; i<30; i++)
			str_buf[i] = ' ';
		str_buf[30] = 0;
		lcm_Putstr(57, 32, str_buf, 22, 0);
		lcm_Putstr(57, 32, sKeyFuncsValue[rtGetTaskFuncValue(&pSet->task)], 9, pSet->byIndex == 2);
	}

	// 显示功能菜单
	lcm_Putstr(2, 48, sPrev, 6, pSet->byIndex == 3);
	lcm_Putstr(50, 48, sAdd, 4, pSet->byIndex == 4);

	if (pSet->byMode)
		lcm_Putstr(84, 48, sSave, 4, pSet->byIndex == 5);
	else
		lcm_Putstr(84, 48, sEdit, 4, pSet->byIndex == 5);

	lcm_Putstr(118, 48, sDelete, 4, pSet->byIndex == 6);
	lcm_Putstr(154, 48, sNext, 6, pSet->byIndex == 7);
}

u32 keydo_PhoneTask(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask){
	static const u8 phoneindex_normal_tab[][9] = {
		// 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16
		{ 0, 0, 0, 4, 5, 6, 7, 3 },	// right
		{ 0, 0, 0, 7, 3, 4, 5, 6 }		// left
	};
	static const u8 phoneindex_edit_tab[][9] = {
		// 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16
		{ 1, 2, 5, 0, 0, 0, 0, 0 },	// right
		{ 5, 0, 1, 0, 0, 2, 0, 0 }		// left
	};
	NUMINOUTPARAM Num;
	PMENUITEM  pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_PHONEEDIT];
	PHONEPARAM *pSet = (PHONEPARAM *)((PMENUITEM)&Menu_Tab[MENUID_PHONEEDIT])->pParam;
	u8 rtCount = rtgGetCount(&g_taskManager.m_lpPhoneTasks), byDig;
	switch (KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
		if (pSet->byIndex == 0) {
			Num.sub.wValue = pSet->task.m_Key;
			byDig = 6;
		}
		else
			return TRUE;
		Num.sub.byInputDig = pSet->byDig;
		if (Num.sub.byInputDig == 0)
			Num.sub.wValue = 0;
		NumInputProc(&Num, KeyCode - 1, byDig);
		dspInputNum(57, 16, &Num, byDig, 1);
		pSet->byDig = Num.sub.byInputDig;
		if (pSet->byIndex == 0) {
			pSet->task.m_Key = Num.sub.wValue;
		}
		return TRUE;
	case VK_UP:
		switch (pSet->byIndex) {
		case 0:
			if (pSet->task.m_Key < 1000000 - 1)
				pSet->task.m_Key++;
			else
				pSet->task.m_Key = 0;
			break;
		case 1:
			if (rtGetTaskType(&pSet->task) == 0) {
				rtSetTaskFuncValue(&pSet->task, 6);
			}
			break;
		case 2:
			if (rtGetTaskType(&pSet->task) == 0) {//func
				rtCount = rtGetTaskFuncValue(&pSet->task);
				if (rtCount < 1) {
					rtCount++;
				}
				else
					rtCount = 0;
				rtSetTaskFuncValue(&pSet->task, rtCount);
			}
			else {
			}
			break;
		default:
			return TRUE;
		}
		break;
	case VK_DOWN:
		switch (pSet->byIndex) {
		case 0:
			if (pSet->task.m_Key > 0)
				pSet->task.m_Key--;
			else
				pSet->task.m_Key = 1000000 - 1;
			break;
		case 1:
			if (rtGetTaskType(&pSet->task) > 0) {
				rtSetTaskFuncValue(&pSet->task, 0);
			}
			break;
		case 2:
			if (rtGetTaskType(&pSet->task) == 0) {//func
				rtCount = rtGetTaskFuncValue(&pSet->task);
				if (rtCount > 0) {
					rtCount--;
				}
				else
					rtCount = 1;
				rtSetTaskFuncValue(&pSet->task, rtCount);
			}
			else {

			}
			break;
		default:
			return TRUE;
		}
		break;
	case VK_LEFT:
	case VK_RIGHT:
		if (pSet->byMode)
		{
			rtCount = pSet->task.m_Key;
			if (rtCount > 1000000 - 1) {
				pSet->byIndex = 0;
			}
			else
				pSet->byIndex = phoneindex_edit_tab[KeyCode == VK_LEFT][pSet->byIndex];
		}
		else
		{
			if (rtCount == 0)
			{
				pSet->byIndex = 4;	// add
				break;
			}
			pSet->byIndex = phoneindex_normal_tab[KeyCode == VK_LEFT][pSet->byIndex];
		}
		break;
	case VK_ENTER:
		switch (pSet->byIndex)
		{
		case 2:
			if (rtGetTaskType(&pSet->task) > 0) {
				pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_SPECIALJOBEDIT];	// 取得新的菜单
				if (pNewMenu->InitFunc)
					pNewMenu->InitFunc(g_pCurMenuItem);
			}
			break;
		case 3:	// prev
			if (pSet->byTaskIndex)
				pSet->byTaskIndex--;
			else
				pSet->byTaskIndex = rtCount - 1;
			rtAssign(&pSet->task, rtgGetAt(&g_taskManager.m_lpPhoneTasks, pSet->byTaskIndex));
			break;
		case 4:// add
			if (rtgCanAddNew(&g_taskManager.m_lpPhoneTasks))
			{
				pSet->byMode = 1;
				rtInit(&pSet->task, tttPhone);
				pSet->byIndex = 0;
			}
			break;
		case 5:// edit/save
			if (pSet->byMode)	// save
			{
				if (pSet->byMode == 1)	// add
					rtAssign(rtgNew(&g_taskManager.m_lpPhoneTasks), &pSet->task);
				else
					rtAssign(rtgGetAt(&g_taskManager.m_lpPhoneTasks, pSet->byTaskIndex), &pSet->task);
				pSet->byIndex = 3 + pSet->byMode;
				pSet->byMode = 0;
				g_bTaskChange = TRUE;
			}
			else
			{
				pSet->byMode = 2;
				pSet->byIndex = 0;
			}
			break;
		case 6:// delete
			g_bTaskChange = TRUE;
			rtgDeleteIndex(&g_taskManager.m_lpPhoneTasks, pSet->byTaskIndex);
			rtCount = rtgGetCount(&g_taskManager.m_lpPhoneTasks);
			if (rtCount)
			{
				if (rtCount <= pSet->byTaskIndex)
				{
					pSet->byTaskIndex = rtCount - 1;
				}
				rtAssign(&pSet->task, rtgGetAt(&g_taskManager.m_lpPhoneTasks, pSet->byTaskIndex));
			}
			else
			{
				memset(&pSet->task, 0, sizeof(CRemoteTask));
				pSet->byIndex = 4;
			}
			break;
		case 7:// next
			if (pSet->byTaskIndex<rtCount - 1)
				pSet->byTaskIndex++;
			else
				pSet->byTaskIndex = 0;
			rtAssign(&pSet->task, rtgGetAt(&g_taskManager.m_lpPhoneTasks, pSet->byTaskIndex));
			break;
		default:
			return TRUE;
		}
		break;
	case VK_ESC:
		if (pSet->byMode)
		{
			rtAssign(&pSet->task, rtgGetAt(&g_taskManager.m_lpPhoneTasks, pSet->byTaskIndex));
			if (rtCount)
				pSet->byIndex = 3 + pSet->byMode;
			else
				pSet->byIndex = 4;
			pSet->byMode = 0;
			break;
		}
		// 放弃数据保存，返回上一级
		pNewMenu = (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}
#endif
// --------------------------------------------------------------------------------------------------------------
typedef struct	{
	DIR	dj;
	u8	bySongPath;				// 路径标识0-SD		1-USB
	u8	byCurRow;				// 当前反显行
	u16	wFileCount;				// 乐曲文件总数 (包含有效文件夹)
	u16	wFileIndex;				// 当前文件编号
}SONGVIEWPARAM;

void dspSongView(SONGVIEWPARAM *pView)
{
	u8 i, j;
	u16 wCnt=0xFFFF, wInd; 
	///XXX
	u16 viewCount;
	char lfn[60];
	FILINFO	fno;
	viewCount = pView->wFileCount;
	if(f_opendir(&pView->dj, sSongPaths[pView->bySongPath])!=FR_OK)
	{
		lcm_Putstr(42, 32, sSongPathOpenError, 18, 0);
		return;
	}
	if(viewCount==0)
	{
		lcm_Putstr(54, 32, sNoSongFile, 14, 0);
		return ;
	}
	fno.lfname= &lfn[5];
	fno.lfsize= 50;
	wInd= pView->wFileIndex-pView->byCurRow;
	if(wInd>viewCount)
	{
		wInd= viewCount-(0xFFFF-wInd)-1;
	}
	for(i=0;i<3; i++)
	{
		if(i<viewCount)
		{
			if(wInd>=viewCount)
			{
				wInd= 0;
				wCnt= 0xFFFF;
				if(f_opendir(&pView->dj, sSongPaths[pView->bySongPath])!=FR_OK) return;
			}
			strfmt(lfn, "[%03d]", wInd+1);
			while(f_readdir(&pView->dj, &fno)==FR_OK)
			{
				if( fno.fname[0] == 0) break;
				if((fno.fattrib&AM_DIR)==0)
				{
					if ( IsSongFile(fno.fname))	// 是乐曲文件
					{
						wCnt++;
						if(wCnt==wInd)
							break;
					}
				}
         ///XXX读取文件夹
        else{
					if(IsSongDir(fno.fname))
					{
							wCnt++;
							if(wCnt==wInd)
								break;
          }
				}
			}
		}
		if(wInd==wCnt)	// 没找到
		{
			wInd++;
			for(j=strlen(fno.lfname);j<30; j++)
				fno.lfname[j]= ' ';
		}else
			memset(lfn, ' ', 31);
		lcm_Putstr(3, (i+1)<<4, lfn, 30, i==pView->byCurRow);
	}
}

void dspSongViewMem(struct _TAGMENUITEM *pItem)
{
	SONGVIEWPARAM *pSet= (SONGVIEWPARAM *)((PMENUITEM)&Menu_Tab[MENUID_SONGVIEWMEM])->pParam;
	if((!g_pCurMenuItem)||(MENUID_SONGVIEWMEM!= g_pCurMenuItem->byMenuID) )
	{
		lcm_Clr();
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_SONGVIEWMEM];
		dspDrawTitle();
		dspDrawRect();
		pSet->bySongPath= 0;
		pSet->byCurRow= 0;
		pSet->wFileIndex= 0;
		GetDirectoryInfo(sSongPaths[0], FALSE, NULL, &pSet->wFileCount);
	}
	dspSongView(pSet);
}

u32 keydo_SongViewMem(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM  pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_SONGVIEWMEM];
	SONGVIEWPARAM *pSet= (SONGVIEWPARAM *)pNewMenu->pParam;
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
		return FALSE;
	case VK_UP:
		if(pSet->byCurRow)
			pSet->byCurRow--;
		pSet->wFileIndex--;
		if(pSet->wFileIndex>pSet->wFileCount)
			pSet->wFileIndex= pSet->wFileCount-1;
		break;
	case VK_DOWN:
		if(pSet->byCurRow<2)
			pSet->byCurRow++;
		pSet->wFileIndex++;
		if(pSet->wFileIndex>=pSet->wFileCount)
			pSet->wFileIndex= 0;
		break;
	case VK_LEFT:
	case VK_RIGHT:
	case VK_ENTER:	
		return TRUE;
	case VK_ESC:	
		// 放弃数据保存，返回上一级
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}
// --------------------------------------------------------------------------------------------------------------
void dspSongViewUSB(struct _TAGMENUITEM *pItem)
{
	SONGVIEWPARAM *pSet= (SONGVIEWPARAM *)((PMENUITEM)&Menu_Tab[MENUID_SONGVIEWUSB])->pParam;
	if((!g_pCurMenuItem)||(MENUID_SONGVIEWUSB!= g_pCurMenuItem->byMenuID) )
	{
		lcm_Clr();
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_SONGVIEWUSB];
		dspDrawTitle();
		dspDrawRect();
		pSet->bySongPath= 1;
		pSet->byCurRow= 0;
		pSet->wFileIndex= 0;
		GetDirectoryInfo(sSongPaths[1], FALSE, NULL, &pSet->wFileCount);
	}
	dspSongView(pSet);
}

u32 keydo_SongViewUSB(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM  pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_SONGVIEWUSB];
	SONGVIEWPARAM *pSet= (SONGVIEWPARAM *)pNewMenu->pParam;
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
		return FALSE;
	case VK_UP:
		if(pSet->byCurRow)
			pSet->byCurRow--;
		pSet->wFileIndex--;
		if(pSet->wFileIndex>pSet->wFileCount)
			pSet->wFileIndex= pSet->wFileCount-1;
		break;
	case VK_DOWN:
		if(pSet->byCurRow<2)
			pSet->byCurRow++;
		pSet->wFileIndex++;
		if(pSet->wFileIndex>=pSet->wFileCount)
			pSet->wFileIndex= 0;
		break;
	case VK_LEFT:
	case VK_RIGHT:
	case VK_ENTER:	
		return TRUE;
	case VK_ESC:	
		// 放弃数据保存，返回上一级
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}
// --------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------
u8 const timesetpos_tab[][2]={	{106, 16}, {142, 16}, {166, 16},
								{142, 32}, {160, 32}, {178, 32} };

typedef struct {
	TMyDate  d;
	TMyTime t;
	u8 byIndex:6;
	u8 byDig:2;
}TIMESETPARAM;

void dspTimeSet(struct _TAGMENUITEM *pItem)
{
	TIMESETPARAM	*pSet=(TIMESETPARAM *) ((PMENUITEM)&Menu_Tab[MENUID_TIMESET])->pParam;
	if(g_pCurMenuItem->byMenuID!= MENUID_TIMESET)
	{
		lcm_Clr();
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_TIMESET];
		memcpy(&pSet->d, &g_SysDate, 4);
		memcpy(&pSet->t, &g_SysTime, 3);
		pSet->byIndex= 0;
		pSet->byDig= 0;
		dspDrawTitle();
		dspDrawRect();
		lcm_Putstr(3, 16, "日期", 4, 0);
		lcm_Putstr(130, 16, "年  月  日", 14, 0);
		lcm_Putstr(3, 32, "时间", 4, 0);
		lcm_Putstr(154, 32, ":  :  ", 8, 0);
	}
	strfmt(str_buf, "%04d", pSet->d.wYear);
	lcm_Putstr(timesetpos_tab[0][0], timesetpos_tab[0][1], str_buf, 4, pSet->byIndex==0);
	strfmt(str_buf, "%02d", pSet->d.byMonth);
	lcm_Putstr(timesetpos_tab[1][0], timesetpos_tab[1][1], str_buf, 2, pSet->byIndex==1);
	strfmt(str_buf, "%02d", pSet->d.byDay);
	lcm_Putstr(timesetpos_tab[2][0], timesetpos_tab[2][1], str_buf, 2, pSet->byIndex==2);
	
	strfmt(str_buf, "%02d", pSet->t.byHour);
	lcm_Putstr(timesetpos_tab[3][0], timesetpos_tab[3][1], str_buf, 2, pSet->byIndex==3);
	strfmt(str_buf, "%02d", pSet->t.byMinute);
	lcm_Putstr(timesetpos_tab[4][0], timesetpos_tab[4][1], str_buf, 2, pSet->byIndex==4);
	strfmt(str_buf, "%02d", pSet->t.bySecond);
	lcm_Putstr(timesetpos_tab[5][0], timesetpos_tab[5][1], str_buf, 2, pSet->byIndex==5);
	lcm_Putstr(84, 48, sSave, 0, pSet->byIndex==6);
}

u32 keydo_TimeSet(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM  pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_TIMESET];
	TIMESETPARAM *pSet= (TIMESETPARAM *)pNewMenu->pParam;
	NUMINOUTPARAM Num;
	u8 byDig=2, index;
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
		switch(pSet->byIndex)
		{
		case 0:	// year
			Num.sub.wValue= pSet->d.wYear;
			byDig= 4;
			break;
		case 1:	// month
			Num.sub.wValue= pSet->d.byMonth;
			break;
		case 2:	// day
			Num.sub.wValue= pSet->d.byDay;
			break;
		case 3:	// hour
			Num.sub.wValue= pSet->t.byHour;
			break;
		case 4:	// minute
			Num.sub.wValue= pSet->t.byMinute;
			break;
		case 5:	// second
			Num.sub.wValue= pSet->t.bySecond;
			break;
		default:
			return TRUE;
		}
		Num.sub.byInputDig = pSet->byDig;
		if(Num.sub.byInputDig==0)
			Num.sub.wValue= 0;
		NumInputProc(&Num, KeyCode-1,  byDig);
		dspInputNum(timesetpos_tab[pSet->byIndex][0], timesetpos_tab[pSet->byIndex][1], &Num, byDig, 1);
		pSet->byDig= Num.sub.byInputDig;
		switch(pSet->byIndex)
		{
		case 0:	// year
			pSet->d.wYear= Num.sub.wValue;
			break;
		case 1:	// month
			pSet->d.byMonth= Num.sub.wValue;
			break;
		case 2:	// day
			pSet->d.byDay= Num.sub.wValue;
			break;
		case 3:	// hour
			pSet->t.byHour= Num.sub.wValue;
			break;
		case 4:	// minute
			pSet->t.byMinute= Num.sub.wValue;
			break;
		case 5:	// second
			pSet->t.bySecond= Num.sub.wValue;
			break;
		}
		
		return TRUE;
	case VK_UP:
		switch(pSet->byIndex)
		{
		case 0:	// year
			if(pSet->d.wYear<2109)
				pSet->d.wYear++;
			else
				pSet->d.wYear= 2013;
			break;
		case 1:	// month
			if(pSet->d.byMonth<12)
				pSet->d.byMonth++;
			else
				pSet->d.byMonth= 1;
			break;
		case 2:	// day
			if(pSet->d.byDay<GetDayCount(pSet->d.wYear, pSet->d.byMonth))
				pSet->d.byDay++;
			else
				pSet->d.byDay= 1;
			break;
		case 3:	// hour
			if(pSet->t.byHour<23)
				pSet->t.byHour++;
			else
				pSet->t.byHour= 0;
			break;
		case 4:	// minute
			if(pSet->t.byMinute<59)
				pSet->t.byMinute++;
			else
				pSet->t.byMinute= 0;
			break;
		case 5:	// second
			if(pSet->t.bySecond<59)
				pSet->t.bySecond++;
			else
				pSet->t.bySecond= 0;
			break;
		}
		break;
	case VK_DOWN:
		switch(pSet->byIndex)
		{
		case 0:	// year
			if(pSet->d.wYear>2013)
				pSet->d.wYear--;
			else
				pSet->d.wYear= 2109;
			break;
		case 1:	// month
			if(pSet->d.byMonth>1)
				pSet->d.byMonth--;
			else
				pSet->d.byMonth= 12;
			break;
		case 2:	// day
			if(pSet->d.byDay>1)
				pSet->d.byDay--;
			else
				pSet->d.byDay= GetDayCount(pSet->d.wYear, pSet->d.byMonth);
			break;
		case 3:	// hour
			if(pSet->t.byHour>0)
				pSet->t.byHour--;
			else
				pSet->t.byHour= 23;
			break;
		case 4:	// minute
			if(pSet->t.byMinute>0)
				pSet->t.byMinute--;
			else
				pSet->t.byMinute= 59;
			break;
		case 5:	// second
			if(pSet->t.bySecond>0)
				pSet->t.bySecond--;
			else
				pSet->t.bySecond= 59;
			break;
		}
		break;
	case VK_LEFT:
		if(pSet->byIndex)
		{
			pSet->byIndex--;
		}else
			pSet->byIndex= 6;
		break;
	case VK_RIGHT:
		if((pSet->byIndex)>=6)
			pSet->byIndex= 0;
		else
			pSet->byIndex++;
		break;
	case VK_ENTER:	
		if(pSet->byIndex == 6)
		{
				// 检查输入是否合法
			index= _DateIsValid(&pSet->d);
			if(index)
			{
				pSet->byIndex= index-1;
				break;
			}
			index= _TimeIsValid(&pSet->t);
			if(index)
			{
				pSet->byIndex= index-1;
				break;
			}
			// 保存相关数据，返回上一级
			rtc_SetDateTime(&pSet->d, &pSet->t, MyDateOfWeek(&pSet->d));
			pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		}
		break; 
	case VK_ESC:	
		// 放弃数据保存，返回上一级
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}
// --------------------------------------------------------------------------------------------------------------
typedef struct {
	u8 bInited;
	u8 byIndex;
	u8 byParam1;
	u8 byParam2;
	char	sPwd[4];
	char sConfirmPwd[4];
}PASSWORDSETPARAM;

void dspPasswordSet(struct _TAGMENUITEM *pItem)
{
	u8 i;
	PASSWORDSETPARAM *pSet=(PASSWORDSETPARAM *) ((PMENUITEM)&Menu_Tab[MENUID_PASSWORDSET])->pParam;
	if(g_pCurMenuItem->byMenuID!= MENUID_PASSWORDSET)
	{
		lcm_Clr();
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_PASSWORDSET];
		pSet->byIndex= 0;
		pSet->byParam1= 0;
		pSet->byParam2= 0;
		dspDrawTitle();
		dspDrawRect();
		lcm_Putstr(3, 24, "输入密码:", 0, 0);
		lcm_Putstr(3, 40, "再输入一次:", 0, 0);
	}
	for(i=0;i<pSet->byParam1; i++)
		str_buf[i]= '*';
	for(; i<4; i++)
		str_buf[i]= '_';
	str_buf[i]= 0;
	lcm_Putstr(166, 24, str_buf, 4, pSet->byIndex==0);
	for(i=0;i<pSet->byParam2; i++)
		str_buf[i]= '*';
	for(; i<4; i++)
		str_buf[i]= '_';
	str_buf[i]= 0;
	lcm_Putstr(166, 40, str_buf, 4, pSet->byIndex==1);
}

u32 keydo_PasswordSet(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM  pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_PASSWORDSET];
	PASSWORDSETPARAM *pSet= (PASSWORDSETPARAM *)pNewMenu->pParam;
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
		if(pSet->byIndex)
		{
			if(pSet->byParam2==4)
				pSet->byParam2= 0;
			pSet->sConfirmPwd[pSet->byParam2++]= 0x2F+KeyCode;
		}else
		{
			if(pSet->byParam1==4)
				pSet->byParam1= 0;
			pSet->sPwd[pSet->byParam1++]= 0x2F+KeyCode;
		}
		break;
	case VK_UP:
	case VK_DOWN:
		return TRUE;
	case VK_LEFT:
	case VK_RIGHT:
		if(pSet->byIndex)
			pSet->byIndex= 0;
		else
			pSet->byIndex= 1;
		break;
	case VK_ENTER:	
 		if((pSet->byParam1==4)&&(pSet->byParam2==4))
		{
			pSet->byParam1= 0;
			pSet->byParam2= 0;
			if(strncmp(pSet->sPwd, pSet->sConfirmPwd, 4)==0)
			{
				// 保存密码到存储器
				strncpy(sysParam.sPwd, pSet->sPwd, 4);
				SaveSystemParam();
			}else
			  break;
		}
	case VK_ESC:	
		// 放弃数据保存，返回上一级
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}
// --------------------------------------------------------------------------------------------------------------

#if(GC_ZR & GC_ZR_BJ)
typedef struct {
	u8 bInited;
	u8 byIndex;
	u8 wBjFcType;
	EDITJOB pEdit;
}BJSETPARAM;


void InitBjSet(void *pItem /* EDITJOB* */)
{
	BJSETPARAM *pSet = (BJSETPARAM *)((PMENUITEM)&Menu_Tab[MENUID_BJSET])->pParam;
	pSet->byIndex = 0;
	tjAssign(&(pSet->pEdit.job), &sysParam.job);
	pSet->pEdit.job.m_Head.byTimeType = tttBj;
	pSet->wBjFcType = sysParam.wBjFcType;
}

void dspBjSet(struct _TAGMENUITEM *pItem)
{
	BJSETPARAM *pSet = (BJSETPARAM *)((PMENUITEM)&Menu_Tab[MENUID_BJSET])->pParam;
	if (g_pCurMenuItem->byMenuID != MENUID_BJSET)
	{
		lcm_Clr();
		g_pCurMenuItem = (PMENUITEM)&Menu_Tab[MENUID_BJSET];
		dspDrawTitle();
		dspDrawRect();
		lcm_Putstr(3, 24, sBjMusic, 8, 0);
		lcm_Putstr(3, 40, sBjFcType, 8, 0);
	}
	//报警音乐
	if(pSet->pEdit.job.m_Head.bySourceType == stSongFile)
		lcm_Putstr(63, 24, GetJobSourceString(&pSet->pEdit.job, str_buf, 20), 0, pSet->byIndex == 0);
	else if (pSet->pEdit.job.m_Head.bySourceType == stAUX) {
		lcm_Putstr(63, 24, "                    ", 0, 0);
		lcm_Putstr(63, 24, sSourceName[2], 0, pSet->byIndex == 0);
	}
	//报警分区器
	lcm_Putstr(63, 40, pSet->wBjFcType > 0 ? sSelected : sUnselected, 0, pSet->byIndex == 1);
	lcm_Putstr(113, 40, sSave, 4, pSet->byIndex == 2);
}

u32 keydo_BjSet(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM  pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_BJSET];
	BJSETPARAM *pSet = (BJSETPARAM *)pNewMenu->pParam;
	switch (KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
		return TRUE;
	case VK_LEFT:
		if (pSet->byIndex > 0)
			pSet->byIndex--;
		else
			pSet->byIndex = 2;
		break;
	case VK_RIGHT:
		if (pSet->byIndex < 2)
			pSet->byIndex++;
		else
			pSet->byIndex = 0;
		break;
	case VK_UP:
		pSet->pEdit.job.m_Head.bySourceType = stAUX;
		break;
	case VK_DOWN:
		pSet->pEdit.job.m_Head.bySourceType = stSongFile;
		break;
	case VK_ENTER:
		switch (pSet->byIndex)
		{
		case 0:	// source
			if (pSet->pEdit.job.m_Head.bySourceType == stSongFile) {
				pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_JOBSONGSET];
				if (pNewMenu->InitFunc)
					pNewMenu->InitFunc(&pSet->pEdit);
			}
			break;
		case 1:
			pSet->wBjFcType = pSet->wBjFcType >0 ? 0 : 1;
			break;
		case 2:
			sysParam.wBjFcType = pSet->wBjFcType;
			tjAssign(&sysParam.job, &(pSet->pEdit.job));
			SaveSystemParam();
			pNewMenu = (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];
		default:
			break;
		}
		break;

	case VK_ESC:
		// 放弃数据保存，返回上一级
		pNewMenu = (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}

#endif
// --------------------------------------------------------------------------------------------------------------
typedef struct	{
	u8	byTitlePos;		// 标题开始显示位置
	u8	byTitleLen;		// 标题显示长度
	u8	byIndex;		// 位置索引
	u8	byDig;			// 输入位数
	char *sTitle;			// 标题文字
	EDITJOB	Edit;
}SPECIALJOBEDITPARAM;
// --------------------------------------------------------------------------------------------------------------
typedef struct {
	SONGVIEWPARAM *pSongView;
	EDITJOB *pEdit;
	char	fname[13];
	//u8 byReturnMenuID;
}JOBSONGSETPARAM;

// pItem为CTaskJob*
void InitJobSongSet(void *pItem /* EDITJOB* */)
{
	JOBSONGSETPARAM *pSet=(JOBSONGSETPARAM *) ((PMENUITEM)&Menu_Tab[MENUID_JOBSONGSET])->pParam;
	pSet->pEdit= (EDITJOB *)pItem;
	pSet->pSongView= (SONGVIEWPARAM *) ((PMENUITEM)&Menu_Tab[MENUID_SONGVIEWMEM])->pParam;
	pSet->pSongView->bySongPath= 0;
	pSet->pSongView->byCurRow= 0;
	GetDirectoryInfo(sSongPaths[0], TRUE, NULL, &(pSet->pSongView->wFileCount));
	FindSongno(sSongPaths[0], tjGetSongID(&pSet->pEdit->job), TRUE, NULL, NULL, &(pSet->pSongView->wFileIndex));
}

void dspSongNumView(JOBSONGSETPARAM *pSet)
{
	u8 i, j;
	u16 wCnt=0xFFFF, wInd;
	char lfn[60];
	FILINFO	fno;
	SONGVIEWPARAM *pView= pSet->pSongView;
	if(f_opendir(&pView->dj, sSongPaths[pView->bySongPath])!=FR_OK)
	{
		lcm_Putstr(42, 32, sSongPathOpenError, 18, 0);
		return;
	}
	if(pView->wFileCount==0)
	{
		lcm_Putstr(54, 32, sNoSongFile, 14, 0);
		return ;
	}
	fno.lfname= &lfn[5];
	fno.lfsize= 50;
	wInd= pView->wFileIndex-pView->byCurRow;
	if(wInd>pView->wFileCount)
	{
		wInd= pView->wFileCount-(0xFFFF-wInd)-1;
	}
	for(i=0;i<3; i++)
	{
		if(i<pView->wFileCount)
		{
			if(wInd>=pView->wFileCount)
			{
				wInd= 0;
				wCnt= 0xFFFF;
				if(f_opendir(&pView->dj, sSongPaths[pView->bySongPath])!=FR_OK) return;
			}
			strfmt(lfn, "[%03d]", wInd+1);
			while(f_readdir(&pView->dj, &fno)==FR_OK)
			{
				if( fno.fname[0] == 0) break;
				if((fno.fattrib&AM_DIR)==0)
				{
					if ( IsSongFile(fno.fname))	// 是乐曲文件
					{
						if(getSongno(fno.fname))
							wCnt++;
						if(wCnt==wInd)
							break;
					}
				}
				else
				{
					if(IsSongDir(fno.fname))
							wCnt++;
						if(wCnt==wInd)
							break;
				}
			}
		}
		if(wInd==wCnt)	// 找到
		{
			if(i==pView->byCurRow)
				strncpy(pSet->fname, fno.fname, 13);
			wInd++;
			for(j=strlen(fno.lfname);j<30; j++)
				fno.lfname[j]= ' ';
		}else
			memset(lfn, ' ', 31);
		lcm_Putstr(3, (i+1)<<4, lfn, 30, i==pView->byCurRow);
	}
}

void dspJobSongSet(struct _TAGMENUITEM *pItem)
{
	JOBSONGSETPARAM *pSet=(JOBSONGSETPARAM *) ((PMENUITEM)&Menu_Tab[MENUID_JOBSONGSET])->pParam;
	if(g_pCurMenuItem->byMenuID!= MENUID_JOBSONGSET)
	{
		lcm_Clr();
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_JOBSONGSET];
		dspDrawTitle();
		dspDrawRect();
	}
	dspSongNumView(pSet);
}

u32 keydo_JobSongSet(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM  pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_JOBSONGSET];
	JOBSONGSETPARAM *pSet=(JOBSONGSETPARAM *)pNewMenu->pParam;
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
	case VK_LEFT:
	case VK_RIGHT:
		return TRUE;
	case VK_UP:
		if(pSet->pSongView->byCurRow)
			pSet->pSongView->byCurRow--;
		pSet->pSongView->wFileIndex--;
		if(pSet->pSongView->wFileIndex>pSet->pSongView->wFileCount)
			pSet->pSongView->wFileIndex= pSet->pSongView->wFileCount-1;
		break;
	case VK_DOWN:
		if(pSet->pSongView->byCurRow<2)
			pSet->pSongView->byCurRow++;
		pSet->pSongView->wFileIndex++;
		if(pSet->pSongView->wFileIndex>=pSet->pSongView->wFileCount)
			pSet->pSongView->wFileIndex= 0;
		break;
	case VK_ENTER:
		tjSetSongID(&pSet->pEdit->job, getSongno(pSet->fname));
		strcpy(str_buf, sSongPaths[0]);
		strncat(str_buf, pSet->fname, 13);
		tjSetPlayLength(&pSet->pEdit->job, GetAudioLength(str_buf));
		MyTimeAdd(&pSet->pEdit->endTime, tjGetStartTime(&pSet->pEdit->job), tjGetPlayLength(&pSet->pEdit->job));
	case VK_ESC:	
		// 放弃数据保存，返回上一级
		if(pSet->pEdit->job.m_Head.byTimeType==tttIntercut)
			pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_SPECIALJOBEDIT];	// 取得新的菜单
#if (GC_ZR & GC_ZR_REMOTE)
		else if (pSet->pEdit->job.m_Head.byTimeType == tttYk)
			pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_SPECIALJOBEDIT];	// 取得新的菜单
		else if (pSet->pEdit->job.m_Head.byTimeType == tttPhone)
			pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_SPECIALJOBEDIT];	// 取得新的菜单
#endif
#if (GC_ZR & GC_ZR_BJ)
		else if (pSet->pEdit->job.m_Head.byTimeType == tttBj)
			pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_BJSET];	// 取得新的菜单		
#endif
		else
			pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_JOBEDIT];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}
// --------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------
typedef struct {
	u8 byIndex;
	u8 byChn:6;
	u8 byBand:2;
	NUMINOUTPARAM Input;
	EDITJOB*pEdit;
}JOBTUNERSETPARAM;

// pItem为EDITJOB*
void InitJobTunerSet(void *pItem /* EDITJOB* */)
{
	JOBTUNERSETPARAM *pSet=(JOBTUNERSETPARAM *) ((PMENUITEM)&Menu_Tab[MENUID_JOBTUNERSET])->pParam;
	pSet->pEdit= (EDITJOB*)pItem;
	pSet->byIndex= 0;
	pSet->byChn= 0;
	pSet->byBand= tjGetTunerBand(&pSet->pEdit->job);
	pSet->Input.w= tjGetTunerFreq(&pSet->pEdit->job);
	if(pSet->byBand==tbFM)
		pSet->Input.w /= 10;
}

void dspJobTunerSet(struct _TAGMENUITEM *pItem)
{
	u16 wFreq;
	JOBTUNERSETPARAM *pSet=(JOBTUNERSETPARAM *) ((PMENUITEM)&Menu_Tab[MENUID_JOBTUNERSET])->pParam;
	if(g_pCurMenuItem->byMenuID!= MENUID_JOBTUNERSET)
	{
		lcm_Clr();
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_JOBTUNERSET];
		dspDrawTitle();
		dspDrawRect();
		lcm_Putstr(3, 16, "波段:", 0, 0);
		lcm_Putstr(3, 32, "频率:", 0, 0);
		lcm_Putstr(3, 48, "频道:", 0, 0);
	}
	lcm_Putstr(48, 16, tuner_getBandName(pSet->byBand), 0, 0);//pSet->byIndex==0);
	wFreq= pSet->Input.sub.wValue;
	if(pSet->byBand==tbFM)
		wFreq *= 10;
	lcm_Putstr(48, 32, tuner_getFreqString(pSet->byBand, wFreq, str_buf), 10, pSet->byIndex==0);
	if(pSet->byChn)
	{
		dspChannel(48, 48, pSet->byChn, pSet->byIndex==1);
	}else
		lcm_Putstr(48, 48, " 无效", 5,  pSet->byIndex==1);
}

void dspFMFreqInputNum(u8 x, u8 y, NUMINOUTPARAM *pInput, u8 instead)
{
	u8 i, n= pInput->sub.byInputDig==0?4:pInput->sub.byInputDig;
	char str[10];
	if(n<4)
	{
		strfmt(str, "%%0%dd", n);
		strfmt(str_buf, str, pInput->sub.wValue);
		for(i=n; i<5; i++)
			if(i==3)
				str_buf[i]= '.';
			else
				str_buf[i]= '_';
	}else
		strfmt(str_buf, "% 3d.%d", pInput->sub.wValue / 10, pInput->sub.wValue % 10);
	lcm_Putstr(x, y, str_buf, 5, instead);
}

u32 keydo_JobTunerSet(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM  pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_JOBTUNERSET];
	JOBTUNERSETPARAM *pSet=(JOBTUNERSETPARAM *) pNewMenu->pParam;
	u16	wTmp, wFreq;
	BOOL bRes;
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
		switch(pSet->byIndex)
		{
		case 0:
			bRes= NumInputProc(&pSet->Input, KeyCode-1, 4);
			if(pSet->byBand==tbFM)
				dspFMFreqInputNum(48, 32, &pSet->Input, 0);
			else
				dspInputNum(48, 32, &pSet->Input, 4, 0);
			wFreq= pSet->Input.sub.wValue;
			if(bRes)
			{
				wTmp= tuner_getMinFreq(pSet->byBand);
				if(pSet->byBand==tbFM)
					wTmp /= 10;
				if(wFreq<wTmp)
				{
					wFreq= wTmp; 
				}else
				{
					wTmp= tuner_getMaxFreq(pSet->byBand);
					if(pSet->byBand==tbFM)
						wTmp /= 10;
					if(wFreq>wTmp)
					{
						wFreq= wTmp;
						break;
					}
				}			
			}
			pSet->Input.sub.wValue= wFreq;
			break;
		default:
			return TRUE;
		}
		break;
	case VK_UP:
		switch(pSet->byIndex)
		{
		case 8:	// 波段未用
			if(pSet->byBand==tbFM)
				pSet->byBand= tbAM;
			else
				pSet->byBand= tbFM;
			pSet->Input.w= tuner_getCurrentFreq(pSet->byBand);
			if(pSet->byBand==tbFM)
				pSet->Input.w /= 10;
			pSet->byChn= 0;
			break;
		case 0:
			wFreq= tuner_getMaxFreq(pSet->byBand);
			wTmp= tuner_getStepFreq(pSet->byBand);
			if(pSet->byBand==tbFM)
			{
				wFreq/= wTmp;
				wTmp= 1;
			}
			if(pSet->Input.sub.wValue<wFreq)
				pSet->Input.sub.wValue+=wTmp;
			else
				pSet->Input.sub.wValue= wFreq;
			break;
		case 1:
			wTmp= tuner_getStationCount(pSet->byBand);
			if(wTmp==0) break;
			if(pSet->byChn<wTmp)
				pSet->byChn++;
			else
				pSet->byChn= 1;
			wFreq= tuner_getStationFreq(pSet->byBand, pSet->byChn-1);
			if(pSet->byBand==tbFM)
				wFreq /= 10;
			pSet->Input.sub.wValue= wFreq;
			break;
		}
		break;	
	case VK_DOWN:
		switch(pSet->byIndex)
		{
		case 8:// 波段未用
			if(pSet->byBand==tbFM)
				pSet->byBand= tbAM;
			else
				pSet->byBand= tbFM;
			pSet->Input.w= tuner_getCurrentFreq(pSet->byBand);
			if(pSet->byBand==tbFM)
				pSet->Input.w /= 10;
			pSet->byChn= 0;
			break;
		case 0:
			wFreq= tuner_getMinFreq(pSet->byBand);
			wTmp= tuner_getStepFreq(pSet->byBand);
			if(pSet->byBand==tbFM)
			{
				wFreq/= wTmp;
				wTmp= 1;
			}
			if(pSet->Input.sub.wValue>wFreq)
				pSet->Input.sub.wValue-=wTmp;
			else
				pSet->Input.sub.wValue= wFreq;
			break;
		case 1:
			wTmp= tuner_getStationCount(pSet->byBand);
			if(wTmp==0) break;
			if(pSet->byChn>1)
				pSet->byChn--;
			else
				pSet->byChn= wTmp;
			wFreq= tuner_getStationFreq(pSet->byBand, pSet->byChn-1);
			if(pSet->byBand==tbFM)
				wFreq /= 10;
			pSet->Input.sub.wValue= wFreq;
			break;
		}
		break;
	case VK_LEFT:
		if(pSet->byIndex)
			pSet->byIndex--;
		else
			pSet->byIndex= 1;
		break;
	case VK_RIGHT:
		if(pSet->byIndex<1)
			pSet->byIndex++;
		else
			pSet->byIndex= 0;
		break;
	case VK_ENTER:
		wFreq= pSet->Input.sub.wValue;
		if(pSet->byBand==tbFM)
			wFreq *= 10;
		tjSetTunerBand(&pSet->pEdit->job, pSet->byBand);
		tjSetTunerFreq(&pSet->pEdit->job, wFreq);
	case VK_ESC:	
		// 放弃数据保存，返回上一级
		if(pSet->pEdit->job.m_Head.byTimeType==tttIntercut)
			pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_SPECIALJOBEDIT];	// 取得新的菜单
#if (GC_ZR & GC_ZR_REMOTE)
		else if (pSet->pEdit->job.m_Head.byTimeType == tttYk)
			pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_SPECIALJOBEDIT];	// 取得新的菜单
		else if (pSet->pEdit->job.m_Head.byTimeType == tttPhone)
			pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_SPECIALJOBEDIT];	// 取得新的菜单
#endif
		else
			pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_JOBEDIT];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}
// --------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------

void InitSpecialJobEdit(void *pItem /* MENUITEM * */)
{
	SPECIALJOBEDITPARAM *pSet= (SPECIALJOBEDITPARAM *)((PMENUITEM)&Menu_Tab[MENUID_SPECIALJOBEDIT])->pParam;
	PMENUITEM	pMenu= (PMENUITEM)pItem;
	INTERCUTPARAM	*pIntercut;
#if (GC_ZR & GC_ZR_REMOTE)
	YKPARAM *pYk;
	PHONEPARAM *pPhone;
#endif
	switch(pMenu->byMenuID)
	{
	case MENUID_INTERCUTLIST:
		pIntercut = (INTERCUTPARAM *)pMenu->pParam;
		tjAssign(&pSet->Edit.job, &pIntercut->task.m_Job);
		break;
#if (GC_ZR & GC_ZR_REMOTE)
	case MENUID_YKEDIT:
		pYk = (YKPARAM *)pMenu->pParam;
		tjAssign(&pSet->Edit.job, &pYk->task.m_Job);
		break;
	case MENUID_PHONEEDIT:
		pPhone = (PHONEPARAM *)pMenu->pParam;
		tjAssign(&pSet->Edit.job, &pPhone->task.m_Job);
		break;
#endif
	default:
		return;
	}
	pSet->sTitle= (char *)pMenu->Title_szName;
	pSet->byTitlePos= (192-pMenu->Title_len*6-24) / 2;
	pSet->byTitleLen= pMenu->Title_len;
	MyTimeAdd(&pSet->Edit.endTime, tjGetStartTime(&pSet->Edit.job), tjGetPlayLength(&pSet->Edit.job));
}

void dspSpecialJobEdit(struct _TAGMENUITEM *pItem)
{
	u8 i, j;
	SPECIALJOBEDITPARAM *pSet= (SPECIALJOBEDITPARAM *)((PMENUITEM)&Menu_Tab[MENUID_SPECIALJOBEDIT])->pParam;
	if((!g_pCurMenuItem)||(MENUID_SPECIALJOBEDIT!= g_pCurMenuItem->byMenuID) )
	{
		lcm_Clr();
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_SPECIALJOBEDIT];
		dspDrawRect();
		lcm_Linexy(1, 14, 190, 14);
		lcm_Linexy(1, 13, 190, 13);
		for(i=0; i<pSet->byTitleLen; i++)
			str_buf[i]= pSet->sTitle[i];
		j=0;
		for(; i<pSet->byTitleLen+4; i++)
			str_buf[i]= sEdit[j++];
		str_buf[i]= 0;
		lcm_Putstr(pSet->byTitlePos,  0, str_buf, 0, 0);
		lcm_Putstr(2, 16, sSource, 4, 0);
#if (GC_ZR & GC_ZR_FC)
		lcm_Putstr(130, 16, sPort, 4, 0);
#else
		lcm_Putstr(136, 16, sPort, 4, 0);
#endif
		
		if (pSet->Edit.job.m_Head.byTimeType == tttYk) {
		}
		else if (pSet->Edit.job.m_Head.byTimeType == tttPhone) {
		}
		else {
			lcm_Putstr(2, 32, sStart, 4, 0);
		}
		lcm_Putstr(118, 32, sEnd, 4, 0);
		strcpy(str_buf, sZeroTime);
	
		if (pSet->Edit.job.m_Head.byTimeType == tttYk) {
		}
		else if (pSet->Edit.job.m_Head.byTimeType == tttPhone) {
		}
		else {
			lcm_Putstr(jobtimecolpos_tab[0], 32, str_buf, 8, 0);
		}
		lcm_Putstr(jobtimecolpos_tab[3], 32, str_buf, 8, 0);
	}
	
	// 音源
#if (GC_ZR & GC_ZR_FC)
	lcm_Putstr(30, 16, GetJobSourceString(&pSet->Edit.job, str_buf, 16), 16, pSet->byIndex == 0);
#else
	lcm_Putstr(30, 16, GetJobSourceString(&pSet->Edit.job, str_buf, 17), 17, pSet->byIndex == 0);
#endif
	// 分区
#if (GC_ZR & GC_ZR_FC)
	lcm_Putstr(154, 16, str_buf, GetJobPortStateString(&pSet->Edit.job, str_buf), pSet->byIndex == 1);
#else
	lcm_Putstr(166, 16, str_buf, GetJobPortStateString(&pSet->Edit.job, str_buf), pSet->byIndex == 1);
#endif
	// 开始时间
	
#if (GC_ZR & GC_ZR_REMOTE)
	if (pSet->Edit.job.m_Head.byTimeType == tttYk) {
	}
	else if (pSet->Edit.job.m_Head.byTimeType == tttPhone) {
	}
	else {
		strfmt(str_buf, sDig2Fmt, pSet->Edit.job.m_Head.srtStartTime.byHour);
		lcm_Putstr(jobtimecolpos_tab[0], 32, str_buf, 2, pSet->byIndex==2);
		
		strfmt(str_buf, sDig2Fmt, pSet->Edit.job.m_Head.srtStartTime.byMinute);
		lcm_Putstr(jobtimecolpos_tab[1], 32, str_buf, 2, pSet->byIndex==3);
	
		strfmt(str_buf, sDig2Fmt, pSet->Edit.job.m_Head.srtStartTime.bySecond);
		lcm_Putstr(jobtimecolpos_tab[2], 32, str_buf, 2, pSet->byIndex==4);
	}
#else
	strfmt(str_buf, sDig2Fmt, pSet->Edit.job.m_Head.srtStartTime.byHour);
	lcm_Putstr(jobtimecolpos_tab[0], 32, str_buf, 2, pSet->byIndex==2);
	
	strfmt(str_buf, sDig2Fmt, pSet->Edit.job.m_Head.srtStartTime.byMinute);
	lcm_Putstr(jobtimecolpos_tab[1], 32, str_buf, 2, pSet->byIndex==3);
	
	strfmt(str_buf, sDig2Fmt, pSet->Edit.job.m_Head.srtStartTime.bySecond);
	lcm_Putstr(jobtimecolpos_tab[2], 32, str_buf, 2, pSet->byIndex==4);
#endif
	
	dspJobEndTime(&pSet->Edit, 5, pSet->byIndex);	

	// 显示功能菜单
	lcm_Putstr(84, 48, sSave, 4, pSet->byIndex==8);
}

u32 keydo_SpecialJobEdit(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	NUMINOUTPARAM Num;
	u8 *pBuf, index;
	PMENUITEM  pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_SPECIALJOBEDIT];
	SPECIALJOBEDITPARAM *pSet= (SPECIALJOBEDITPARAM *)pNewMenu->pParam;
	CTaskJob		*pJob=NULL;

	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
		switch(pSet->byIndex)
		{
		case 2:	// start:hour
		case 3:	// start:minute
		case 4:	// start:second
			pBuf= (u8 *)&pSet->Edit.job.m_Head.srtStartTime;
			index= pSet->byIndex-2;
			Num.sub.wValue= pBuf[index];
			break;
		case 5:	// end:hour
		case 6:	// end:minute
		case 7:	// end:second
			pBuf= (u8 *)&pSet->Edit.endTime;
			index= pSet->byIndex-5;
			Num.sub.wValue= pBuf[index];
			index+= 3;
			break;
		default:
			return TRUE;
		}
		Num.sub.byInputDig = pSet->byDig;
		//if(Num.sub.byInputDig==0)
		//	Num.sub.wValue= 0;
		NumInputProc(&Num, KeyCode-1,  2);
		dspInputNum(jobtimecolpos_tab[index], 32, &Num, 2, 1);
		pSet->byDig= Num.sub.byInputDig;
		switch(pSet->byIndex)
		{
		case 2:	// start:hour
		case 3:	// start:minute
		case 4:	// start:second
			pBuf[pSet->byIndex-2]= Num.sub.wValue;
			MyTimeAdd(&pSet->Edit.endTime,  tjGetStartTime(&pSet->Edit.job), tjGetPlayLength(&pSet->Edit.job));
			dspJobEndTime(&pSet->Edit, 5, pSet->byIndex);
			break;
		case 5:	// end:hour
		case 6:	// end:minute
		case 7:	// end:second
			pBuf[pSet->byIndex-5]= Num.sub.wValue;
			
			break;
		}
		return TRUE;
	case VK_UP:
		switch(pSet->byIndex)
		{
		case 0:
			if(pSet->Edit.job.m_Head.bySourceType>stTuner)
				pSet->Edit.job.m_Head.bySourceType--;
			else
				pSet->Edit.job.m_Head.bySourceType= stMicphone;
			switch(pSet->Edit.job.m_Head.bySourceType)
			{
			case stSongFile:
				tjSetSongType(&pSet->Edit.job, pstSongFile);
				tjSetSongID(&pSet->Edit.job, 1);
				break;
			case stTuner:
				tjSetTunerBand(&pSet->Edit.job, tuner_getBand());
				tjSetTunerFreq(&pSet->Edit.job, tuner_getCurrentFreq(0xFF));
				break;
			}
			break;
		case 2:	// start:hour
			if(pSet->Edit.job.m_Head.srtStartTime.byHour<23)
				pSet->Edit.job.m_Head.srtStartTime.byHour++;
			else
				pSet->Edit.job.m_Head.srtStartTime.byHour= 0;
			MyTimeAdd(&pSet->Edit.endTime,  tjGetStartTime(&pSet->Edit.job), tjGetPlayLength(&pSet->Edit.job));
			dspJobEndTime(&pSet->Edit, 5, pSet->byIndex);
			break;
		case 3:	// start:minute
		case 4:	// start:second
			pBuf= (u8 *)&pSet->Edit.job.m_Head.srtStartTime;
			index= pSet->byIndex-2;
			if(pBuf[index]<59)
				pBuf[index]++;
			else
				pBuf[index]= 0;
			MyTimeAdd(&pSet->Edit.endTime,  tjGetStartTime(&pSet->Edit.job), tjGetPlayLength(&pSet->Edit.job));
			dspJobEndTime(&pSet->Edit, 4, pSet->byIndex);
			break;
		case 5:	// end:hour
			if(pSet->Edit.endTime.byHour<23)
				pSet->Edit.endTime.byHour++;
			else
				pSet->Edit.endTime.byHour= 0;
			break;
		case 6:	// end:minute
		case 7:	// end:second
			pBuf= (u8 *)&pSet->Edit.endTime;
			index= pSet->byIndex-5;
			if(pBuf[index]<59)
				pBuf[index]++;
			else
				pBuf[index]= 0;
			break;
		default:
			return TRUE;
		}
		break;
	case VK_DOWN:
		switch(pSet->byIndex)
		{
		case 0:
			if(pSet->Edit.job.m_Head.bySourceType<stMicphone)
				pSet->Edit.job.m_Head.bySourceType++;
			else
				pSet->Edit.job.m_Head.bySourceType= stTuner;
			switch(pSet->Edit.job.m_Head.bySourceType)
			{
			case stSongFile:
				tjSetSongType(&pSet->Edit.job, pstSongFile);
				tjSetSongID(&pSet->Edit.job, 1);
				break;
			case stTuner:
				tjSetTunerBand(&pSet->Edit.job, tuner_getBand());
				tjSetTunerFreq(&pSet->Edit.job, tuner_getCurrentFreq(0xFF));
				break;
			}
			break;
		case 2:	// start:hour
			if(pSet->Edit.job.m_Head.srtStartTime.byHour)
				pSet->Edit.job.m_Head.srtStartTime.byHour--;
			else
				pSet->Edit.job.m_Head.srtStartTime.byHour= 23;
			MyTimeAdd(&pSet->Edit.endTime,  tjGetStartTime(&pSet->Edit.job), tjGetPlayLength(&pSet->Edit.job));
			dspJobEndTime(&pSet->Edit, 5, pSet->byIndex);
			break;
		case 3:	// start:minute
		case 4:	// start:second
			pBuf= (u8 *)&pSet->Edit.job.m_Head.srtStartTime;
			index= pSet->byIndex-2;
			if(pBuf[index])
				pBuf[index]--;
			else
				pBuf[index]= 59;
			MyTimeAdd(&pSet->Edit.endTime,  tjGetStartTime(&pSet->Edit.job), tjGetPlayLength(&pSet->Edit.job));
			dspJobEndTime(&pSet->Edit, 5, pSet->byIndex);
			break;
		case 5:	// end:hour
			if(pSet->Edit.endTime.byHour)
				pSet->Edit.endTime.byHour--;
			else
				pSet->Edit.endTime.byHour= 23;
			break;
		case 6:	// end:minute
		case 7:	// end:second
			pBuf= (u8 *)&pSet->Edit.endTime;
			index= pSet->byIndex-5;
			if(pBuf[index])
				pBuf[index]--;
			else
				pBuf[index]= 59;
			break;
		default:
			return TRUE;
		}
		break;
	case VK_LEFT:
		if ((pSet->Edit.job.m_Head.byTimeType == tttYk)| (pSet->Edit.job.m_Head.byTimeType == tttPhone)) {
			if(pSet->byIndex == 5)
			{
				pSet->byIndex=1;
				break;
			}
		}
		if(pSet->byIndex)
			pSet->byIndex--;
		else
			pSet->byIndex= 8;
		break;
	case VK_RIGHT:
		if ((pSet->Edit.job.m_Head.byTimeType == tttYk)| (pSet->Edit.job.m_Head.byTimeType == tttPhone)) {
			if(pSet->byIndex == 1)
			{
				pSet->byIndex=5;
				break;
			}
		}
		if(pSet->byIndex<8)
			pSet->byIndex++;
		else
			pSet->byIndex= 0;
		break;
	case VK_ENTER:
		switch(pSet->byIndex)
		{
		case 0:	// source
			switch(pSet->Edit.job.m_Head.bySourceType)
			{
			case stTuner:
				pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_JOBTUNERSET];	// 取得新的菜单
				break;
			case stSongFile:
				pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_JOBSONGSET];	// 取得新的菜单
				break; 
			default:
				return TRUE;
			}		
			if(pNewMenu->InitFunc)
				pNewMenu->InitFunc(&pSet->Edit);
			break;
		case 1:	// port
			pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_PORTEDIT];
			if(pNewMenu->InitFunc)
				pNewMenu->InitFunc(&pSet->Edit.job);
			break;
 		case 2:	// start:hour
		case 3:	// start:minute
		case 4:	// start:second
		case 5:	// end:hour
		case 6:	// end:minute
		case 7:	// end:second
			return TRUE;
		case 8:	// edit/save
			if(pSet->Edit.job.m_Head.byTimeType==tttIntercut)
				pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_INTERCUTLIST];	// 取得新的菜单
#if (GC_ZR & GC_ZR_REMOTE)
			else if (pSet->Edit.job.m_Head.byTimeType == tttYk)
				pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_YKEDIT];	// 取得新的菜单
			else if (pSet->Edit.job.m_Head.byTimeType == tttPhone)
				pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_PHONEEDIT];	// 取得新的菜单
#endif
			else
				// pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_JOBEDIT];	// 取得新的菜单
				break;
			tjSetPlayLength(&pSet->Edit.job, (WORD)(MyTimeToSecond(&pSet->Edit.endTime)-MyTimeToSecond(tjGetStartTime(&pSet->Edit.job))));
			switch(pNewMenu->byMenuID)
			{
			case MENUID_INTERCUTLIST:
				pJob=&((INTERCUTPARAM *)pNewMenu->pParam)->task.m_Job;
				break;
#if (GC_ZR & GC_ZR_REMOTE)
			case MENUID_YKEDIT:
				pJob =&((YKPARAM *)pNewMenu->pParam)->task.m_Job;
				break;
			case MENUID_PHONEEDIT:
				pJob = &((PHONEPARAM *)pNewMenu->pParam)->task.m_Job;
				break;
#endif
			default:
				break;
			}
			if(pJob)
				if(memcmp(tjGetHeadBuffer(pJob), tjGetHeadBuffer(&pSet->Edit.job), tjGetHeadBufferSize())!=0)
				{
					tjAssign(pJob, &pSet->Edit.job);
					g_bTaskChange= TRUE;
				}
			break;
		}
		break;
	case VK_ESC:
		// 放弃数据保存，返回上一级
		if(pSet->Edit.job.m_Head.byTimeType==tttIntercut)
			pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_INTERCUTLIST];	// 取得新的菜单
#if (GC_ZR & GC_ZR_REMOTE)
		else if (pSet->Edit.job.m_Head.byTimeType == tttYk)
			pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_YKEDIT];	// 取得新的菜单
		else if (pSet->Edit.job.m_Head.byTimeType == tttPhone)
			pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_PHONEEDIT];	// 取得新的菜单
#endif
		//else
		//	pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_JOBEDIT];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}
// --------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------
typedef struct	{
	CTaskJobGroup	group;
	u8 byIndex;
	u8 byJobIndex;
}TASKVIEWLISTPARAM;
void InitTaskViewList(void *pItem)
{
	TASKVIEWPARAM	*pView= (TASKVIEWPARAM *)((PMENUITEM)pItem)->pParam;
	TASKVIEWLISTPARAM *pSet= (TASKVIEWLISTPARAM *)((PMENUITEM)&Menu_Tab[MENUID_TASKVIEWLIST])->pParam;

	pSet->byIndex= 0;
	pSet->byJobIndex= 0;
	tjgInit(&pSet->group, TASK_COUNT_OF_GROUP);
	tmGetTodayTaskGroup(&g_taskManager, &pSet->group, &pView->dt);
}

void dspTaskViewList(struct _TAGMENUITEM *pItem)
{
	TASKVIEWLISTPARAM *pSet= (TASKVIEWLISTPARAM *)((PMENUITEM)&Menu_Tab[MENUID_TASKVIEWLIST])->pParam;
	CTaskJobGroup *pJobGroup= &pSet->group;
	u8 nJobCount= tjgGetCount(pJobGroup);
	TMyTime	endTime;
	CTaskJob	*pJob=NULL;
	if((!g_pCurMenuItem)||(MENUID_TASKVIEWLIST!= g_pCurMenuItem->byMenuID) )
	{
		lcm_Clr();
		g_pCurMenuItem= (PMENUITEM)&Menu_Tab[MENUID_TASKVIEWLIST];
		dspDrawTitle();
		dspDrawRect();
		lcm_Putstr(jobtimecolpos_tab[0], 32, sZeroTime, 8, 0);
		lcm_Putstr(jobtimecolpos_tab[3], 32, sZeroTime, 8, 0);
	}
	// 标题	
	strfmt(str_buf, sCountFmt, nJobCount?pSet->byJobIndex+1:0, nJobCount);
	lcm_Putstr(93, 0, str_buf, 9, 0);
	if(nJobCount)
	{
		lcm_ClrLine(2, 3, 189, 2, 0);
		pJob= tjgGetAt(pJobGroup, pSet->byJobIndex);
		// 音源
#if (GC_ZR & GC_ZR_FC)
		lcm_Putstr(30, 16, GetJobSourceString(pJob, str_buf, 16), 16, 0);
#else
		lcm_Putstr(30, 16, GetJobSourceString(pJob, str_buf, 17), 17, 0);
#endif

		// 分区
#if (GC_ZR & GC_ZR_FC)
		strncpy(str_buf, sPort, 4);
		lcm_Putstr(130, 16, str_buf, GetJobPortStateString(pJob, &str_buf[4]) + 4, 0);
#else
		strncpy(str_buf, sPort, 4);
		lcm_Putstr(136, 16, str_buf, GetJobPortStateString(pJob, &str_buf[5]) + 5, 0);
#endif
		
		// 开始时间
		dspTime(jobtimecolpos_tab[0], 32, &pJob->m_Head.srtStartTime);
		dspTime(jobtimecolpos_tab[3], 32, tjGetEndTime(pJob, &endTime));	

		lcm_Putstr(2, 16, sSource, 4, 0);
		lcm_Putstr(2, 32, sStart, 4, 0);
		lcm_Putstr(118, 32, sEnd, 4, 0);
		// 显示功能菜单
		lcm_Putstr(2, 48, sPrev, 6, pSet->byIndex==0);
		lcm_Putstr(154, 48, sNext, 6, pSet->byIndex==1);
	}else
	{
		lcm_ClrLine(2, 2, 189, 4, 0);
		lcm_Putstr(48, 24, sTodayNoJob, 0, 0);
	}
}

u32 keydo_TaskViewList(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	u8 nJobCount;
	PMENUITEM  pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_TASKVIEWLIST];
	TASKVIEWLISTPARAM *pSet= (TASKVIEWLISTPARAM *)((PMENUITEM)&Menu_Tab[MENUID_TASKVIEWLIST])->pParam;
	CTaskJobGroup *pJobGroup= &pSet->group;

	nJobCount= tjgGetCount(pJobGroup);
	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
	case VK_UP:
	case VK_DOWN:
		return	TRUE;
	case VK_LEFT:
	case VK_RIGHT:
		if(pSet->byIndex)
			pSet->byIndex= 0;
		else
			pSet->byIndex= 1;
		break;
	case VK_ENTER:
		switch(pSet->byIndex)
		{
		case 0:	// prev
			if(nJobCount==1)
				return TRUE;
			if(pSet->byJobIndex)
				pSet->byJobIndex--;
			else
				pSet->byJobIndex= nJobCount-1;
			break;
		case 1:// next
			if(nJobCount==1)
				return TRUE;
			if(pSet->byJobIndex<nJobCount-1)
				pSet->byJobIndex++;
			else
				pSet->byJobIndex= 0;
			break;
		}
		break;
	case VK_ESC:
		pNewMenu= (PMENUITEM)&Menu_Tab[g_pCurMenuItem->byParentMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}
// --------------------------------------------------------------------------------------------------------------
typedef struct {
	// PMENUITEM pPrevMenu;
	CTaskJob *pJob;
	u8 byIndex;
#if (GC_ZR & GC_ZR_FC)
	u8 byInited;
	u16 byPort;
#else
	u8 byPort;
#endif
}PORTEDITPARAM;

void InitPortEditParam(void *pItem /* CTaskJob * */)
{
	PORTEDITPARAM *pSet=(PORTEDITPARAM *)((PMENUITEM)&Menu_Tab[MENUID_PORTEDIT])->pParam;
	pSet->pJob = (CTaskJob *)pItem;
#if (GC_ZR & GC_ZR_FC)
	pSet->byInited = 0;
#endif
	pSet->byIndex= 0;
	pSet->byPort= pSet->pJob->mPortState;
}
#if (GC_ZR & GC_ZR_FC)
void dspPortEdit(struct _TAGMENUITEM *pItem)
{
	
	static	const u8 portPos_tab0[6][2] = { { 3, 16 }, { 73, 16 }, { 142, 16 }, { 3, 32 }, { 73, 32 }, { 142, 32 } };	
	static	const u8 portPos_tab1[6][2] = { { 39, 16 }, { 109, 16 }, { 178, 16 }, { 39, 32 }, { 109, 32 }, { 178, 32 } };

	u8 i, j;
	PORTEDITPARAM* pSet = (PORTEDITPARAM*)((PMENUITEM)&Menu_Tab[MENUID_PORTEDIT])->pParam;
	if ((!g_pCurMenuItem) || (MENUID_PORTEDIT != g_pCurMenuItem->byMenuID))
	{
		pSet->byInited = 0;
		g_pCurMenuItem = (PMENUITEM)&Menu_Tab[MENUID_PORTEDIT];
	}
	if (pSet->byInited == 0) {
		pSet->byInited = 1;
		lcm_Clr();
		dspDrawTitle();
		dspDrawRect();
	}
	if (pSet->byIndex < 6) {
		for (i = 0; i < 6; i++)
		{
			j = i + 0;
			lcm_Putstr(portPos_tab0[i][0], portPos_tab0[i][1], sFenqu[j], 6, 0);
			lcm_Putstr(portPos_tab1[i][0], portPos_tab1[i][1], (1 << j)&pSet->byPort ? sSelected : sUnselected, 2, pSet->byIndex == j);
		}
	}
	else if ((pSet->byIndex >= 6) && (pSet->byIndex < 12)) {
		for (i = 0; i < 6; i++)
		{
			j = 6 + i;
			lcm_Putstr(portPos_tab0[i][0], portPos_tab0[i][1], sFenqu[j], 6, 0);
			lcm_Putstr(portPos_tab1[i][0], portPos_tab1[i][1], (1 << j)&pSet->byPort ? sSelected : sUnselected, 2, pSet->byIndex == j);
		}
	}
	else if ((pSet->byIndex >= 12) && (pSet->byIndex <= 16)) {
		for (i = 0; i < 4; i++)
		{
			j = 12 + i;
			lcm_Putstr(portPos_tab0[i][0], portPos_tab0[i][1], sFenqu[j], 6, 0);
			lcm_Putstr(portPos_tab1[i][0], portPos_tab1[i][1], (1 << j)&pSet->byPort ? sSelected : sUnselected, 2, pSet->byIndex == j);
		}
	}
	lcm_Putstr(84, 48, sSave, 4, pSet->byIndex == 16);
}
#else
void dspPortEdit(struct _TAGMENUITEM *pItem)
{
	static	const u8 portXPos_tab[] = { 39, 178, 39, 178 };
	static	const u8 portYPos_tab[] = { 16, 16, 32, 32 };

	u8 i;
	PORTEDITPARAM *pSet = (PORTEDITPARAM *)((PMENUITEM)&Menu_Tab[MENUID_PORTEDIT])->pParam;
	if ((!g_pCurMenuItem) || (MENUID_PORTEDIT != g_pCurMenuItem->byMenuID))
	{
		lcm_Clr();
		g_pCurMenuItem = (PMENUITEM)&Menu_Tab[MENUID_PORTEDIT];
		dspDrawTitle();
		dspDrawRect();
		lcm_Putstr(3, 16, "分区1", 5, 0);
		lcm_Putstr(142, 16, "分区2", 5, 0);
		lcm_Putstr(3, 32, "分区3", 5, 0);
		lcm_Putstr(142, 32, "分区4", 5, 0);
	}
	for (i = 0; i<4; i++)
	{
		lcm_Putstr(portXPos_tab[i], portYPos_tab[i], (1 << i)&pSet->byPort ? sSelected : sUnselected, 2, pSet->byIndex == i);
	}
	lcm_Putstr(84, 48, sSave, 4, pSet->byIndex == 4);
}
#endif
u32 keydo_PortEdit(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
#if (GC_ZR & GC_ZR_FC)
	u16 n;
#else
	u8 n;
#endif
	PMENUITEM  pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_PORTEDIT];
	PORTEDITPARAM *pSet= (PORTEDITPARAM *)pNewMenu->pParam;

	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
	case VK_UP:
	case VK_DOWN:
		return	TRUE;
#if (GC_ZR & GC_ZR_FC)
	case VK_LEFT:
		if(pSet->byIndex>0)
			pSet->byIndex--;
		else
			pSet->byIndex= 16;

		switch (pSet->byIndex)
		{
		case 5:
		case 11:
		case 16:
			pSet->byInited = 0;
			break;
		default:
			break;
		}

		break;
	case VK_RIGHT:
		if(pSet->byIndex<16)
			pSet->byIndex++;
		else
			pSet->byIndex= 0;

		switch (pSet->byIndex)
		{
		case 0:
		case 6:
		case 12:
			pSet->byInited = 0;
			break;
		default:
			break;
		}

		break;
	case VK_ENTER:
		if(pSet->byIndex==16)	// 保存
		{
			pSet->pJob->mPortState= pSet->byPort;
		}else
		{
			n= 1<<pSet->byIndex;
			if(pSet->byPort &n)
				pSet->byPort &= ~n;
			else
				pSet->byPort |= n;
			if(pSet->byIndex<16)	// 按确认键自动下一个
				pSet->byIndex++;
			else
				pSet->byIndex = 0;

			switch (pSet->byIndex)
			{
			case 0:
			case 6:
			case 12:
				pSet->byInited = 0;
				break;
			default:
				break;
			}

			break;
		}
#else
	case VK_LEFT:
		if (pSet->byIndex>0)
			pSet->byIndex--;
		else
			pSet->byIndex = 4;
		break;
	case VK_RIGHT:
		if (pSet->byIndex<4)
			pSet->byIndex++;
		else
			pSet->byIndex = 0;
		break;
	case VK_ENTER:
		if (pSet->byIndex == 4)	// 保存
		{
			pSet->pJob->mPortState = pSet->byPort;
		}
		else
		{
			n = 1 << pSet->byIndex;
			if (pSet->byPort &n)
				pSet->byPort &= ~n;
			else
				pSet->byPort |= n;
			if (pSet->byIndex<4)	// 按确认键自动下一个
				pSet->byIndex++;
			else
				pSet->byIndex = 0;
			break;
		}
#endif
	case VK_ESC:
		if(pSet->pJob->m_Head.byTimeType==tttIntercut)
			pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_SPECIALJOBEDIT];	// 取得新的菜单
#if (GC_ZR & GC_ZR_REMOTE)
		else if (pSet->pJob->m_Head.byTimeType == tttYk)
			pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_SPECIALJOBEDIT];	// 取得新的菜单
		else if (pSet->pJob->m_Head.byTimeType == tttPhone)
			pNewMenu = (PMENUITEM)&Menu_Tab[MENUID_SPECIALJOBEDIT];	// 取得新的菜单
#endif
		else
			pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_JOBEDIT];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}
// --------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------
typedef struct {
	u8 byInited;
	u8 byIndex;
	u8 byReturnMenuID;
	u8 byRes;
}PORTCTRLPARAM;

void InitPortCtrlParam(void *pItem /* PMENUITEM */)
{
	PORTCTRLPARAM *pSet=(PORTCTRLPARAM *)((PMENUITEM)&Menu_Tab[MENUID_PORTCTRL])->pParam;
	pSet->byInited = 0;
	pSet->byIndex = 0;
	pSet->byReturnMenuID= ((PMENUITEM)pItem)->byMenuID;
}

#if (GC_ZR & GC_ZR_FC)
void dspPortCtrl(struct _TAGMENUITEM *pItem)
{
	static	const u8 portPos_tab0[6][2] = { { 3, 24 }, { 73, 24 }, { 142, 24 }, { 3, 40 }, { 73, 40 }, { 142, 40 } };
	static	const u8 portPos_tab1[6][2] = { { 39, 24 }, { 109, 24 }, { 178, 24 }, { 39, 40 }, { 109, 40 }, { 178, 40 } };

	u8 i,j;
	PORTCTRLPARAM* pSet = (PORTCTRLPARAM*)((PMENUITEM)&Menu_Tab[MENUID_PORTCTRL])->pParam;
	if ((!g_pCurMenuItem) || (MENUID_PORTCTRL != g_pCurMenuItem->byMenuID))
	{
		pSet->byInited = 0;
		pSet->byReturnMenuID = g_pCurMenuItem->byMenuID;
		g_pCurMenuItem = (PMENUITEM)&Menu_Tab[MENUID_PORTCTRL];
	}
	if (pSet->byInited == 0) {
		pSet->byInited = 1;
		lcm_Clr();
		dspDrawTitle();
		dspDrawRect();
	}
	if (pSet->byIndex < 6) {
		for (i = 0; i < 6; i++)
		{
			j = i + 0;
			lcm_Putstr(portPos_tab0[i][0], portPos_tab0[i][1], sFenqu[j], 6, 0);
			lcm_Putstr(portPos_tab1[i][0], portPos_tab1[i][1], audio_GetPortState(j) ? sSelected : sUnselected , 2, pSet->byIndex == j);
		}
	}
	else if ((pSet->byIndex >= 6) && (pSet->byIndex < 12)) {
		for (i = 0; i < 6; i++)
		{
			j = 6 + i;
			lcm_Putstr(portPos_tab0[i][0], portPos_tab0[i][1], sFenqu[j], 6, 0);
			lcm_Putstr(portPos_tab1[i][0], portPos_tab1[i][1], audio_GetPortState(j) ? sSelected : sUnselected, 2, pSet->byIndex == j);
		}
	}
	else if ((pSet->byIndex >= 12) && (pSet->byIndex < 16)) {
		for (i = 0; i < 4; i++)
		{
			j = 12 + i;
			lcm_Putstr(portPos_tab0[i][0], portPos_tab0[i][1], sFenqu[j], 6, 0);
			lcm_Putstr(portPos_tab1[i][0], portPos_tab1[i][1], audio_GetPortState(j) ? sSelected : sUnselected, 2, pSet->byIndex == j);
		}
	}
}
#else
void dspPortCtrl(struct _TAGMENUITEM *pItem)
{
	static	const u8 portXPos_tab1[] = { 39, 178, 39, 178 };
	static	const u8 portYPos_tab1[] = { 24, 24, 40, 40 };

	u8 i;
	PORTCTRLPARAM* pSet = (PORTCTRLPARAM*)((PMENUITEM)&Menu_Tab[MENUID_PORTCTRL])->pParam;
	if ((!g_pCurMenuItem) || (MENUID_PORTCTRL != g_pCurMenuItem->byMenuID))
	{
		lcm_Clr();
		pSet->byReturnMenuID = g_pCurMenuItem->byMenuID;
		if (pSet->byInited == 0)
		{
			pSet->byIndex = 0;
		}
		g_pCurMenuItem = (PMENUITEM)&Menu_Tab[MENUID_PORTCTRL];
		dspDrawTitle();
		dspDrawRect();
		lcm_Putstr(3, 24, "分区1", 5, 0);
		lcm_Putstr(142, 24, "分区2", 5, 0);
		lcm_Putstr(3, 40, "分区3", 5, 0);
		lcm_Putstr(142, 40, "分区4", 5, 0);
	}
	for (i = 0; i<4; i++)
	{
		lcm_Putstr(portXPos_tab1[i], portYPos_tab1[i], audio_GetPortState(i) ? sSelected : sUnselected, 2, pSet->byIndex == i);
	}
}
#endif

u32 keydo_PortCtrl(struct _TAGMENUITEM * pItem, unsigned char KeyCode, unsigned char keyMask)
{
	PMENUITEM  pNewMenu= (PMENUITEM)&Menu_Tab[MENUID_PORTCTRL];
	PORTCTRLPARAM* pSet= (PORTCTRLPARAM*)pNewMenu->pParam;

	switch(KeyCode)
	{
	case VK_NUM0:
	case VK_NUM1:
	case VK_NUM2:
	case VK_NUM3:
	case VK_NUM4:
	case VK_NUM5:
	case VK_NUM6:
	case VK_NUM7:
	case VK_NUM8:
	case VK_NUM9:
	case VK_UP:
	case VK_DOWN:
		return	TRUE;
#if (GC_ZR & GC_ZR_FC)
	case VK_LEFT:
		if(pSet->byIndex>0)
			pSet->byIndex--;
		else
			pSet->byIndex= 15;
		switch (pSet->byIndex)
		{
		case 5:
		case 11:
		case 15:
			pSet->byInited = 0;
			break;
		default:
			break;
		}
		break;
	case VK_RIGHT:
		if(pSet->byIndex<15)
			pSet->byIndex++;
		else
			pSet->byIndex= 0;
		switch (pSet->byIndex)
		{
		case 0:
		case 6:
		case 12:
			pSet->byInited = 0;
			break;
		default:
			break;
		}
		break;
	case VK_ENTER:
		if (audio_GetPortState(pSet->byIndex))
			audio_SetPortState(pSet->byIndex, FALSE);
		else
			audio_SetPortState(pSet->byIndex, TRUE);
		if (pSet->byIndex<15)	// 按确认键自动下一个
			pSet->byIndex++;
		else
			pSet->byIndex = 0;

		switch (pSet->byIndex)
		{
		case 0:
		case 6:
		case 12:
			pSet->byInited = 0;
			break;
		default:
			break;
		}
		break;
#else
	case VK_LEFT:
		if(pSet->byIndex>0)
			pSet->byIndex--;
		else
			pSet->byIndex= 3;
		break;
	case VK_RIGHT:
		if(pSet->byIndex<3)
			pSet->byIndex++;
		else
			pSet->byIndex = 0;
		break;
	case VK_ENTER:
		if (audio_GetPortState(pSet->byIndex))
			audio_SetPortState(pSet->byIndex, FALSE);
		else
			audio_SetPortState(pSet->byIndex, TRUE);
		if (pSet->byIndex<3)	// 按确认键自动下一个
			pSet->byIndex++;
		else
			pSet->byIndex = 0;
		break;
#endif
	case VK_ESC:
		pNewMenu= (PMENUITEM)&Menu_Tab[pSet->byReturnMenuID];	// 取得新的菜单
		break;
	default:
		return FALSE;
	}
	pNewMenu->DisplayUIFunc(pItem);
	return TRUE;
}
// --------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------
PMENUITEM g_pCurMenuItem= NULL;// Menu_Tab;

static PLAYTUNERPARAM mPlayTunerParam;
static PLAYMEMPARAM mPlayMemParam;
static PLAYAUXPARAM mPlayAuxParam;
static PLAYUSBPARAM mPlayUsbParam;
static PASSWORDINPUTPARAM	mPwdInput;
static MAINMENUPARAM mMainMenuParam;
static TASKMANAGERMENUPARAM mTaskManagerParam;
static SONGVIEWMENUPARAM mSongViewMenuParam;
static SYSTEMSETPARAM mSystemSetParam;
static SPECIALMODESETPARAM mSpecialMode;
static TIMESETPARAM	mTimeSet;
static PASSWORDSETPARAM mPassWordSetParam;
#if(GC_ZR & GC_ZR_BJ)
static BJSETPARAM mBjSetParam;
#endif
static SONGVIEWPARAM	mSongView;
static JOBEDITPARAM	mJobEdit;
static JOBSONGSETPARAM mJobSongSet;
static JOBTUNERSETPARAM	mJobTunerSet;
static GROUPLISTPARAM	mGroupList;
static INTERCUTPARAM		mIntercutTask;
#if (GC_ZR & GC_ZR_REMOTE)
static YKPARAM mYkTask;
static PHONEPARAM mPhoneTask;
#endif
static SPECIALJOBEDITPARAM	mSpecialJob;
static TASKVIEWPARAM	mTaskView;
static TASKVIEWLISTPARAM	mTaskViewList;
static PORTEDITPARAM mPortEditParam;
static PORTCTRLPARAM mPortCtrlParam;
const MENUITEM Menu_Tab[]={
	{	MENUID_DESKTOP,									// byMenuID
		MENUID_INVALID,										// byParentMenuID
		0, 0, NULL,											// Title
		NULL,												// pParam
		keydo_Desktop,										// KeyDoFunc
		NULL,												// InitFunc;
		dspDesktop											// DisplayFunc
	},
	{	MENUID_PLAYTUNER,									// byMenuID
		MENUID_DESKTOP,									// byParentMenuID
		72, 8, sManualPlay,									// Title
		&mPlayTunerParam,									// pParam
		keydo_PlayTuner,									// KeyDoFunc
		InitPlayeTuner,												// InitFunc;
		dspPlayTuner										// DisplayFunc
	},
	{	MENUID_PLAYMEM,										// byMenuID
		MENUID_DESKTOP,									// byParentMenuID
		72, 8, sManualPlay,									// Title
		&mPlayMemParam,									// pParam
		keydo_PlayMem,										// KeyDoFunc
		InitPlayMem,												// InitFunc;
		dspPlayMem											// DisplayFunc
	},
	{	MENUID_PLAYAUX,										// byMenuID
		MENUID_DESKTOP,									// byParentMenuID
		72, 8, sManualPlay,									// Title
		&mPlayAuxParam,												// pParam
		keydo_PlayAUX,										// KeyDoFunc
		InitPlayAUX,												// InitFunc;
		dspPlayAUX											// DisplayFunc
	},
	{	MENUID_PLAYUSB,										// byMenuID
		MENUID_DESKTOP,									// byParentMenuID
		72, 8, sManualPlay,									// Title
		&mPlayUsbParam,									// pParam
		keydo_PlayUSB,										// KeyDoFunc
		InitPlayUSB,												// InitFunc;
		dspPlayUSB											// DisplayFunc
	},
	{	MENUID_PLAYPHONE,										// byMenuID
		MENUID_DESKTOP,									// byParentMenuID
		72, 8, sManualPlay,									// Title
		NULL,									// pParam
		keydo_PlayPhone,										// KeyDoFunc
		NULL,												// InitFunc;
		dspPlayPhone											// DisplayFunc
	},
#if (GC_ZR & GC_ZR_PLAY)
	{ 
		MENUID_PLAYYJ,										// byMenuID
		MENUID_DESKTOP,									// byParentMenuID
		72, 8, "紧急播音",									// Title BJ
		NULL,									// pParam
		keydo_PlayYj,										// KeyDoFunc
		NULL,												// InitFunc;
		dspPlayYj											// DisplayFunc
	},
	{
		MENUID_PLAYBJ,										// byMenuID
		MENUID_DESKTOP,									// byParentMenuID
		72, 8, "报警播音",									// Title BJ
		NULL,									// pParam
		keydo_PlayBj,										// KeyDoFunc
		NULL,												// InitFunc;
		dspPlayBj											// DisplayFunc
	},
#endif
	{	MENUID_PASSWORDINPUT,								// byMenuID
		MENUID_DESKTOP,									// byParentMenuID
		54, 14, sModal,										// Title
		&mPwdInput,											// pParam
		keydo_PasswordInput,									// KeyDoFunc
		NULL,												// InitFunc;
		dspPasswordInput										// DisplayFunc
	},
	{	MENUID_MAINMENU,										// byMenuID
		MENUID_DESKTOP,										// byParentMenuID
		54, 14, sModal,										// Title
		&mMainMenuParam,									// pParam
		keydo_MainMenu,									// KeyDoFunc
		InitMainMenu,												// InitFunc;
		dspMainMenu										// DisplayFunc
	},
	{	MENUID_TASKMANAGER,								// byMenuID
		MENUID_MAINMENU,									// byParentMenuID
		72, 8, sMenuTitles[0],								// Title
		&mTaskManagerParam,									// pParam
		keydo_TaskManagerMenu,							// KeyDoFunc
		InitTaskMannagerMenu,												// InitFunc;
		dspTaskManagerMenu										// DisplayFunc
	},
	{	MENUID_SONGVIEWMENU,								// byMenuID
		MENUID_MAINMENU,									// byParentMenuID
		72, 8, sMenuTitles[1],								// Title
		&mSongViewMenuParam,									// pParam
		keydo_SongViewMenu,							// KeyDoFunc
		InitSongViewMenu,												// InitFunc;
		dspSongViewMenu										// DisplayFunc
	},
	{	MENUID_SYSTEMSET,								// byMenuID
		MENUID_MAINMENU,									// byParentMenuID
		72, 8, sMenuTitles[2],								// Title
		&mSystemSetParam,									// pParam
		keydo_SystemSet,							// KeyDoFunc
		InitSystemSet,												// InitFunc;
		dspSystemSet										// DisplayFunc
	},
	{	MENUID_SPECIALMODE,								// byMenuID
		MENUID_MAINMENU,									// byParentMenuID
		72, 8, sMenuTitles[3],								// Title
		&mSpecialMode,									// pParam
		keydo_SpecialMode,								// KeyDoFunc
		NULL,												// InitFunc;
		dspSpecialMode									// DisplayFunc
	},
	{	MENUID_TASKVIEW,								// byMenuID
		MENUID_TASKMANAGER,									// byParentMenuID
		72, 8, sTaskMenuTitles[0],								// Title
		&mTaskView,									// pParam
		keydo_TaskView,								// KeyDoFunc
		InitTaskView,												// InitFunc;
		dspTaskView									// DisplayFunc
	},
	{	MENUID_JOBEDIT,								// byMenuID
		MENUID_TASKMANAGER,									// byParentMenuID
		72, 8, sTaskMenuTitles[1],								// Title
		&mJobEdit,									// pParam
		keydo_JobEdit,								// KeyDoFunc
		InitJobEdit,												// InitFunc;
		dspJobEdit									// DisplayFunc
	},
	{	MENUID_GROUPLIST,								// byMenuID
		MENUID_TASKMANAGER,									// byParentMenuID
		18, 8, sTaskMenuTitles[2],								// Title
		&mGroupList,									// pParam
		keydo_GroupList,								// KeyDoFunc
		InitGroupList,												// InitFunc;
		dspGroupList									// DisplayFunc
	},
	{	MENUID_INTERCUTLIST,								// byMenuID
		MENUID_TASKMANAGER,									// byParentMenuID
		45, 8, sTaskMenuTitles[3],								// Title
		&mIntercutTask,									// pParam
		keydo_IntercutTask,								// KeyDoFunc
		InitIntercutTask,												// InitFunc;
		dspIntercutTask									// DisplayFunc
	},
#if (GC_ZR & GC_ZR_REMOTE)
	{
		MENUID_YKEDIT,								// byMenuID
		MENUID_TASKMANAGER,									// byParentMenuID
		45, 8, sTaskMenuTitles[4],								// Title
		&mYkTask,									// pParam
		keydo_YkTask,								// KeyDoFunc
		InitYkTask,												// InitFunc;
		dspYkTask									// DisplayFunc
	},
	{
		MENUID_PHONEEDIT,								// byMenuID
		MENUID_TASKMANAGER,									// byParentMenuID
		45, 8, sTaskMenuTitles[5],								// Title
		&mPhoneTask,									// pParam
		keydo_PhoneTask,								// KeyDoFunc
		InitPhoneTask,												// InitFunc;
		dspPhoneTask									// DisplayFunc
	},
#endif
	{	MENUID_SONGVIEWMEM,								// byMenuID
		MENUID_SONGVIEWMENU,								// byParentMenuID
		72, 8, sSourceName[1],								// Title
		&mSongView,									// pParam
		keydo_SongViewMem,								// KeyDoFunc
		NULL,												// InitFunc;
		dspSongViewMem									// DisplayFunc
	},
	{	MENUID_SONGVIEWUSB,								// byMenuID
		MENUID_SONGVIEWMENU,									// byParentMenuID
		72, 8, sSourceName[3],								// Title
		&mSongView,									// pParam
		keydo_SongViewUSB,								// KeyDoFunc
		NULL,												// InitFunc;
		dspSongViewUSB									// DisplayFunc
	},
	{	MENUID_TIMESET,								// byMenuID
		MENUID_SYSTEMSET,								// byParentMenuID
		72, 8, sSystemSets[0],								// Title
		&mTimeSet,									// pParam
		keydo_TimeSet,								// KeyDoFunc
		NULL,												// InitFunc;
		dspTimeSet									// DisplayFunc
	},
	{	MENUID_PASSWORDSET,								// byMenuID
		MENUID_SYSTEMSET,									// byParentMenuID
		72, 8, sSystemSets[1],								// Title
		&mPassWordSetParam,									// pParam
		keydo_PasswordSet,								// KeyDoFunc
		NULL,												// InitFunc;
		dspPasswordSet									// DisplayFunc
	},
#if (GC_ZR & GC_ZR_BJ)
	{ 
		MENUID_BJSET,								// byMenuID
		MENUID_SYSTEMSET,									// byParentMenuID
		72, 8, sSystemSets[2],								// Title
		&mBjSetParam,									// pParam
		keydo_BjSet,								// KeyDoFunc
		InitBjSet,												// InitFunc;
		dspBjSet									// DisplayFunc
	},
#endif
	{	MENUID_JOBSONGSET,								// byMenuID
		MENUID_JOBEDIT,									// byParentMenuID
		72, 8, "乐曲选择",								// Title
		&mJobSongSet,									// pParam
		keydo_JobSongSet,								// KeyDoFunc
		InitJobSongSet,												// InitFunc;
		dspJobSongSet									// DisplayFunc
	},
	{	MENUID_JOBTUNERSET,								// byMenuID
		MENUID_JOBEDIT,									// byParentMenuID
		72, 8, "收音设置",								// Title
		&mJobTunerSet,									// pParam
		keydo_JobTunerSet,								// KeyDoFunc
		InitJobTunerSet,												// InitFunc;
		dspJobTunerSet									// DisplayFunc
	},
	{	MENUID_SPECIALJOBEDIT,								// byMenuID
		MENUID_INTERCUTLIST,								// byParentMenuID
		72, 8, sTaskMenuTitles[3],							// Title
		&mSpecialJob,									// pParam
		keydo_SpecialJobEdit,								// KeyDoFunc
		InitSpecialJobEdit,												// InitFunc;
		dspSpecialJobEdit									// DisplayFunc
	},
	{	MENUID_TASKVIEWLIST,								// byMenuID
		MENUID_TASKVIEW,									// byParentMenuID
		45, 8, sTaskMenuTitles[0],								// Title
		&mTaskViewList,									// pParam
		keydo_TaskViewList,								// KeyDoFunc
		InitTaskViewList,												// InitFunc;
		dspTaskViewList									// DisplayFunc
	},
	{	MENUID_PORTEDIT,								// byMenuID
		MENUID_TASKVIEW,								// byParentMenuID
		72, 8, sPort,								// Title
		&mPortEditParam,									// pParam
		keydo_PortEdit,								// KeyDoFunc
		InitPortEditParam,												// InitFunc;
		dspPortEdit									// DisplayFunc
	},
	{	MENUID_PORTCTRL,								// byMenuID
		MENUID_MAINMENU,								// byParentMenuID
		72, 8, sPort,								// Title
		&mPortCtrlParam,									// pParam
		keydo_PortCtrl,								// KeyDoFunc
		InitPortCtrlParam,												// InitFunc;
		dspPortCtrl									// DisplayFunc
	},
	
	
};

