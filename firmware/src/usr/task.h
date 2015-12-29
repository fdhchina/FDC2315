#ifndef _TASK_H
#define _TASK_H

#include "arrayList.h"

#define PLAY_LENGTH_MAX				36000	//单次任务播放长度最长不超过10小时=36000秒

#define TASK_COUNT_OF_GROUP			MYLIST_MAX_SIZE		//每个任务组的最大任务数
#define TASKGROUP_COUNT_OFLIST		0x08		//任务组总数
#define TASK_VALID					0x80
#define TASKCOUNT_MASK				0x7F	


#define TASK_HEAD_STR				0x43475348UL	// "HSGC" 小端模式
#define TASK_VERSION				0x02010000UL	// 2.1.0.0

#define TASK_GRANULARITY			3				// 任务间间隔时间 秒

#define PORTCOUNT_MAX				4				// 最多支持160个分区
#define S32 signed int
#define S16 signed short
#define S8 signed char
#define U32 unsigned int
#define U16 unsigned short
#define U8 unsigned char

typedef enum{
	stInvalid= 0,
	stTuner,
	stSongFile,
	stAUX,
	stUSB,
	stMicphone
}TSourceType, *PSourceType;

// 收音控制
/*
typedef enum	{
	tbFM,
	tbAM, 
	tbSW
} TTunerBand;
*/

// TTunerState C语言版本
typedef struct {
	BYTE		byBand;      // 波段
	BYTE			by2;		// 未用数据
	WORD 	wData;       // 频率数值
}TTunerState,	*PTunerState;

// *****************************************************************************
// 播放控制
// 曲目类型
typedef enum	{
	pstInvalid,
	pstSongFile,        	// 乐曲文件
	pstSongFolder,    	// 乐曲目录
	pstRecordFile      	// 录音文件
}TPlaySongType;

// 播放模式  低半字节用于保存曲目类型
#define	pmRandom				1		// 随机模式
#define	pmRepeat				2		// 重复播放

// 乐曲播放模式
typedef struct { 
	BYTE	byRandom:1;					// 随机播放否
	BYTE	byReapeat:1;				// 重复播放
	BYTE	byVolume:6;					// 音量0-31
}TSongMode, *PSongMode;

typedef struct	{
	TPlaySongType	bySongType;
	TSongMode		byMode;
	WORD			wSongID;
}TSongState, *PSongState;

typedef struct{
	BYTE	by1;
	BYTE	by2;
	BYTE	by3;
	BYTE	by4;
}TOtherSourceState, *POtherSourceState;

/*
#define	IsRandomMode(x)		(x&pmRandom)		// 是随机模式否
#define	ISRepeatMode(x)			(x&pmRepeat)		// 是重复模式否
#define	SetRandomMode(x)		(x|=pmRandom)		// 设为随机模式
#define	ClrRandomMode(x)		(x&=~pmRandom)		// 取消随机模式
#define	SetRepeatMode(x)		(x|=pmRepeat)		// 设为随机模式
#define	ClrRepeatMode(x)		(x&=~pmRepeat)		// 取消随机模式
*/

typedef union	{
	TTunerState			srtTuner;
	TSongState			srtSong;
	TOtherSourceState	srtOther;
}TSourceState, *PSourceState;

// 任务结构
typedef struct	{
	BYTE	byHour;
	BYTE	byMinute;
	BYTE	bySecond;
}TMyTime, *PMyTime;

typedef struct	{
	WORD	wYear;
	BYTE	byMonth;
	BYTE	byDay;
}TMyDate, *PMyDate;

typedef struct  
{
	BYTE	byMonth;
	BYTE	byDay;
}TShortDate, *PShortDate;
// *************************************************************************************************
// 任务工作类型
typedef enum	{
	tjmInvalid=0,					// 无效类型
	tjmPlay,						// 播音
	tjmRecord						// 录音
}TTaskJobMode;
// *************************************************************************************************
// 任务类型
typedef enum	{
	tttNormal,						// 常规
	tttIntercut,					// 插播
#if (GC_ZR & GC_ZR_REMOTE)
	tttYk,						// 特殊任务，报警、遥控。。。
	tttPhone,
#endif
#if(GC_ZR & GC_ZR_BJ)
	tttBj
#endif
}TTaskTimeType;

typedef struct 
{
	TTaskTimeType	byType:7;		
	BYTE			bValided:1;
}TTaskType;

typedef struct	{
	TTaskTimeType	byTimeType;						// 任务有效否														1
	TMyTime			srtStartTime;			   		// 任务开始时间   									 				3
	TTaskJobMode	byJobMode;					// 任务类型															1
	TSourceType		bySourceType;					// 音源类型															1
	WORD			wPlayLength;			   		// 播音时长															2
	TSourceState	srtSourceState;					// 音源状态值													 	4
}_TaskHead;

// =========================================================================================

// ------------------------------------------------------------------------------------------------------------------
typedef struct _TaskJob
{
	//void	*m_lpManager;	// CTaskManager				// 任务管理者
	//void	*m_lpOwner;	// CTaskJobGroup				// 所属任务事件数
	_TaskHead		m_Head;
#if (GC_ZR & GC_ZR_FC)
	WORD	mPortState;
#else
	BYTE	mPortState;											// 分区状态每一位代表1个分区
#endif
}CTaskJob;
// ------------------------------------------------------------------------------------------------------------------
typedef struct _TaskJobGroup
{
	u8			m_bValid;										// 事件组有效否
	u8				m_nMaxCount;									// 事件数
	u8				m_nIndex;										// 组编?1~8?
	CTaskJob			m_Jobs[TASK_COUNT_OF_GROUP];									// 播音事件数组
	ArrayList			m_TaskJobs;									// 事件列表
}CTaskJobGroup;
// ------------------------------------------------------------------------------------------------------------------
typedef struct _TaskJobGroupList
{
	int				m_nMaxCount;
	CTaskJobGroup	JobGroups[TASKGROUP_COUNT_OFLIST];
	ArrayList			m_Groups;
}CTaskJobGroupList;
// ------------------------------------------------------------------------------------------------------------------
typedef struct  
{
	TShortDate	srtStartDate;				// 开始日期
	TShortDate	srtEndDate;					// 结束日期
	BYTE		byTaskGroupOfWeek[7];		// 星期 1--日每天的任务组编号（1--32），0--为无任务
	BYTE		bySpecialGroup;				// 特殊任务组编号，用于晴雨模式
}_TaskGroupTag;

typedef enum	{
	wdSunday,
	wdMonday,
	wdTuesday,
	wdWednesday,
	wdThursday,
	wdFriday,
	wdSaturday
}WEEKDAY;

extern  const WEEKDAY nDisplayWeekday_tab[7];

typedef struct _TaskGroup
{
	_TaskGroupTag	m_Head;
}CTaskGroup;
	
// ------------------------------------------------------------------------------------------------------------------
typedef struct _TaskGroupList
{
	int					m_nMaxCount;						// 最大任务组数
	CTaskGroup			Groups[TASKGROUP_COUNT_OFLIST];
	ArrayList				m_Groups;							// 任务组表
}CTaskGroupList;

// ------------------------------------------------------------------------------------------------------------------
typedef struct _IntercutTask
{
	TMyDate	m_Date;
	CTaskJob m_Job;
}CIntercutTask;
// ------------------------------------------------------------------------------------------------------------------
typedef struct _IntercutTaskGroup
{
	int					m_nMaxCount;
	CIntercutTask			its[TASK_COUNT_OF_GROUP];
	ArrayList				m_Tasks;
}CIntercutTaskGroup;
// ------------------------------------------------------------------------------------------------------------------
#if (GC_ZR & GC_ZR_REMOTE)
typedef struct _RemoteTask
{
	int m_Key;
	int m_funcValue;
	CTaskJob m_Job;
}CRemoteTask;
// ------------------------------------------------------------------------------------------------------------------
typedef struct _RemoteTaskGroup
{
	int					m_nMaxCount;
	CRemoteTask					its[TASK_COUNT_OF_GROUP];
	ArrayList				m_Tasks;
}CRemoteTaskGroup;
// ------------------------------------------------------------------------------------------------------------------
#endif
// 晴雨模式 类型
#define SPECIALMODE_INVALID		0		// 无效
#define SPECIALMODE_CURRENT		1		// 当天有效
#define SPECIALMODE_TWOLY		2		// 第二天有效
#define SPECIALMODE_THIRDLY		4		// 第三天有效

typedef struct _SpecialMode
{
	TMyDate			m_SpecialDate;
	BYTE	m_bySpecMode;				// 特殊任务模式
	BYTE	m_bySpecDayCount;			// 特殊任务执行天数
}CSpecialMode;
// ------------------------------------------------------------------------------------------------------------------
typedef enum {
	tsTaskInvalid,				// 任务未加载
	tsTaskDone,					// 任务结束
	tsTaskReady,				// 下一条任务准备
	tsTaskStart,				// 下一条任务功放已打开
	tsTaskPauseed,				// 任务暂停中
	tsTaskRunning				// 任务执行中
}TTaskState;

typedef struct _TodayTask
{
	//int				m_nCurSecond;			// 任务系统时间--换算为秒计数
	//int				m_nHour;				// 当前小时
	//int				m_nMinute;				// 当前分
	//int				m_nSecond;				// 当前秒
	CTaskJobGroup	m_lpTodayTask;			// 今日任务数据
	TTaskState		m_JobStates[TASK_COUNT_OF_GROUP];
	S32 volatile	m_iNextTask;			// 即将播放或当前正在播放的任务编号
	int	m_nAmpPwrPreOpen;			// 功放提前打开时间 秒

	TTaskState	m_tsTaskState;					// 当前任务状态
}CTodayTask;

// ------------------------------------------------------------------------------------------------------------------
typedef struct _TaskManager
{
	CTaskJobGroupList	m_lpJobGroups;				// 任务事件组表
	CTaskGroupList		m_lpTaskGroups;			// 任务时间组表
	CIntercutTaskGroup	m_lpIntercutTasks;			// 插播任务表
#if (GC_ZR & GC_ZR_REMOTE)
	CRemoteTaskGroup	m_lpYkTasks;				//遥控任务
	CRemoteTaskGroup	m_lpPhoneTasks;				//电话任务
#endif
	CTodayTask			m_lpTodayTask;				// 今日任务组
	int					m_nPortMaxCount;			// 最大分区数
	CSpecialMode		m_lpSpecMode;				// 晴雨模式
}CTaskManager;

// =========================================================================================

// =========================================================================================
/* // 任务事件结构
typedef struct _TaskJob
{
	void	*m_lpManager;	// CTaskManager				// 任务管理者
	void	*m_lpOwner;	// CTaskJobGroup				// 所属任务事件数
	_TaskHead		m_Head;
}CTaskJob;
*/

void tjInit(CTaskJob *tj);
void tjAssign(CTaskJob *, CTaskJob *);
BYTE *tjGetHeadBuffer(CTaskJob *tj);
int	tjGetHeadBufferSize(void);
int	tjGetSourceStateBufferSize(void);
BYTE *tjGetSourceStateBuffer(CTaskJob *tj );
TTaskTimeType tjGetTimeType(CTaskJob *tj );
PMyTime	tjGetStartTime(CTaskJob *tj );
PMyTime tjGetEndTime(CTaskJob *tj , PMyTime tt);
TTaskJobMode tjGetJobMode(CTaskJob *tj );
WORD tjGetPlayLength(CTaskJob *tj );
TSourceType tjGetSourceType(CTaskJob *tj );
BOOL tjGetPortState(CTaskJob *tj, BYTE nPort);

void tjSetTimeType(CTaskJob *tj, TTaskTimeType byType);
void tjSetStartMyTime(CTaskJob *tj, PMyTime);
void tjSetStartTime(CTaskJob *tj, BYTE byHour,  BYTE byMinute, BYTE bySecond);
void tjSetPlayLength(CTaskJob *tj, WORD);
void tjSetEndTime(CTaskJob *tj, PMyTime timeEnd);
void tjSetJobMode(CTaskJob *tj, TTaskJobMode byMode);
void tjSetSourceType(CTaskJob *tj, TSourceType bySourceType);
void tjSetPortState(CTaskJob *tj, BYTE nPort, BOOL bState);

TPlaySongType tjGetSongType(CTaskJob *tj );
BYTE tjGetVolume(CTaskJob *tj );
BOOL tjGetRandomMode(CTaskJob *tj );
BOOL tjGetRepeatMode(CTaskJob *tj );
WORD tjGetSongID(CTaskJob *tj );
BYTE tjGetTunerBand(CTaskJob *tj );
WORD tjGetTunerFreq(CTaskJob *tj );
WORD tjGetTunerStationID(CTaskJob *tj );
void tjSetSongType(CTaskJob *tj, TPlaySongType );
void tjSetVolume(CTaskJob *tj, BYTE );
void tjSetRandomMode(CTaskJob *tj, BOOL);
void tjSetRepeatMode(CTaskJob *tj, BOOL);
void tjSetSongID(CTaskJob *tj, WORD);
void tjSetTunerBand(CTaskJob *tj, BYTE);
void tjSetTunerFreq(CTaskJob *tj, WORD);

int tjCompareTime(CTaskJob *tj, PMyTime tm); // 比较任务时间， 返回值:<0--任务时间小于tm的秒数, 0--相等, >0--任务时间大于tm的秒数
// =========================================================================================

// 任务事件组结构
#if 0
typedef struct	{
	BYTE	  		byValid;									// 任务组有效否												1
	BYTE			byRes1;										// 占位														1
	BYTE			byRes2;										// 占位														1
	BYTE	  		byCount;									// 已填写的任务数											1
} TTaskJobGroupHead, *PTaskJobGroupHead;
// *************************************************************************************************
typedef	struct	{
	TTaskJobGroupHead	srtHead;								// 任务事件组头												4
	BYTE				byTaskIndexs[TASK_COUNT_OF_GROUP];		// 任务事件索引
	TTask				srtTaskJobs[TASK_COUNT_OF_GROUP];		// 任务事件列表												TASK_COUNT_OF_GROUP*TASK_STRUCT_SIZE 40*100
}TTaskJobGroup, *PTaskJobGroup;
// *************************************************************************************************
#endif
/*
// =========================================================================================
typedef struct _TaskJobGroup
{
	BOOL			m_bValid;										// 事件组有效否
	int				m_nMaxCount;									// 事件数
	int				m_nIndex;										// 组编号
	CTaskJob			m_Jobs[256];									// 播音事件数组
	ArrayList			m_TaskJobs;									// 事件列表
}CTaskJobGroup;
*/
int tjgCompareStartTime( Element elm1, Element elm2);

void tjgInit(CTaskJobGroup* tjg, int nMaxCount);
PMyTime	tjgGetNewTaskStartTime(CTaskJobGroup* tjg, PMyTime);
int	tjgGetIndex(CTaskJobGroup* tjg);
void tjgSetIndex(CTaskJobGroup* tjg, int nIndex);
void tjgAssign(CTaskJobGroup* tjg, CTaskJobGroup *);
BOOL tjgGetValid(CTaskJobGroup* tjg);
int	tjgGetMaxCount(CTaskJobGroup* tjg);
int	tjgGetCount(CTaskJobGroup* tjg);
BOOL tjgCanAddNew(CTaskJobGroup* tjg);
CTaskJob *tjgGetAt(CTaskJobGroup* tjg, int );
void tjgSort(CTaskJobGroup* tjg);
int	tjgAdd(CTaskJobGroup* tjg, CTaskJob *);
int	tjgIndexOf(CTaskJobGroup* tjg, CTaskJob *);
void tjgDeleteIndex(CTaskJobGroup* tjg, int );
void tjgDeleteJob(CTaskJobGroup* tjg, CTaskJob *);
void tjgClear(CTaskJobGroup* tjg);
CTaskJob *tjgNew(CTaskJobGroup* tjg);
// =========================================================================================

// =========================================================================================
// 任务事件组列表类型结构
#if 0
typedef struct	{
	DWORD  	dwCount;									// 4
	BYTE		byGroupIndexs[TASKGROUP_COUNT_OFLIST];	// 任务组索引
	TTaskJobGroup srtJobGroups[TASKGROUP_COUNT_OFLIST];	// 131584
}TTaskJobGroupList, *PTaskJobGroupList;
#endif
/*
typedef struct _TaskJobGroupList
{
	int				m_nMaxCount;
	CTaskJobGroup	JobGroups[TASKGROUP_COUNT_OFLIST];
	ArrayList			m_Groups;
}CTaskJobGroupList;
*/
int tjglCompare( Element elm1, Element elm2);

void tjglInit(CTaskJobGroupList *tjgl, int nMaxCount);
int	tjglGetCount(CTaskJobGroupList *tjgl);
BOOL tjglCanAddNew(CTaskJobGroupList *tjgl);
int	tjglGetMaxCount(CTaskJobGroupList *tjgl);
CTaskJobGroup *tjglGetAt(CTaskJobGroupList *tjgl, int );
CTaskJobGroup *tjglGetJobGroup(CTaskJobGroupList *tjgl, int nGroupIndex); // 根据组编号获取组
//int tjglAdd(CTaskJobGroup *);
int	tjglIndexOf(CTaskJobGroupList *tjgl, CTaskJobGroup *);
void tjglDeleteIndex(CTaskJobGroupList *tjgl, int );
void tjglDeleteGroup(CTaskJobGroupList *tjgl, CTaskJobGroup *);
void tjglClear(CTaskJobGroupList *tjgl);
CTaskJobGroup *tjglNew(CTaskJobGroupList *tjgl);
// =========================================================================================

// =========================================================================================
// 任务组类型结构
/*
typedef struct  
{
	TShortDate	srtStartDate;				// 开始日期
	TShortDate	srtEndDate;					// 结束日期
	BYTE		byTaskGroupOfWeek[7];		// 星期 1--日每天的任务组编号（1--32），0--为无任务
	BYTE		bySpecialGroup;				// 特殊任务组编号，用于晴雨模式
}_TaskGroupTag;

typedef enum	{
	wdSunday,
	wdMonday,
	wdTuesday,
	wdWednesday,
	wdThursday,
	wdFriday,
	wdSaturday
}WEEKDAY;

extern  const WEEKDAY nDisplayWeekday_tab[7];

typedef struct _TaskGroup
{
	_TaskGroupTag	m_Head;
}CTaskGroup;
*/	
void tgInit(CTaskGroup* tg);
int	tgGetHeadBufferSize(void);
BYTE *tgGetHeadBuffer(CTaskGroup* tg);
void tgAssign(CTaskGroup* tg, CTaskGroup *);
u8	tgGetGroupOfWeek(CTaskGroup* tg, WEEKDAY);
u8	tgGetGroupOfDate(CTaskGroup* tg, PMyDate );
u8	tgGetSpecialGroup(CTaskGroup* tg);
PShortDate tgGetStartDate(CTaskGroup* tg);
PShortDate tgGetEndDate(CTaskGroup* tg);

void tgSetGroupOfWeek(CTaskGroup* tg, WEEKDAY, CTaskJobGroup *);
void tgSetSpecialGorup(CTaskGroup* tg, CTaskJobGroup *lpGroup);
void tgSetStartDate(CTaskGroup* tg, BYTE byMonth, BYTE byDay);
void tgSetEndDate(CTaskGroup* tg, BYTE byMonth, BYTE byDay);
void tgSetStartMyDate(CTaskGroup* tg, PShortDate);
void tgSetEndMyDate(CTaskGroup* tg, PShortDate);
BOOL tgValidDate(CTaskGroup* tg, PShortDate lpdate);
// =========================================================================================

// =========================================================================================
// 任务组表
#if 0
typedef struct  
{
	DWORD		dwCount;								// 本表中任务组数
	BYTE		byGroupIndexs[TASKGROUP_COUNT_OFLIST];	// 任务组索引
	TTaskGroup	srtGroups[TASKGROUP_COUNT_OFLIST];		// 任务组列表 最多32个组
}TTaskGroupList, *PTaskGroupList;
#endif
/*
typedef struct _TaskGroupList
{
	int					m_nMaxCount;						// 最大任务组数
	CTaskManager		*m_lpOwner;
	CTaskGroup			Groups[TASKGROUP_COUNT_OFLIST];
	ArrayList				m_Groups;							// 任务组表
}CTaskGroupList;
*/
int tglCompareDate(Element elm1, Element elm2);

void tglInit(CTaskGroupList *tgl, int nMaxCount);
int	tglGetMaxCount(CTaskGroupList *tgl);
int	tglGetCount(CTaskGroupList *tgl);
BOOL tglCanAddNew(CTaskGroupList *tgl);
CTaskGroup *tglGetTaskGroup(CTaskGroupList *tgl, PMyDate tm);
CTaskGroup *tglGetAt(CTaskGroupList *tgl, int );
void tglSort(CTaskGroupList *tgl);
// int tglAdd(CTaskGroupList *tgl, CTaskGroup *);
int	tglIndexOf(CTaskGroupList *tgl, CTaskGroup *);
void tglDeleteIndex(CTaskGroupList *tgl, int );
void tglDeleteGroup(CTaskGroupList *tgl, CTaskGroup *);
void tglClear(CTaskGroupList *tgl);
CTaskGroup *tglNew(CTaskGroupList *tgl);
// =========================================================================================

// =========================================================================================
// 插播任务类型结构 
/*
typedef struct _IntercutTask
{
	TMyDate	m_Date;
	CTaskJob m_Job;
	CIntercutTaskGroup	*m_lpOwner;
}CIntercutTask;
*/
void itInit(CIntercutTask *it, PMyDate tm);
int	itGetDateBufferSize(void);
BYTE *itGetDateBuffer(CIntercutTask *it);
void itAssign(CIntercutTask *it, CIntercutTask *lpTask);
CTaskJob *itGetTaskJob(CIntercutTask *it);
void itSetTaskJob(CIntercutTask *it, CTaskJob *lpJob);
PMyDate itGetTaskDate(CIntercutTask *it);
void itSetTaskMyDate(CIntercutTask *it, PMyDate);
void itSetTaskDate(CIntercutTask *it, WORD wYear, WORD wMonth, WORD wDay);
BOOL itValidDate(CIntercutTask *it, PMyDate lpdate);

// =========================================================================================

// =========================================================================================
/*
typedef struct _IntercutTaskGroup
{
	int					m_nMaxCount;
	CTaskManager *		m_lpOwner;
	CIntercutTask			its[TASK_COUNT_OF_GROUP];
	ArrayList				m_Tasks;
}CIntercutTaskGroup;
*/
int itgCompareDate(Element elm1, Element elm2);

void itgInit(CIntercutTaskGroup *itg, int nMaxCount);
int	itgGetCount(CIntercutTaskGroup *itg);
BOOL itgCanAddNew(CIntercutTaskGroup *itg);
CIntercutTask *itgGetAt(CIntercutTaskGroup *itg, int );
// int itgAdd(CIntercutTaskGroup *itg, CIntercutTask *);
int	itgIndexOf(CIntercutTaskGroup *itg, CIntercutTask *);
void itgDeleteIndex(CIntercutTaskGroup *itg, int );
void itgDeleteTask(CIntercutTaskGroup *itg, CIntercutTask *);
void itgClear(CIntercutTaskGroup *itg);
void itgSort(CIntercutTaskGroup *itg);
CIntercutTask *itgNew(CIntercutTaskGroup *itg);
// =========================================================================================

// =========================================================================================
#if (GC_ZR & GC_ZR_REMOTE)
/*typedef struct _RemoteTask
{
int m_key;
int m_funcValue;
CTaskJob m_Job;
}CRemoteTask;*/
void rtInit(CRemoteTask *rt, TTaskTimeType type);
void rtAssign(CRemoteTask *rt, CRemoteTask *lpTask);
CTaskJob *rtGetTaskJob(CRemoteTask *rt);
int rtGetTaskKey(CRemoteTask *rt);
void rtSetTaskKey(CRemoteTask *rt, int key); 
int rtGetTaskFuncValue(CRemoteTask *rt);
void rtSetTaskFuncValue(CRemoteTask *rt, int funcValue);
BOOL rtGetTaskType(CRemoteTask *rt);//TRUE music, FALSE func
// =========================================================================================

// =========================================================================================
/*typedef struct _RemoteTaskGroup
{
	int					m_nMaxCount;
	CRemoteTask					its[TASK_COUNT_OF_GROUP];
	ArrayList				m_Tasks;
}CRemoteTaskGroup;*/
void rtgInit(CRemoteTaskGroup *rtg, int nMaxCount/* =TASK_COUNT_OF_GROUP */);
int rtgGetCount(CRemoteTaskGroup *rtg);
BOOL rtgCanAddNew(CRemoteTaskGroup *rtg);
int rtgCompareDate(Element elm1, Element elm2);
CRemoteTask	*rtgGetAt(CRemoteTaskGroup *rtg, int index);
int rtgAdd(CRemoteTaskGroup *rtg, CRemoteTask *lpTask);
int	rtgIndexOf(CRemoteTaskGroup *rtg, CRemoteTask *lpTask);
void rtgDeleteIndex(CRemoteTaskGroup *rtg, int index);
void rtgDeleteTask(CRemoteTaskGroup *rtg, CRemoteTask *lpTask);
void rtgClear(CRemoteTaskGroup *rtg);
void rtgSort(CRemoteTaskGroup *rtg);
CRemoteTask *rtgNew(CRemoteTaskGroup *rtg);
#endif
// =========================================================================================

// =========================================================================================
/*typedef struct _RemoteTaskGroup
{
int					m_nMaxCount;
CRemoteTask					its[TASK_COUNT_OF_GROUP];
ArrayList				m_Tasks;
}CRemoteTaskGroup;*/
// =========================================================================================

// =========================================================================================
/*
// 晴雨模式 类型
#define SPECIALMODE_INVALID		0		// 无效
#define SPECIALMODE_CURRENT		1		// 当天有效
#define SPECIALMODE_TWOLY		2		// 第二天有效
#define SPECIALMODE_THIRDLY		4		// 第三天有效

typedef struct _SpecialMode
{
	PMyDate			m_SpecialDate;
	BYTE	m_bySpecMode;				// 特殊任务模式
	BYTE	m_bySpecDayCount;			// 特殊任务执行天数
	CTaskManager *	m_lpOwner;
}CSpecialMode;
*/
void smInit(CSpecialMode *sm);
void smResetData(CSpecialMode *sm);
PMyDate smGetSpecialDate(CSpecialMode *sm);
void smSetSpecialDate(CSpecialMode *sm, PMyDate lpdate);
BOOL smValidDate(CSpecialMode *sm, PMyDate lpdate);
// =========================================================================================

// =========================================================================================
// 任务管理器结构
#if 0
typedef struct	{
	DWORD			dwHead;								// 任务文件头			4
	DWORD			dwVersion;							// 任务文件版本			4
	TTaskJobGroupList	srtTaskJob;						// 任务事件组		
	TTaskGroupList		srtTaskGroup;					// 常规任务组
	TIntercutTaskGroup	srtIntercut;					// 插播任务组		
	TRemoteTaskGroup	srtRemoteTask;					// 无线遥控任务组
	TRemoteTaskGroup	srtPhoneTask;					// 电话遥控任务
	TWaringTaskGroup	srtWaringTask;					// 警报任务组
	TSpecialMode		srtSpecMode;					// 晴雨模式
}TTaskData, *PTaskData;
#endif
// =========================================================================================

// #define TASK_DATA_SIZE			sizeof(TTaskData)				//任务文件的大小
extern U8 DaysOfMonth[];


void MyTimeAdd(PMyTime res, PMyTime tt, S32 iSecond);
void SecondToMyTime(U32 iSecond,PMyTime pt);
U32 MyTimeToSecond(PMyTime time1);
S8 CmpMyDate(PMyDate date1,PMyDate date2);
// -----------------------------------------------------------------------------------------
// 获取日期y-m-d的星期值 1-星期一， 2-星期二.....7-星期天
u8 DayOfWeek(u16 y, u8 m, u8 d);
u8 MyDateOfWeek(PMyDate);
// -----------------------------------------------------------------------------------------
// 计算从公元元年1月1号到tm日期经过的天数
u32 MyDateOfDays(PMyDate tm);
// -----------------------------------------------------------------------------------------
// 计算从1月1号到tm日期经过的天数
u32 ShortDateOfDays(PShortDate tm);

// 取得某年某月的天数
u8 GetDayCount(u16 wYear, u8 byMonth);

// =========================================================================================

// =========================================================================================
/*
typedef enum {
	tsTaskInvalid,				// 任务未加载
	tsTaskDone,					// 任务结束
	tsTaskReady,				// 下一条任务准备
	tsTaskStart,				// 下一条任务功放已打开
	tsTaskPauseed,				// 任务暂停中
	tsTaskRunning				// 任务执行中
}TTaskState;

typedef struct _TodayTask
{
	int				m_nCurSecond;			// 任务系统时间--换算为秒计数
	int				m_nHour;				// 当前小时
	int				m_nMinute;				// 当前分
	int				m_nSecond;				// 当前秒
	CTaskJobGroup	*m_lpTodayTask;			// 今日任务数据
	CTaskManager	*m_lpOwner;
	S32 volatile	m_iNextTask;			// 即将播放或当前正在播放的任务编号
	int	m_nAmpPwrPreOpen;			// 功放提前打开时间 秒

	TTaskState	m_tsTaskState;					// 当前任务状态
}CTodayTask;
*/
void ttInit(CTodayTask* tt);
void ttClear(CTodayTask* tt);					// 清除所有数据
TTaskState ttTaskState(CTodayTask* tt, int nCurSecond, int index);				// 获取nCurSecond时间index的任务状态
int ttGetCount(CTodayTask* tt);
CTaskJobGroup *ttGetTaskGroup(CTodayTask* tt);
TTaskState ttGetTaskState(CTodayTask* tt);		// 返回当前任务状态
TTaskState ttGetJobState(CTodayTask* tt, u8 index);		// 返回播音事件状态
void ttUpdateTaskState(CTodayTask* tt, PMyTime );
CTaskJob *ttGetNextTask(CTodayTask* tt);			// 返回当前时间的任务
int ttGetNextTaskID(CTodayTask* tt);			// 返回当前时间的任务编号


/* *******************************************************************************************************************************
任务文件格式：
项目								大小（字节）		内容
头(Header)							4				0x544C5958UL	// "XYLT"
版本号(Ver)							4				0x02010000UL	// 2.1.0.0
// 分区最大数(PortMaxCount)			1 // 已取消
事件组总数(JobGroupCount)			1				
事件组表1
	组编号(JobGroupID)				1
	组内事件总数(JobCount)			1
	事件表							sizeof(事件)*JobCount
		事件类型m_byTimeType		1
		开始时间m_StartTime			3
		播音长度m_wPlayLength		2
		事件类型m_byJobMode			1
		音源类型m_bySourceType		1
		音源状态m_SourceState		4
		分区状态表m_lpbyPorts;		(PortmaxCount+7) / 8

******************************************************************************************************************************* */

// =========================================================================================
/*
typedef struct _TaskManager
{
	CTaskJobGroupList	*m_lpJobGroups;				// 任务事件组表
	CTaskGroupList		*m_lpTaskGroups;			// 任务时间组表
	CIntercutTaskGroup	*m_lpIntercutTasks;			// 插播任务表
	CTodayTask			*m_lpTodayTask;				// 今日任务组
	int					m_nPortMaxCount;			// 最大分区数
	CSpecialMode		*m_lpSpecMode;				// 晴雨模式
	CRemoteFuncKeyList	*m_lpRemoteFuncKey;			// 无线遥控功能按键
}CTaskManager; */

void tmInit(CTaskManager *tm);
CTaskJobGroup*	tmGetGroupOfWeek(CTaskManager *tm, CTaskGroup* tg, WEEKDAY);
CTaskJobGroup*	tmGetGroupOfDate(CTaskManager *tm, CTaskGroup* tg, PMyDate );
CTaskJobGroup*	tmGetSpecialGroup(CTaskManager *tm, CTaskGroup* tg);

BOOL tmLoadHeaderFromFile(CTaskManager *tm, FIL * hFil);		// 读文件头
void tmLoadJobFromFile(CTaskManager *tm, FIL * hFil, CTaskJob *);			// 读事件
void tmLoadTaskGroupFromFile(CTaskManager *tm, FIL * hFil, CTaskGroup *lpGroup);
void tmLoadJobTaskFromFile(CTaskManager *tm, FIL * hFil);		// 读事件组
void tmLoadTaskGroupsFromFile(CTaskManager *tm, FIL * hFil);
void tmLoadIntercutTaskFromFile(CTaskManager *tm, FIL * hFil);

#if (GC_ZR &GC_ZR_REMOTE)
void tmLoadYkTaskFromFile(CTaskManager *tm, FIL * hFil);
void tmLoadPhoneTaskFromFile(CTaskManager *tm, FIL * hFil);
#endif

BOOL tmSaveHeaderToFile(CTaskManager *tm, FIL * hFil);		// 读文件头
BOOL tmSaveTaskGroupToFile(CTaskManager *tm, FIL * hFil, CTaskGroup *lpGroup);
BOOL tmSaveJobToFile(CTaskManager *tm, FIL * hFil, CTaskJob *);			// 读事件
BOOL tmSaveJobTaskToFile(CTaskManager *tm, FIL * hFil);		// 读事件组
BOOL tmSaveTaskGroupsToFile(CTaskManager *tm, FIL * hFil);
BOOL tmSaveIntercutTaskFromFile(CTaskManager *tm, FIL * hFil);

#if (GC_ZR & GC_ZR_REMOTE)
BOOL mSaveYkTaskFromFile(CTaskManager *tm, FIL * hFil);
BOOL mSavePhoneTaskFromFile(CTaskManager *tm, FIL * hFil);
#endif

void tmResetTaskData(CTaskManager *tm);
BOOL tmLoadFromFile(CTaskManager *tm, const char *strFileName);
BOOL tmSaveToFile(CTaskManager *tm, const char *strFileName);
void tmUpdateTodayTask(CTaskManager *tm, PMyDate dtm);					// 获取今日任务
BOOL tmIsSpecialDate(CTaskManager *tm, PMyDate dtm);			// 日期tm应该执行晴雨模式否
void tmGetTodayTaskGroup(CTaskManager *tm, CTaskJobGroup *lpGroup, PMyDate dtm);
// 晴雨模式相关函数
void tmSetSpecMode(CTaskManager *tm, PMyDate, BYTE );
// =========================================================================================

#endif

