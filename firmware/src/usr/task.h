#ifndef _TASK_H
#define _TASK_H

#include "arrayList.h"

#define PLAY_LENGTH_MAX				36000	//�������񲥷ų����������10Сʱ=36000��

#define TASK_COUNT_OF_GROUP			MYLIST_MAX_SIZE		//ÿ������������������
#define TASKGROUP_COUNT_OFLIST		0x08		//����������
#define TASK_VALID					0x80
#define TASKCOUNT_MASK				0x7F	


#define TASK_HEAD_STR				0x43475348UL	// "HSGC" С��ģʽ
#define TASK_VERSION				0x02010000UL	// 2.1.0.0

#define TASK_GRANULARITY			3				// �������ʱ�� ��

#define PORTCOUNT_MAX				4				// ���֧��160������
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

// ��������
/*
typedef enum	{
	tbFM,
	tbAM, 
	tbSW
} TTunerBand;
*/

// TTunerState C���԰汾
typedef struct {
	BYTE		byBand;      // ����
	BYTE			by2;		// δ������
	WORD 	wData;       // Ƶ����ֵ
}TTunerState,	*PTunerState;

// *****************************************************************************
// ���ſ���
// ��Ŀ����
typedef enum	{
	pstInvalid,
	pstSongFile,        	// �����ļ�
	pstSongFolder,    	// ����Ŀ¼
	pstRecordFile      	// ¼���ļ�
}TPlaySongType;

// ����ģʽ  �Ͱ��ֽ����ڱ�����Ŀ����
#define	pmRandom				1		// ���ģʽ
#define	pmRepeat				2		// �ظ�����

// ��������ģʽ
typedef struct { 
	BYTE	byRandom:1;					// ������ŷ�
	BYTE	byReapeat:1;				// �ظ�����
	BYTE	byVolume:6;					// ����0-31
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
#define	IsRandomMode(x)		(x&pmRandom)		// �����ģʽ��
#define	ISRepeatMode(x)			(x&pmRepeat)		// ���ظ�ģʽ��
#define	SetRandomMode(x)		(x|=pmRandom)		// ��Ϊ���ģʽ
#define	ClrRandomMode(x)		(x&=~pmRandom)		// ȡ�����ģʽ
#define	SetRepeatMode(x)		(x|=pmRepeat)		// ��Ϊ���ģʽ
#define	ClrRepeatMode(x)		(x&=~pmRepeat)		// ȡ�����ģʽ
*/

typedef union	{
	TTunerState			srtTuner;
	TSongState			srtSong;
	TOtherSourceState	srtOther;
}TSourceState, *PSourceState;

// ����ṹ
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
// ����������
typedef enum	{
	tjmInvalid=0,					// ��Ч����
	tjmPlay,						// ����
	tjmRecord						// ¼��
}TTaskJobMode;
// *************************************************************************************************
// ��������
typedef enum	{
	tttNormal,						// ����
	tttIntercut,					// �岥
#if (GC_ZR & GC_ZR_REMOTE)
	tttYk,						// �������񣬱�����ң�ء�����
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
	TTaskTimeType	byTimeType;						// ������Ч��														1
	TMyTime			srtStartTime;			   		// ����ʼʱ��   									 				3
	TTaskJobMode	byJobMode;					// ��������															1
	TSourceType		bySourceType;					// ��Դ����															1
	WORD			wPlayLength;			   		// ����ʱ��															2
	TSourceState	srtSourceState;					// ��Դ״ֵ̬													 	4
}_TaskHead;

// =========================================================================================

// ------------------------------------------------------------------------------------------------------------------
typedef struct _TaskJob
{
	//void	*m_lpManager;	// CTaskManager				// ���������
	//void	*m_lpOwner;	// CTaskJobGroup				// ���������¼���
	_TaskHead		m_Head;
#if (GC_ZR & GC_ZR_FC)
	WORD	mPortState;
#else
	BYTE	mPortState;											// ����״̬ÿһλ����1������
#endif
}CTaskJob;
// ------------------------------------------------------------------------------------------------------------------
typedef struct _TaskJobGroup
{
	u8			m_bValid;										// �¼�����Ч��
	u8				m_nMaxCount;									// �¼���
	u8				m_nIndex;										// ���?1~8?
	CTaskJob			m_Jobs[TASK_COUNT_OF_GROUP];									// �����¼�����
	ArrayList			m_TaskJobs;									// �¼��б�
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
	TShortDate	srtStartDate;				// ��ʼ����
	TShortDate	srtEndDate;					// ��������
	BYTE		byTaskGroupOfWeek[7];		// ���� 1--��ÿ����������ţ�1--32����0--Ϊ������
	BYTE		bySpecialGroup;				// �����������ţ���������ģʽ
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
	int					m_nMaxCount;						// �����������
	CTaskGroup			Groups[TASKGROUP_COUNT_OFLIST];
	ArrayList				m_Groups;							// �������
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
// ����ģʽ ����
#define SPECIALMODE_INVALID		0		// ��Ч
#define SPECIALMODE_CURRENT		1		// ������Ч
#define SPECIALMODE_TWOLY		2		// �ڶ�����Ч
#define SPECIALMODE_THIRDLY		4		// ��������Ч

typedef struct _SpecialMode
{
	TMyDate			m_SpecialDate;
	BYTE	m_bySpecMode;				// ��������ģʽ
	BYTE	m_bySpecDayCount;			// ��������ִ������
}CSpecialMode;
// ------------------------------------------------------------------------------------------------------------------
typedef enum {
	tsTaskInvalid,				// ����δ����
	tsTaskDone,					// �������
	tsTaskReady,				// ��һ������׼��
	tsTaskStart,				// ��һ�����񹦷��Ѵ�
	tsTaskPauseed,				// ������ͣ��
	tsTaskRunning				// ����ִ����
}TTaskState;

typedef struct _TodayTask
{
	//int				m_nCurSecond;			// ����ϵͳʱ��--����Ϊ�����
	//int				m_nHour;				// ��ǰСʱ
	//int				m_nMinute;				// ��ǰ��
	//int				m_nSecond;				// ��ǰ��
	CTaskJobGroup	m_lpTodayTask;			// ������������
	TTaskState		m_JobStates[TASK_COUNT_OF_GROUP];
	S32 volatile	m_iNextTask;			// �������Ż�ǰ���ڲ��ŵ�������
	int	m_nAmpPwrPreOpen;			// ������ǰ��ʱ�� ��

	TTaskState	m_tsTaskState;					// ��ǰ����״̬
}CTodayTask;

// ------------------------------------------------------------------------------------------------------------------
typedef struct _TaskManager
{
	CTaskJobGroupList	m_lpJobGroups;				// �����¼����
	CTaskGroupList		m_lpTaskGroups;			// ����ʱ�����
	CIntercutTaskGroup	m_lpIntercutTasks;			// �岥�����
#if (GC_ZR & GC_ZR_REMOTE)
	CRemoteTaskGroup	m_lpYkTasks;				//ң������
	CRemoteTaskGroup	m_lpPhoneTasks;				//�绰����
#endif
	CTodayTask			m_lpTodayTask;				// ����������
	int					m_nPortMaxCount;			// ��������
	CSpecialMode		m_lpSpecMode;				// ����ģʽ
}CTaskManager;

// =========================================================================================

// =========================================================================================
/* // �����¼��ṹ
typedef struct _TaskJob
{
	void	*m_lpManager;	// CTaskManager				// ���������
	void	*m_lpOwner;	// CTaskJobGroup				// ���������¼���
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

int tjCompareTime(CTaskJob *tj, PMyTime tm); // �Ƚ�����ʱ�䣬 ����ֵ:<0--����ʱ��С��tm������, 0--���, >0--����ʱ�����tm������
// =========================================================================================

// �����¼���ṹ
#if 0
typedef struct	{
	BYTE	  		byValid;									// ��������Ч��												1
	BYTE			byRes1;										// ռλ														1
	BYTE			byRes2;										// ռλ														1
	BYTE	  		byCount;									// ����д��������											1
} TTaskJobGroupHead, *PTaskJobGroupHead;
// *************************************************************************************************
typedef	struct	{
	TTaskJobGroupHead	srtHead;								// �����¼���ͷ												4
	BYTE				byTaskIndexs[TASK_COUNT_OF_GROUP];		// �����¼�����
	TTask				srtTaskJobs[TASK_COUNT_OF_GROUP];		// �����¼��б�												TASK_COUNT_OF_GROUP*TASK_STRUCT_SIZE 40*100
}TTaskJobGroup, *PTaskJobGroup;
// *************************************************************************************************
#endif
/*
// =========================================================================================
typedef struct _TaskJobGroup
{
	BOOL			m_bValid;										// �¼�����Ч��
	int				m_nMaxCount;									// �¼���
	int				m_nIndex;										// ����
	CTaskJob			m_Jobs[256];									// �����¼�����
	ArrayList			m_TaskJobs;									// �¼��б�
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
// �����¼����б����ͽṹ
#if 0
typedef struct	{
	DWORD  	dwCount;									// 4
	BYTE		byGroupIndexs[TASKGROUP_COUNT_OFLIST];	// ����������
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
CTaskJobGroup *tjglGetJobGroup(CTaskJobGroupList *tjgl, int nGroupIndex); // �������Ż�ȡ��
//int tjglAdd(CTaskJobGroup *);
int	tjglIndexOf(CTaskJobGroupList *tjgl, CTaskJobGroup *);
void tjglDeleteIndex(CTaskJobGroupList *tjgl, int );
void tjglDeleteGroup(CTaskJobGroupList *tjgl, CTaskJobGroup *);
void tjglClear(CTaskJobGroupList *tjgl);
CTaskJobGroup *tjglNew(CTaskJobGroupList *tjgl);
// =========================================================================================

// =========================================================================================
// ���������ͽṹ
/*
typedef struct  
{
	TShortDate	srtStartDate;				// ��ʼ����
	TShortDate	srtEndDate;					// ��������
	BYTE		byTaskGroupOfWeek[7];		// ���� 1--��ÿ����������ţ�1--32����0--Ϊ������
	BYTE		bySpecialGroup;				// �����������ţ���������ģʽ
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
// �������
#if 0
typedef struct  
{
	DWORD		dwCount;								// ��������������
	BYTE		byGroupIndexs[TASKGROUP_COUNT_OFLIST];	// ����������
	TTaskGroup	srtGroups[TASKGROUP_COUNT_OFLIST];		// �������б� ���32����
}TTaskGroupList, *PTaskGroupList;
#endif
/*
typedef struct _TaskGroupList
{
	int					m_nMaxCount;						// �����������
	CTaskManager		*m_lpOwner;
	CTaskGroup			Groups[TASKGROUP_COUNT_OFLIST];
	ArrayList				m_Groups;							// �������
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
// �岥�������ͽṹ 
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
// ����ģʽ ����
#define SPECIALMODE_INVALID		0		// ��Ч
#define SPECIALMODE_CURRENT		1		// ������Ч
#define SPECIALMODE_TWOLY		2		// �ڶ�����Ч
#define SPECIALMODE_THIRDLY		4		// ��������Ч

typedef struct _SpecialMode
{
	PMyDate			m_SpecialDate;
	BYTE	m_bySpecMode;				// ��������ģʽ
	BYTE	m_bySpecDayCount;			// ��������ִ������
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
// ����������ṹ
#if 0
typedef struct	{
	DWORD			dwHead;								// �����ļ�ͷ			4
	DWORD			dwVersion;							// �����ļ��汾			4
	TTaskJobGroupList	srtTaskJob;						// �����¼���		
	TTaskGroupList		srtTaskGroup;					// ����������
	TIntercutTaskGroup	srtIntercut;					// �岥������		
	TRemoteTaskGroup	srtRemoteTask;					// ����ң��������
	TRemoteTaskGroup	srtPhoneTask;					// �绰ң������
	TWaringTaskGroup	srtWaringTask;					// ����������
	TSpecialMode		srtSpecMode;					// ����ģʽ
}TTaskData, *PTaskData;
#endif
// =========================================================================================

// #define TASK_DATA_SIZE			sizeof(TTaskData)				//�����ļ��Ĵ�С
extern U8 DaysOfMonth[];


void MyTimeAdd(PMyTime res, PMyTime tt, S32 iSecond);
void SecondToMyTime(U32 iSecond,PMyTime pt);
U32 MyTimeToSecond(PMyTime time1);
S8 CmpMyDate(PMyDate date1,PMyDate date2);
// -----------------------------------------------------------------------------------------
// ��ȡ����y-m-d������ֵ 1-����һ�� 2-���ڶ�.....7-������
u8 DayOfWeek(u16 y, u8 m, u8 d);
u8 MyDateOfWeek(PMyDate);
// -----------------------------------------------------------------------------------------
// ����ӹ�ԪԪ��1��1�ŵ�tm���ھ���������
u32 MyDateOfDays(PMyDate tm);
// -----------------------------------------------------------------------------------------
// �����1��1�ŵ�tm���ھ���������
u32 ShortDateOfDays(PShortDate tm);

// ȡ��ĳ��ĳ�µ�����
u8 GetDayCount(u16 wYear, u8 byMonth);

// =========================================================================================

// =========================================================================================
/*
typedef enum {
	tsTaskInvalid,				// ����δ����
	tsTaskDone,					// �������
	tsTaskReady,				// ��һ������׼��
	tsTaskStart,				// ��һ�����񹦷��Ѵ�
	tsTaskPauseed,				// ������ͣ��
	tsTaskRunning				// ����ִ����
}TTaskState;

typedef struct _TodayTask
{
	int				m_nCurSecond;			// ����ϵͳʱ��--����Ϊ�����
	int				m_nHour;				// ��ǰСʱ
	int				m_nMinute;				// ��ǰ��
	int				m_nSecond;				// ��ǰ��
	CTaskJobGroup	*m_lpTodayTask;			// ������������
	CTaskManager	*m_lpOwner;
	S32 volatile	m_iNextTask;			// �������Ż�ǰ���ڲ��ŵ�������
	int	m_nAmpPwrPreOpen;			// ������ǰ��ʱ�� ��

	TTaskState	m_tsTaskState;					// ��ǰ����״̬
}CTodayTask;
*/
void ttInit(CTodayTask* tt);
void ttClear(CTodayTask* tt);					// �����������
TTaskState ttTaskState(CTodayTask* tt, int nCurSecond, int index);				// ��ȡnCurSecondʱ��index������״̬
int ttGetCount(CTodayTask* tt);
CTaskJobGroup *ttGetTaskGroup(CTodayTask* tt);
TTaskState ttGetTaskState(CTodayTask* tt);		// ���ص�ǰ����״̬
TTaskState ttGetJobState(CTodayTask* tt, u8 index);		// ���ز����¼�״̬
void ttUpdateTaskState(CTodayTask* tt, PMyTime );
CTaskJob *ttGetNextTask(CTodayTask* tt);			// ���ص�ǰʱ�������
int ttGetNextTaskID(CTodayTask* tt);			// ���ص�ǰʱ���������


/* *******************************************************************************************************************************
�����ļ���ʽ��
��Ŀ								��С���ֽڣ�		����
ͷ(Header)							4				0x544C5958UL	// "XYLT"
�汾��(Ver)							4				0x02010000UL	// 2.1.0.0
// ���������(PortMaxCount)			1 // ��ȡ��
�¼�������(JobGroupCount)			1				
�¼����1
	����(JobGroupID)				1
	�����¼�����(JobCount)			1
	�¼���							sizeof(�¼�)*JobCount
		�¼�����m_byTimeType		1
		��ʼʱ��m_StartTime			3
		��������m_wPlayLength		2
		�¼�����m_byJobMode			1
		��Դ����m_bySourceType		1
		��Դ״̬m_SourceState		4
		����״̬��m_lpbyPorts;		(PortmaxCount+7) / 8

******************************************************************************************************************************* */

// =========================================================================================
/*
typedef struct _TaskManager
{
	CTaskJobGroupList	*m_lpJobGroups;				// �����¼����
	CTaskGroupList		*m_lpTaskGroups;			// ����ʱ�����
	CIntercutTaskGroup	*m_lpIntercutTasks;			// �岥�����
	CTodayTask			*m_lpTodayTask;				// ����������
	int					m_nPortMaxCount;			// ��������
	CSpecialMode		*m_lpSpecMode;				// ����ģʽ
	CRemoteFuncKeyList	*m_lpRemoteFuncKey;			// ����ң�ع��ܰ���
}CTaskManager; */

void tmInit(CTaskManager *tm);
CTaskJobGroup*	tmGetGroupOfWeek(CTaskManager *tm, CTaskGroup* tg, WEEKDAY);
CTaskJobGroup*	tmGetGroupOfDate(CTaskManager *tm, CTaskGroup* tg, PMyDate );
CTaskJobGroup*	tmGetSpecialGroup(CTaskManager *tm, CTaskGroup* tg);

BOOL tmLoadHeaderFromFile(CTaskManager *tm, FIL * hFil);		// ���ļ�ͷ
void tmLoadJobFromFile(CTaskManager *tm, FIL * hFil, CTaskJob *);			// ���¼�
void tmLoadTaskGroupFromFile(CTaskManager *tm, FIL * hFil, CTaskGroup *lpGroup);
void tmLoadJobTaskFromFile(CTaskManager *tm, FIL * hFil);		// ���¼���
void tmLoadTaskGroupsFromFile(CTaskManager *tm, FIL * hFil);
void tmLoadIntercutTaskFromFile(CTaskManager *tm, FIL * hFil);

#if (GC_ZR &GC_ZR_REMOTE)
void tmLoadYkTaskFromFile(CTaskManager *tm, FIL * hFil);
void tmLoadPhoneTaskFromFile(CTaskManager *tm, FIL * hFil);
#endif

BOOL tmSaveHeaderToFile(CTaskManager *tm, FIL * hFil);		// ���ļ�ͷ
BOOL tmSaveTaskGroupToFile(CTaskManager *tm, FIL * hFil, CTaskGroup *lpGroup);
BOOL tmSaveJobToFile(CTaskManager *tm, FIL * hFil, CTaskJob *);			// ���¼�
BOOL tmSaveJobTaskToFile(CTaskManager *tm, FIL * hFil);		// ���¼���
BOOL tmSaveTaskGroupsToFile(CTaskManager *tm, FIL * hFil);
BOOL tmSaveIntercutTaskFromFile(CTaskManager *tm, FIL * hFil);

#if (GC_ZR & GC_ZR_REMOTE)
BOOL mSaveYkTaskFromFile(CTaskManager *tm, FIL * hFil);
BOOL mSavePhoneTaskFromFile(CTaskManager *tm, FIL * hFil);
#endif

void tmResetTaskData(CTaskManager *tm);
BOOL tmLoadFromFile(CTaskManager *tm, const char *strFileName);
BOOL tmSaveToFile(CTaskManager *tm, const char *strFileName);
void tmUpdateTodayTask(CTaskManager *tm, PMyDate dtm);					// ��ȡ��������
BOOL tmIsSpecialDate(CTaskManager *tm, PMyDate dtm);			// ����tmӦ��ִ������ģʽ��
void tmGetTodayTaskGroup(CTaskManager *tm, CTaskJobGroup *lpGroup, PMyDate dtm);
// ����ģʽ��غ���
void tmSetSpecMode(CTaskManager *tm, PMyDate, BYTE );
// =========================================================================================

#endif

