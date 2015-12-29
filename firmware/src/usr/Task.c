// TaskData.cpp : 实现文件
//
#include "hal.h"
#include "ff.h"
#include "arrayList.h"
#include "Task.h"
#include <string.h>
#include "log.h"
#define SetTGValid(d)	(d = 1)
#define SetTGInvalid(d) (d = 0)

const WEEKDAY nDisplayWeekday_tab[7]={wdMonday, wdTuesday, wdWednesday, wdThursday, wdFriday, wdSaturday, wdSunday};

#define ZeroMemory(x, y)  memset(x, 0, y)

// =========================================================================================
// CTaskJob
void tjInit(CTaskJob *tj)
{
	ZeroMemory(tjGetHeadBuffer(tj), tjGetHeadBufferSize());
	tj->m_Head.byJobMode= tjmPlay;
	tj->m_Head.byTimeType= tttNormal;
	tjSetSourceType(tj, stSongFile);
	tjSetSongType(tj, pstSongFile);
	tjSetSongID(tj, 1);
	tjSetVolume(tj, 31);
	// tjSetRepeatMode(FALSE);
	tjSetRandomMode(tj, FALSE);
	tj->m_Head.wPlayLength= 60; //默认任务结束时间1分钟
#if (GC_ZR & GC_ZR_FC)
	tj->mPortState= 0xFFFF;			// 默认打开所有分区
#else
	tj->mPortState= 0xFF;			// 默认打开所有分区
#endif
	// ZeroMemory(tjGetSourceStateBuffer(tj), tjGetSourceStateBufferSize());
}

BYTE	*tjGetHeadBuffer(CTaskJob *tj)
{
	return (BYTE *)&tj->m_Head;
}

int		tjGetHeadBufferSize()
{
	return sizeof(_TaskHead);
}

int		tjGetSourceStateBufferSize()
{
	return sizeof(TSourceState);
}

BYTE	*tjGetSourceStateBuffer(CTaskJob *tj)
{
	return (BYTE *)&tj->m_Head.srtSourceState;
}
TTaskTimeType	tjGetTimeType(CTaskJob *tj )
{
	return tj->m_Head.byTimeType;
}

PMyTime tjGetStartTime(CTaskJob *tj )
{
	return &tj->m_Head.srtStartTime;
}

PMyTime tjGetEndTime(CTaskJob *tj , PMyTime tt)
{
	MyTimeAdd(tt, &tj->m_Head.srtStartTime, tj->m_Head.wPlayLength);
	return tt;
}


TTaskJobMode	tjGetJobMode(CTaskJob *tj )
{
	return tj->m_Head.byJobMode;
}

WORD			tjGetPlayLength(CTaskJob *tj )
{
	return tj->m_Head.wPlayLength;
}

TSourceType		tjGetSourceType(CTaskJob *tj )
{
	return tj->m_Head.bySourceType;
}

void			tjSetTimeType(CTaskJob *tj, TTaskTimeType byType)
{
	tj->m_Head.byTimeType= byType;
}

void tjSetStartMyTime(CTaskJob *tj, PMyTime lpTime)
{
	tj->m_Head.srtStartTime.byHour= lpTime->byHour;
	tj->m_Head.srtStartTime.byMinute= lpTime->byMinute;
	tj->m_Head.srtStartTime.bySecond= lpTime->bySecond;
}

void tjSetStartTime(CTaskJob *tj, BYTE byHour,  BYTE byMinute, BYTE bySecond)
{
	tj->m_Head.srtStartTime.byHour= byHour;
	tj->m_Head.srtStartTime.byMinute= byMinute;
	tj->m_Head.srtStartTime.bySecond= bySecond;
}

void tjSetPlayLength(CTaskJob *tj, WORD wLength)
{
	tj->m_Head.wPlayLength= wLength;
}

void tjSetEndTime(CTaskJob *tj, PMyTime timeEnd)
{
	tj->m_Head.wPlayLength=MyTimeToSecond(timeEnd)-MyTimeToSecond(tjGetStartTime(tj));
}

void			tjSetJobMode(CTaskJob *tj, TTaskJobMode byMode)
{
	tj->m_Head.byJobMode= byMode;
}

void			tjSetSourceType(CTaskJob *tj, TSourceType bySourceType)
{
	tj->m_Head.bySourceType=bySourceType;
}

TPlaySongType	tjGetSongType(CTaskJob *tj )
{
	return tj->m_Head.srtSourceState.srtSong.bySongType;
}

BYTE			tjGetVolume(CTaskJob *tj )
{
	return tj->m_Head.srtSourceState.srtSong.byMode.byVolume;
}

BOOL			tjGetRandomMode(CTaskJob *tj )
{
	return (BOOL)(tj->m_Head.srtSourceState.srtSong.byMode.byRandom>0);
}

BOOL			tjGetRepeatMode(CTaskJob *tj )
{
	return (BOOL)(tj->m_Head.srtSourceState.srtSong.byMode.byReapeat>0);
}

WORD			tjGetSongID(CTaskJob *tj )
{
	return tj->m_Head.srtSourceState.srtSong.wSongID;
}

BYTE		tjGetTunerBand(CTaskJob *tj )
{
	return tj->m_Head.srtSourceState.srtTuner.byBand;
}

WORD			tjGetTunerFreq(CTaskJob *tj )
{
	return tj->m_Head.srtSourceState.srtTuner.wData;
}

WORD			tjGetTunerStationID(CTaskJob *tj )
{
	return tj->m_Head.srtSourceState.srtTuner.wData;
}

void tjSetSongType(CTaskJob *tj, TPlaySongType bySongType)
{
	tj->m_Head.srtSourceState.srtSong.bySongType= bySongType;
}

void tjSetVolume(CTaskJob *tj, BYTE byVolume)
{
	if(byVolume>31)
		byVolume=31;
	tj->m_Head.srtSourceState.srtSong.byMode.byVolume=byVolume;
}

void tjSetRandomMode(CTaskJob *tj, BOOL bRandomMode)
{
	tj->m_Head.srtSourceState.srtSong.byMode.byRandom=(BYTE)bRandomMode;
}

void tjSetRepeatMode(CTaskJob *tj, BOOL bRepeatMode)
{
	tj->m_Head.srtSourceState.srtSong.byMode.byReapeat=(BYTE)bRepeatMode;
}

void tjSetSongID(CTaskJob *tj, WORD nSongID)
{
	tj->m_Head.srtSourceState.srtSong.wSongID= nSongID;
}

void tjSetTunerBand(CTaskJob *tj, BYTE byBand)
{
	tj->m_Head.srtSourceState.srtTuner.byBand=byBand;
}

void tjSetTunerFreq(CTaskJob *tj, WORD wFreq)
{
	tj->m_Head.srtSourceState.srtTuner.wData=wFreq;
}

void tjAssign(CTaskJob *tj, CTaskJob *lpTask)
{
	memcpy(tjGetHeadBuffer(tj), tjGetHeadBuffer(lpTask), tjGetHeadBufferSize());
	tj->mPortState= lpTask->mPortState;
}

BOOL tjGetPortState(CTaskJob *tj, BYTE nPort)
{
	return (BOOL)((tj->mPortState&(1<<(nPort-1)))>0);
}

void tjSetPortState(CTaskJob *tj, BYTE nPort, BOOL bState)
{
	BYTE byState=1<<(nPort-1);
	if(bState)
		tj->mPortState |= byState;
	else
		tj->mPortState &= ~byState;
}

// 比较任务时间， 返回值:<0--任务时间小于tm的秒数, 0--相等, >0--任务时间大于tm的秒数
int	tjCompareTime(CTaskJob *tj, PMyTime tm)
{
	u32 nCurSec= MyTimeToSecond(tm);
	u32 nStartSec= MyTimeToSecond(tjGetStartTime(tj));
	if (nStartSec>nCurSec)
		return 1;
	else
		if(nStartSec<nCurSec)
			return -1;
		else
			return 0;
}


// CTaskJobGroup
void tjgInit(CTaskJobGroup* tjg, int nMaxCount)
{
	tjg->m_nMaxCount= nMaxCount;
	alInit(&tjg->m_TaskJobs, tjgCompareStartTime);
}

int			tjgGetIndex(CTaskJobGroup* tjg)
{
	return tjg->m_nIndex;
}

void			tjgSetIndex(CTaskJobGroup* tjg, int nIndex)
{
	tjg->m_nIndex= nIndex;
}

BOOL			tjgGetValid(CTaskJobGroup* tjg)
{
	return (BOOL)tjg->m_bValid;
}

int				tjgGetMaxCount(CTaskJobGroup* tjg)
{
	return tjg->m_nMaxCount;
}

int				tjgGetCount(CTaskJobGroup* tjg)
{
	return tjg->m_TaskJobs.size;
}

BOOL			tjgCanAddNew(CTaskJobGroup* tjg)
{
	return (BOOL)(tjgGetCount(tjg)<tjg->m_nMaxCount);
}

CTaskJob	*tjgGetAt(CTaskJobGroup* tjg, int index)
{
	return (CTaskJob *)alGetElement(&tjg->m_TaskJobs, index);
}

int	tjgAdd(CTaskJobGroup* tjg, CTaskJob *lpTask)
{
	int index;
	if(lpTask==NULL) return -1;
	index= tjgIndexOf(tjg, lpTask);
	if(index!=-1) return -1;
	return alAddElement(&tjg->m_TaskJobs, lpTask);
}

int	tjgIndexOf(CTaskJobGroup* tjg, CTaskJob *lpTask)
{
	return alIndexof(&tjg->m_TaskJobs, lpTask);
}

void tjgDeleteIndex(CTaskJobGroup* tjg, int index)
{
	alRemoveIndex(&tjg->m_TaskJobs, index);
}

void tjgDeleteJob(CTaskJobGroup* tjg, CTaskJob *lpTask)
{
	alRemoveElement(&tjg->m_TaskJobs, lpTask);
}

int tjgCompareStartTime( Element elm1, Element elm2)
{
	CTaskJob *lpItem1= (CTaskJob *)elm1, *lpItem2= (CTaskJob *)elm2;
	u32 tm1= MyTimeToSecond(tjGetStartTime(lpItem1)),
		tm2= MyTimeToSecond(tjGetStartTime(lpItem2));
	if(tm1>tm2)
		return 1;
	else
		if(tm1==tm2)
		{
			TTaskTimeType t1, t2;
			t1= tjGetTimeType(lpItem1);
			t2= tjGetTimeType(lpItem2);
			if(t1>t2)
				return 1;
			else
				if(t1==t2)
					return 0;
				else
					return -1;
		}
		else
			return -1;
}

void tjgSort(CTaskJobGroup* tjg)
{
	alSort(&tjg->m_TaskJobs);
}

void tjgAssign(CTaskJobGroup* tjg, CTaskJobGroup *lpGroup)
{
	CTaskJob *lpTask;
	int i;
	alClear(&tjg->m_TaskJobs);
	if(lpGroup==NULL) return;
	tjg->m_bValid= lpGroup->m_bValid;
	tjg->m_nMaxCount= lpGroup->m_nMaxCount;
	for(i=0; i<lpGroup->m_TaskJobs.size; i++)
	{
		lpTask= tjgNew(tjg);
		tjAssign(lpTask, tjgGetAt(lpGroup, i));
	}
}

PMyTime tjgGetNewTaskStartTime(CTaskJobGroup* tjg, PMyTime tm)
{
	CTaskJob *lptask;
	u32 nSec=0;
	if(tjgGetCount(tjg))
	{
		lptask= tjgGetAt(tjg, tjgGetCount(tjg)-1);
		nSec =MyTimeToSecond(tjGetStartTime(lptask))+tjGetPlayLength(lptask)+TASK_GRANULARITY;
	}
	SecondToMyTime(nSec, tm);
	return tm;
}

void tjgClear(CTaskJobGroup* tjg)
{
	alClear(&tjg->m_TaskJobs);	
}

CTaskJob *tjgNew(CTaskJobGroup* tjg)
{
	CTaskJob *lpJob;
	if(!tjgCanAddNew(tjg)) return NULL;
	lpJob= &tjg->m_Jobs[tjgGetCount(tjg)];
	//lpJob->m_lpOwner= tjg;
	//lpJob->m_lpManager= tjg->m_lpManager;
	alAddElement(&tjg->m_TaskJobs, lpJob);
	return lpJob;
}

// =========================================================================================
int tjglCompare( Element elm1, Element elm2)
{
	return 0;
}

// CTaskJobGroupList
void tjglInit(CTaskJobGroupList *tjgl, int nMaxCount/* = TASKGROUP_COUNT_OFLIST */)
{
	u8 i;
	CTaskJobGroup *lpGroup;
	tjgl->m_nMaxCount= nMaxCount;
	alInit(&tjgl->m_Groups, tjglCompare);
	for(i=0;i<TASKGROUP_COUNT_OFLIST;i++)
	{
#if 0 // 固定使用，不使用动态加入
		tjglNew(tjgl); 
#else
		lpGroup= &tjgl->JobGroups[i];
		tjgInit(lpGroup, TASK_COUNT_OF_GROUP);
		alAddElement(&tjgl->m_Groups, lpGroup);
		tjgSetIndex(lpGroup, i+1);
#endif
	}
}

int		tjglGetCount(CTaskJobGroupList *tjgl)
{
	return (int)tjgl->m_Groups.size;
}

BOOL	tjglCanAddNew(CTaskJobGroupList *tjgl)
{
	return (BOOL)(tjglGetCount(tjgl)<tjgl->m_nMaxCount);
}

int		tjglGetMaxCount(CTaskJobGroupList *tjgl)
{
	return tjgl->m_nMaxCount;
}

CTaskJobGroup	*tjglGetAt(CTaskJobGroupList *tjgl, int index)
{
	return (CTaskJobGroup *)alGetElement(&tjgl->m_Groups, index);
}

CTaskJobGroup   *tjglGetJobGroup(CTaskJobGroupList *tjgl, int nGroupIndex) // 根据组编号获取组
{
	int i;
	CTaskJobGroup *lpGroup;
	for(i=0;i<tjglGetCount(tjgl);i++)
	{
		lpGroup= tjglGetAt(tjgl, i);
		if (tjgGetIndex(lpGroup)==nGroupIndex)
			return lpGroup;
	}
	return NULL;
}

int				tjglAdd(CTaskJobGroupList *tjgl, CTaskJobGroup *lpGroup)
{
	int index= tjglIndexOf(tjgl, lpGroup);
	if(index!=-1) return -1;
	return alAddElement(&tjgl->m_Groups, lpGroup);
}

int				tjglIndexOf(CTaskJobGroupList *tjgl, CTaskJobGroup *lpGroup)
{
	return  alIndexof(&tjgl->m_Groups, lpGroup);
}

void			tjglDeleteIndex(CTaskJobGroupList *tjgl, int index)
{
	alRemoveIndex(&tjgl->m_Groups, index);
}

void			tjglDeleteGroup(CTaskJobGroupList *tjgl, CTaskJobGroup *lpGroup)
{
	alRemoveElement(&tjgl->m_Groups, lpGroup);
}

void			tjglClear(CTaskJobGroupList *tjgl)
{
	alClear(&tjgl->m_Groups);
}

CTaskJobGroup *tjglNew(CTaskJobGroupList *tjgl)
{
	int i, nCount;
	CTaskJobGroup *lpGroup;
	if(!tjglCanAddNew(tjgl)) return NULL;
	nCount= tjglGetMaxCount(tjgl);
	for(i=0; i<nCount; i++)
	{
		lpGroup= &tjgl->JobGroups[i];
		if(lpGroup->m_bValid==0)
			break;
	}
	if(i>=nCount) return NULL;
	tjgInit(lpGroup, TASK_COUNT_OF_GROUP);
	alAddElement(&tjgl->m_Groups, lpGroup);
	tjgSetIndex(lpGroup, i+1);
	// lpGroup->m_bValid= 1; 未加入事件前不设置为有效
	return lpGroup;
}

// =================================================================================
// CTaskGroup
void tgInit(CTaskGroup* tg)
{
	memset(tgGetHeadBuffer(tg), 1, tgGetHeadBufferSize());
	tgSetStartDate(tg, 1, 1);
	tgSetEndDate(tg, 12, 31);
}

int	tgGetHeadBufferSize()
{
	return sizeof(_TaskGroupTag);
}

BYTE* tgGetHeadBuffer(CTaskGroup* tg)
{
	return (BYTE *)&tg->m_Head;
}

void tgAssign(CTaskGroup* tg, CTaskGroup *lpGroup)
{
	if(lpGroup==NULL) return ;

	memcpy(tgGetHeadBuffer(tg), tgGetHeadBuffer(lpGroup), tgGetHeadBufferSize());
}

u8	tgGetGroupOfWeek(CTaskGroup* tg, WEEKDAY nWeek)
{
	return tg->m_Head.byTaskGroupOfWeek[nWeek];
}

u8	tgGetGroupOfDate(CTaskGroup* tg, PMyDate tmDate)
{
	return tg->m_Head.byTaskGroupOfWeek[MyDateOfWeek(tmDate)-1];
}

u8 tgGetSpecialGroup(CTaskGroup* tg)
{
	return tg->m_Head.bySpecialGroup;
}

PShortDate tgGetStartDate(CTaskGroup* tg)
{
	return &tg->m_Head.srtStartDate;
}

PShortDate tgGetEndDate(CTaskGroup* tg)
{
	return &tg->m_Head.srtEndDate;
}

void tgSetGroupOfWeek(CTaskGroup* tg, WEEKDAY nWeek, CTaskJobGroup *lpJobGroup)
{
	if(lpJobGroup)
		tg->m_Head.byTaskGroupOfWeek[nWeek]= tjgGetIndex(lpJobGroup);
	else
		tg->m_Head.byTaskGroupOfWeek[nWeek]= 0;
}

void tgSetSpecialGorup(CTaskGroup* tg, CTaskJobGroup *lpGroup)
{
	if(lpGroup)
		tg->m_Head.bySpecialGroup= tjgGetIndex(lpGroup);
	else
		tg->m_Head.bySpecialGroup= 0;
}

void		tgSetStartDate(CTaskGroup* tg, BYTE byMonth, BYTE byDay)
{
	tg->m_Head.srtStartDate.byMonth= byMonth;
	tg->m_Head.srtStartDate.byDay= byDay;
}

void		tgSetEndDate(CTaskGroup* tg, BYTE byMonth, BYTE byDay)
{
	tg->m_Head.srtEndDate.byMonth= byMonth;
	tg->m_Head.srtEndDate.byDay= byDay;
}

void			tgSetStartMyDate(CTaskGroup* tg, PShortDate lpdate)
{
	tg->m_Head.srtStartDate.byMonth= lpdate->byMonth;
	tg->m_Head.srtStartDate.byDay= lpdate->byDay;
}

void			tgSetEndMyDate(CTaskGroup* tg, PShortDate lpdate)
{
	tg->m_Head.srtEndDate.byMonth= lpdate->byMonth;
	tg->m_Head.srtEndDate.byDay= lpdate->byDay;
}

BOOL tgValidDate(CTaskGroup* tg, PShortDate lpdate)
{
	u32 dwStart= ShortDateOfDays(tgGetStartDate(tg)),	
		dwEnd= ShortDateOfDays(tgGetEndDate(tg)),
		dwCur= ShortDateOfDays(lpdate);
	return (BOOL)( (dwCur>=dwStart)&&(dwCur<=dwEnd));
}

// ==========================================================================
// CTaskGroupList
void tglInit(CTaskGroupList *tgl, int nMaxCount)
{
	tgl->m_nMaxCount= nMaxCount;
	alInit(&tgl->m_Groups, tglCompareDate);
}

int		tglGetMaxCount(CTaskGroupList *tgl)
{
	return tgl->m_nMaxCount;
}

int		tglGetCount(CTaskGroupList *tgl)
{
	return tgl->m_Groups.size;
}

BOOL	tglCanAddNew(CTaskGroupList *tgl)
{
	return (BOOL)(tglGetCount(tgl)<tgl->m_nMaxCount);
}


int tglCompareDate(Element elm1, Element elm2)
{
	CTaskGroup *lpItem1=(CTaskGroup *)elm1,  *lpItem2= (CTaskGroup *)elm2;

	u32 tm1, tm2;
	tm1= ShortDateOfDays(tgGetStartDate(lpItem1));
	tm2= ShortDateOfDays(tgGetStartDate(lpItem2));
	if(tm1==tm2)
		return 0;
	else
		if(tm1<tm2)
			return -1;
		else
			return 1;
}

CTaskGroup*		tglGetAt(CTaskGroupList *tgl, int index)
{
	return (CTaskGroup *)alGetElement(&tgl->m_Groups, index);
}

void			tglSort(CTaskGroupList *tgl)
{
	alSort(&tgl->m_Groups);
}

int				tglAdd(CTaskGroupList *tgl, CTaskGroup *lpGroup)
{
	int index= tglIndexOf(tgl, lpGroup);
	if(index!=-1) return -1;
	return alAddElement(&tgl->m_Groups, lpGroup);
}

int				tglIndexOf(CTaskGroupList *tgl, CTaskGroup *lpGroup)
{
	return alIndexof(&tgl->m_Groups, lpGroup);
}

void			tglDeleteIndex(CTaskGroupList *tgl, int index)
{
	alRemoveIndex(&tgl->m_Groups, index);
}

void			tglDeleteGroup(CTaskGroupList *tgl, CTaskGroup *lpGroup)
{
	alRemoveElement(&tgl->m_Groups, lpGroup);
}

void			tglClear(CTaskGroupList *tgl)
{
	alClear(&tgl->m_Groups);
}

CTaskGroup *tglNew(CTaskGroupList *tgl)
{
	CTaskGroup *lpGroup;
	if(!tglCanAddNew(tgl)) return NULL;
	lpGroup= &tgl->Groups[tgl->m_Groups.size];
	alAddElement(&tgl->m_Groups, lpGroup);
	return lpGroup;
}

CTaskGroup *tglGetTaskGroup(CTaskGroupList *tgl, PMyDate tm)
{
	int i;
	CTaskGroup *lpGroup;
	for(i=0;i<tglGetCount(tgl);i++)
	{
		lpGroup= tglGetAt(tgl, i);
		if(tgValidDate(lpGroup, (PShortDate)&tm->byMonth))
		{
			return lpGroup;
		}
	}
	return NULL;
}

// =====================================================================
// CIntercutTask
void itInit(CIntercutTask *it, PMyDate tm)
{
	itSetTaskMyDate(it, tm);
	tjInit(&it->m_Job);
	it->m_Job.m_Head.byTimeType= tttIntercut;
}

int		itGetDateBufferSize()
{
	return sizeof(TMyDate);
}

BYTE *  itGetDateBuffer(CIntercutTask *it)
{
	return (BYTE *)&it->m_Date;
}

void	itAssign(CIntercutTask *it, CIntercutTask *lpTask)
{
	if(lpTask==NULL) return ;

	itSetTaskMyDate(it, itGetTaskDate(lpTask));
	tjAssign(&it->m_Job, itGetTaskJob(lpTask));
}

CTaskJob	*itGetTaskJob(CIntercutTask *it)
{
	return &it->m_Job;
}

void		itSetTaskJob(CIntercutTask *it, CTaskJob *lpJob)
{
	tjAssign(&it->m_Job, lpJob);
}

PMyDate itGetTaskDate(CIntercutTask *it)
{
	return &it->m_Date;
}

void	itSetTaskMyDate(CIntercutTask *it, PMyDate lpdate)
{
	it->m_Date.wYear= lpdate->wYear;
	it->m_Date.byMonth= lpdate->byMonth;
	it->m_Date.byDay= lpdate->byDay;
}

void	itSetTaskDate(CIntercutTask *it, WORD wYear, WORD wMonth, WORD wDay)
{
	it->m_Date.wYear= wYear;
	it->m_Date.byMonth= (BYTE)wMonth;
	it->m_Date.byDay= (BYTE)wDay;
}

BOOL itValidDate(CIntercutTask *it, PMyDate lpdate)
{
	u32	tm1= MyDateOfDays(itGetTaskDate(it)),
		tm2= MyDateOfDays(lpdate);
	return (BOOL)(tm1==tm2);
}

// =========================================================================
// CIntercutTaskGroup
void itgInit(CIntercutTaskGroup *itg, int nMaxCount/* =TASK_COUNT_OF_GROUP */)
{
	itg->m_nMaxCount= nMaxCount;
	alInit(&itg->m_Tasks, itgCompareDate);
}

int		itgGetCount(CIntercutTaskGroup *itg)
{
	return itg->m_Tasks.size;
}

BOOL	itgCanAddNew(CIntercutTaskGroup *itg)
{
	return (BOOL)(itgGetCount(itg)<itg->m_nMaxCount);
}

int itgCompareDate(Element elm1, Element elm2)
{
	CIntercutTask* lpItem1= (CIntercutTask *)elm1, 
				*lpItem2= (CIntercutTask *)elm2;
	u32 tm1, tm2;
	tm1= MyDateOfDays(itGetTaskDate(lpItem1));
	tm2= MyDateOfDays(itGetTaskDate(lpItem2));
	if(tm1==tm2)
		return 0;
	else
		if(tm1>tm2)
			return 1;
		else
			return -1;
}

CIntercutTask	*itgGetAt(CIntercutTaskGroup *itg, int index)
{
	return (CIntercutTask *)alGetElement(&itg->m_Tasks, index);
}
int				itgAdd(CIntercutTaskGroup *itg, CIntercutTask *lpTask)
{
	int index= itgIndexOf(itg, lpTask);
	if(index!=-1) return -1;
	return alAddElement(&itg->m_Tasks, lpTask);
}

int				itgIndexOf(CIntercutTaskGroup *itg, CIntercutTask *lpTask)
{
	return alIndexof(&itg->m_Tasks, lpTask);
}

void			itgDeleteIndex(CIntercutTaskGroup *itg, int index)
{
	alRemoveIndex(&itg->m_Tasks, index);
}

void			itgDeleteTask(CIntercutTaskGroup *itg, CIntercutTask *lpTask)
{
	alRemoveElement(&itg->m_Tasks, lpTask);
}

void			itgClear(CIntercutTaskGroup *itg)
{
	alClear(&itg->m_Tasks);
}

void itgSort(CIntercutTaskGroup *itg)
{
	alSort(&itg->m_Tasks);
}

CIntercutTask *itgNew(CIntercutTaskGroup *itg)
{
	CIntercutTask *lpTask;
	if(!itgCanAddNew(itg)) return NULL;
	lpTask= &itg->its[itg->m_Tasks.size];
	alAddElement(&itg->m_Tasks, lpTask);
	return lpTask;
}
// =======================================================================
#if (GC_ZR & GC_ZR_REMOTE)
// CRemoteTask
void rtInit(CRemoteTask *rt, TTaskTimeType type)
{
	rt->m_Key = 0;
	rt->m_funcValue = 0;
	tjInit(&rt->m_Job);
	rt->m_Job.m_Head.byTimeType = type;
	rt->m_Job.m_Head.wPlayLength = 300;
}

void rtAssign(CRemoteTask *rt, CRemoteTask *lpTask)
{
	if (lpTask == NULL) return;

	rt->m_Key = lpTask->m_Key;
	rt->m_funcValue = lpTask->m_funcValue;
	tjAssign(&rt->m_Job, rtGetTaskJob(lpTask));
}

CTaskJob *rtGetTaskJob(CRemoteTask *rt)
{
	return &rt->m_Job;
}

void rtSetTaskJob(CRemoteTask *rt, CTaskJob *lpJob)
{
	tjAssign(&rt->m_Job, lpJob);
}

int rtGetTaskKey(CRemoteTask *rt)
{
	return rt->m_Key;
}

void rtSetTaskKey(CRemoteTask *rt, int key)
{
	rt->m_Key = key;
}

int rtGetTaskFuncValue(CRemoteTask *rt)
{
	return rt->m_funcValue;
}

void rtSetTaskFuncValue(CRemoteTask *rt, int funcValue)
{
	rt->m_funcValue = funcValue;
}

BOOL rtGetTaskType(CRemoteTask *rt)
{
	return ((rt->m_funcValue > 5)? TRUE: FALSE);
}

// =======================================================================
// CRemoteTaskGroup
void rtgInit(CRemoteTaskGroup *rtg, int nMaxCount/* =TASK_COUNT_OF_GROUP */)
{
	rtg->m_nMaxCount = nMaxCount;
	alInit(&rtg->m_Tasks, rtgCompareDate);
}

int rtgGetCount(CRemoteTaskGroup *rtg)
{
	return rtg->m_Tasks.size;
}

BOOL rtgCanAddNew(CRemoteTaskGroup *rtg)
{
	return (BOOL)(rtgGetCount(rtg)<rtg->m_nMaxCount);
}

int rtgCompareDate(Element elm1, Element elm2)
{
	CRemoteTask* lpItem1 = (CRemoteTask *)elm1,
		*lpItem2 = (CRemoteTask *)elm2;
	u32 tm1, tm2;
	tm1 = rtGetTaskKey(lpItem1);
	tm2 = rtGetTaskKey(lpItem2);
	if (tm1 == tm2)
		return 0;
	else
	if (tm1>tm2)
		return 1;
	else
		return -1;
}

CRemoteTask	*rtgGetAt(CRemoteTaskGroup *rtg, int index)
{
	return (CRemoteTask *)alGetElement(&rtg->m_Tasks, index);
}

int rtgAdd(CRemoteTaskGroup *rtg, CRemoteTask *lpTask)
{
	int index = rtgIndexOf(rtg, lpTask);
	if (index != -1) return -1;
	return alAddElement(&rtg->m_Tasks, lpTask);
}

int	rtgIndexOf(CRemoteTaskGroup *rtg, CRemoteTask *lpTask)
{
	return alIndexof(&rtg->m_Tasks, lpTask);
}

void rtgDeleteIndex(CRemoteTaskGroup *rtg, int index)
{
	alRemoveIndex(&rtg->m_Tasks, index);
}

void rtgDeleteTask(CRemoteTaskGroup *rtg, CRemoteTask *lpTask)
{
	alRemoveElement(&rtg->m_Tasks, lpTask);
}

void rtgClear(CRemoteTaskGroup *rtg)
{
	alClear(&rtg->m_Tasks);
}

void rtgSort(CRemoteTaskGroup *rtg)
{
	alSort(&rtg->m_Tasks);
}

CRemoteTask *rtgNew(CRemoteTaskGroup *rtg)
{
	CRemoteTask *lpTask;
	if (!rtgCanAddNew(rtg)) return NULL;
	lpTask = &rtg->its[rtg->m_Tasks.size];
	alAddElement(&rtg->m_Tasks, lpTask);
	return lpTask;
}
// =======================================================================
#endif
// CSpecialMode
void smInit(CSpecialMode* sm)
{
	sm->m_bySpecMode= SPECIALMODE_INVALID;
	sm->m_bySpecDayCount= 1;
}

PMyDate smGetSpecialDate(CSpecialMode* sm)
{
	return &sm->m_SpecialDate;
}

void smSetSpecialDate(CSpecialMode* sm, PMyDate lpdate)
{
	sm->m_SpecialDate.wYear= lpdate->wYear;
	sm->m_SpecialDate.byMonth= lpdate->byMonth;
	sm->m_SpecialDate.byDay= lpdate->byDay;
}

BOOL smValidDate(CSpecialMode* sm, PMyDate tm)
{
	u32 tm1, tm2;
	int nDays, nStartDays, nEndDays;
	if(sm->m_bySpecMode)		// 特殊任务已开启
	{
		tm1= MyDateOfDays(&sm->m_SpecialDate);
		tm2= MyDateOfDays(tm);
		nDays= tm2-tm1;
		if(nDays>=0)
		{
			nStartDays= sm->m_bySpecMode-1;
			nEndDays= sm->m_bySpecMode-1+sm->m_bySpecDayCount;
			if((nDays>=nStartDays)&&(nDays<nEndDays))
				return TRUE;
		} 
	}
	return FALSE;
}

void smResetData(CSpecialMode* sm)
{
	sm->m_bySpecMode= SPECIALMODE_INVALID;
}

// ========================================================================================
// CTodayTask
void ttInit(CTodayTask *tt)
{
	tjgInit(&tt->m_lpTodayTask, 255);

	tt->m_nAmpPwrPreOpen= 20;			// 功放提前打开时间 秒
	// tt->m_iNextTask=-1;					// 即将播放或当前正在播放的任务编号
	tt->m_tsTaskState=tsTaskInvalid;			// 当前任务状态
}

int ttGetCount(CTodayTask* tt)
{
	return tjgGetCount(&tt->m_lpTodayTask);
}

// 返回今日任务总数
CTaskJobGroup *ttGetTaskGroup(CTodayTask* tt)
{
	return &tt->m_lpTodayTask;
}		// 返回今日任务组指针

void ttClear(CTodayTask* tt)
{
	tjgClear(&tt->m_lpTodayTask);					// 清除所有数据
}

// CTodayTask 成员函数
TTaskState ttTaskState(CTodayTask* tt, int nCurSecond, int index)		// 返回当前任务状态
{
	TTaskState tsRes;
	int nSecSpan;
	CTaskJob *lpJob= tjgGetAt(&tt->m_lpTodayTask, index);
	u32 nStartSec= MyTimeToSecond(tjGetStartTime(lpJob)), nEndSec= nStartSec+tjGetPlayLength(lpJob), nAmpOpenSec=(nStartSec-tt->m_nAmpPwrPreOpen);
	tsRes= tsTaskInvalid;
	if((tt->m_iNextTask==index)&&(nCurSecond<nAmpOpenSec)&&(tt->m_JobStates[index]<tsTaskReady))	// 当前时间小于功放打开时间， 任务准备期间
	{
		tsRes= tsTaskReady;
		tt->m_tsTaskState= tsRes;
		tt->m_JobStates[index]= tsRes;
		tt->m_iNextTask= index;
		return tsRes;
	}
	if(nCurSecond<nAmpOpenSec) return tsRes;
	if((nCurSecond>=nAmpOpenSec)&&(nCurSecond<nStartSec)&&(tt->m_JobStates[index]<tsTaskStart)) // 任务准备好(打开功放)，开始播放前
	{
		tsRes= tsTaskStart;
		tt->m_tsTaskState= tsRes;
		tt->m_JobStates[index]= tsRes;
		return tsRes;
	}
	nSecSpan=nCurSecond-nStartSec;
	if((nSecSpan>=0)&&(nSecSpan<=5)&&(tt->m_JobStates[index]<tsTaskRunning))  // 任务正式播放到任务结束期间
	{
		if(tt->m_iNextTask<index)
		{
			// tt->m_tsTaskState= tsTaskDone;
			tt->m_iNextTask= index;
		}
		tsRes= tsTaskRunning;
		tt->m_tsTaskState= tsRes;
		tt->m_JobStates[index]= tsRes;
		return tsRes;
	}
	if((nCurSecond>= nEndSec)&&(tt->m_JobStates[index]==tsTaskRunning)&&(tt->m_iNextTask>-1))  // 本次任务已结束
	{
		tt->m_iNextTask++;
		tsRes= tsTaskDone;
		tt->m_tsTaskState= tsRes;
		tt->m_JobStates[index]=tsRes;
	}
	return tsRes;
}

TTaskState ttGetTaskState(CTodayTask* tt)		// 返回当前任务状态
{
	return tt->m_tsTaskState;
}

TTaskState ttGetJobState(CTodayTask* tt, u8 index)		// 返回播音事件状态
{
	return tt->m_JobStates[index];
}

void ttUpdateTaskState(CTodayTask* tt, PMyTime tm)
{
	int nSec, i, nStartTime;
	CTaskJob *lpJob;
	tt->m_iNextTask= -1;
	nSec= MyTimeToSecond(tm);
	tt->m_tsTaskState= tsTaskInvalid;
	for(i=0;i<TASK_COUNT_OF_GROUP;i++)
		tt->m_JobStates[i]= tsTaskInvalid;
	for( i=0;i<ttGetCount(tt);i++)
	{
		lpJob= tjgGetAt(&tt->m_lpTodayTask, i);
		nStartTime= MyTimeToSecond(tjGetStartTime(lpJob));
		if((nStartTime>nSec+5)&&(tt->m_iNextTask==-1))
		{
			tt->m_iNextTask= i;
			tt->m_tsTaskState= tsTaskReady;
			return;
		}
	}
	tt->m_tsTaskState= tsTaskDone;
}

CTaskJob *ttGetNextTask(CTodayTask* tt)				// 返回当前时间的任务
{
	if((tt->m_iNextTask<0)||(tt->m_iNextTask>=ttGetCount(tt)))
		return NULL;
	return tjgGetAt(&tt->m_lpTodayTask, tt->m_iNextTask);
}

// ************************************************************************************************************
// 返回当前时间的任务编号
// -1：任务执行完毕； 0：今日无任务；大于0：任务编号
int ttGetNextTaskID(CTodayTask* tt)				
{
	if(ttGetCount(tt)==0)
		return 0;
	if(tt->m_iNextTask==-1)
		return -1;
	return tt->m_iNextTask+1;
}

/*
DWORD CALLBACK ttTaskRunThreadProc(PVOID pArg)
{
	CTodayTask	*pSC=(CTodayTask *)pArg;
	TTaskState tsTmp;
	int iNextTask, i;
	CTime tm;
	static int nOldSencond=-1;
	while(pSC->m_bThreadRuning)
	{
		switch(WaitForSingleObject(pSC->m_hTaskRunThread, 250))
		{
		case WAIT_TIMEOUT:
			//SuspendThread(pSC->m_hTaskRunThread);
			// 获取任务状态，并执行之
			tm=CTime::GetCurrentTime();
			i= tm.GetSecond();
			if(i==nOldSencond) break;
			nOldSencond= i;
			pSC->m_nHour= tm.GetHour();
			pSC->m_nMinute= tm.GetMinute();
			pSC->m_nSecond= i;
			pSC->m_nCurSecond= pSC->m_nHour*3600+pSC->m_nMinute*60+pSC->m_nSecond;
			if(pSC->m_bTaskRuning==FALSE) break;
			if(pSC->m_iNextTask>-1)
				iNextTask= pSC->m_iNextTask;
			else
				iNextTask= 0;
			for(i=iNextTask;i<pSC->GetCount();i++)
			{ 
				tsTmp= pSC->TaskState(pSC->m_nCurSecond, i);
				if((tsTmp!=tsTaskInvalid)&&(pSC->m_pbyJobStates[i]!=tsTmp))
				{
					pSC->m_pbyJobStates[i]= tsTmp;
					if(pSC->m_fpOnTaskRun)
						pSC->m_fpOnTaskRun(pSC->m_lpCaller, pSC->m_lpTodayTask->GetAt(i), i+1, tsTmp);
				}
			}
			//if(pSC->m_iNextTask>=pSC->GetCount())		// 最后一条
			//{
			//	pSC->m_iNextTask++;			// 全天任务结束
			//	if(pSC->m_fpOnTaskRun)
			//		pSC->m_fpOnTaskRun(pSC->m_lpCaller, pSC->m_lpTodayTask->GetAt(pSC->m_iNextTask), pSC->m_iNextTask, tsTaskDone);
			//}
			//ResumeThread(pSC->m_hTaskRunThread);
			break;
		case WAIT_OBJECT_0:
		case WAIT_FAILED:
			return 0;
		}
	}	// while(bPlayThreadContinue && (pSC->m_psPlayerState>psNotready))
	return 1;
}
*/

// ============================================================================
// CTaskManager

void tmInit(CTaskManager* tm )
{
	memset(tm, 0, sizeof(CTaskManager));
	tjglInit(&tm->m_lpJobGroups, TASKGROUP_COUNT_OFLIST);
	tglInit(&tm->m_lpTaskGroups, TASKGROUP_COUNT_OFLIST);
	itgInit(&tm->m_lpIntercutTasks, TASK_COUNT_OF_GROUP);
#if (GC_ZR & GC_ZR_REMOTE)
	rtgInit(&tm->m_lpYkTasks, TASK_COUNT_OF_GROUP);
	rtgInit(&tm->m_lpPhoneTasks, TASK_COUNT_OF_GROUP);
#endif
	ttInit(&tm->m_lpTodayTask);
	smInit(&tm->m_lpSpecMode);
}

CTaskJobGroup*	tmGetGroupOfWeek(CTaskManager *tm, CTaskGroup* tg, WEEKDAY nWeek)
{
	CTaskJobGroupList *lpGroups=  &tm->m_lpJobGroups;
	return tjglGetJobGroup(lpGroups, tg->m_Head.byTaskGroupOfWeek[nWeek]);
}
CTaskJobGroup*	tmGetGroupOfDate(CTaskManager *tm, CTaskGroup* tg, PMyDate tmDate)
{
	CTaskJobGroupList *lpGroups=  &tm->m_lpJobGroups;
	return tjglGetJobGroup(lpGroups, tg->m_Head.byTaskGroupOfWeek[MyDateOfWeek(tmDate)-1]);
}
CTaskJobGroup*	tmGetSpecialGroup(CTaskManager *tm, CTaskGroup* tg)
{
	CTaskJobGroupList *lpGroups=  &tm->m_lpJobGroups;
	return tjglGetJobGroup(lpGroups, tg->m_Head.bySpecialGroup);
}

// CTaskManager 成员函数
void tmResetTaskData(CTaskManager* tm)
{
	tm->m_lpSpecMode.m_bySpecMode= SPECIALMODE_INVALID;
	ttClear(&tm->m_lpTodayTask);
	itgClear(&tm->m_lpIntercutTasks);
#if (GC_ZR & GC_ZR_REMOTE)
	rtgClear(&tm->m_lpYkTasks);
	rtgClear(&tm->m_lpPhoneTasks);
#endif
	tglClear(&tm->m_lpTaskGroups);
	tjglClear(&tm->m_lpJobGroups);
	tjglInit(&tm->m_lpJobGroups, TASKGROUP_COUNT_OFLIST);
}

#define LPVOID	void *
BOOL tmLoadHeaderFromFile(CTaskManager *tm, FIL * hFil)
{
	UINT	dwBuf[2]={0,0}, dwReadCount;
	BOOL bRes=(BOOL)(f_read(hFil, (LPVOID)dwBuf, sizeof(dwBuf), &dwReadCount)==FR_OK);
	return (BOOL)(bRes && (dwBuf[0]==TASK_HEAD_STR) && (dwBuf[1]==TASK_VERSION));
}

void tmLoadJobFromFile(CTaskManager *tm, FIL * hFil, CTaskJob *lpJob)
{
	UINT	dwReadCount;
	f_read(hFil, (LPVOID)tjGetHeadBuffer(lpJob), tjGetHeadBufferSize(), &dwReadCount);
	f_read(hFil, (LPVOID)&lpJob->mPortState, sizeof(lpJob->mPortState), &dwReadCount);
}

void tmLoadTaskGroupFromFile(CTaskManager *tm, FIL * hFil, CTaskGroup *lpGroup)
{
	UINT	dwReadCount;
	f_read(hFil, (LPVOID)tgGetHeadBuffer(lpGroup), tgGetHeadBufferSize(), &dwReadCount);
}

void tmLoadJobTaskFromFile(CTaskManager *tm, FIL * hFil)
{
	UINT	dwReadCount;
	BYTE	byGroupCount, byGroupID, byJobCount;
	CTaskJobGroup *lpGroup;
	// tjglClear(&tm->m_lpJobGroups);
	if(f_read(hFil, (LPVOID)&byGroupCount, 1, &dwReadCount)==FR_OK)
	{
		int i;
		for(i=0;i<byGroupCount;i++)
		{
			if(f_read(hFil, (LPVOID)&byGroupID, 1, &dwReadCount)==FR_OK)
			{
				lpGroup= tjglGetJobGroup(&tm->m_lpJobGroups, byGroupID);// tjglNew(&tm->m_lpJobGroups);
				tjgSetIndex(lpGroup, byGroupID);
				lpGroup->m_bValid= 1;
				if(f_read(hFil, (LPVOID)&byJobCount, 1, &dwReadCount)==FR_OK)
				{
					int j;
					for(j=0;j<byJobCount;j++)
					{
						CTaskJob *lpJob= tjgNew(lpGroup);
						tmLoadJobFromFile(tm, hFil, lpJob);
					}
				}			
			}
		}
	}
	
}

void tmLoadTaskGroupsFromFile(CTaskManager *tm, FIL * hFil)
{
	UINT	dwReadCount;
	BYTE	byGroupCount;
	tglClear(&tm->m_lpTaskGroups);
	if(f_read(hFil, (LPVOID)&byGroupCount, 1, &dwReadCount)==FR_OK)
	{
		int i;
		for(i=0;i<byGroupCount;i++)
		{
			CTaskGroup *lpGroup= tglNew(&tm->m_lpTaskGroups);
			tmLoadTaskGroupFromFile(tm, hFil, lpGroup);
		}
	}
}

void tmLoadIntercutTaskFromFile(CTaskManager *tm, FIL * hFil)
{
	UINT	dwReadCount;
	BYTE	byJobCount;
	itgClear(&tm->m_lpIntercutTasks);
	if(f_read(hFil, (LPVOID)&byJobCount, 1, &dwReadCount)==FR_OK)
	{
		int j;
		for(j=0;j<byJobCount;j++)
		{
			CIntercutTask *lpInter= itgNew(&tm->m_lpIntercutTasks);
			if(f_read(hFil, itGetDateBuffer(lpInter), itGetDateBufferSize(), &dwReadCount)==FR_OK)
			{
				CTaskJob *lpJob= itGetTaskJob(lpInter);
				tmLoadJobFromFile(tm, hFil, lpJob);
			}
		}
	}			
}

#if (GC_ZR &GC_ZR_REMOTE)
void tmLoadYkTaskFromFile(CTaskManager *tm, FIL * hFil) {
	UINT	dwReadCount;
	BYTE	byJobCount;
	rtgClear(&tm->m_lpYkTasks);
	if (f_read(hFil, (LPVOID)&byJobCount, 1, &dwReadCount) == FR_OK)
	{
		int j;
		for (j = 0; j<byJobCount; j++)
		{
			CRemoteTask *lpYk = rtgNew(&tm->m_lpYkTasks);
			if (f_read(hFil, (LPVOID)&lpYk->m_Key, 4, &dwReadCount) == FR_OK) {
				if (f_read(hFil, (LPVOID)&lpYk->m_funcValue, 4, &dwReadCount) == FR_OK) {
					CTaskJob *lpJob = rtGetTaskJob(lpYk);
					tmLoadJobFromFile(tm, hFil, lpJob);
				}
			}
		}
	}
}

void tmLoadPhoneTaskFromFile(CTaskManager *tm, FIL * hFil) {
	UINT	dwReadCount;
	BYTE	byJobCount;
	rtgClear(&tm->m_lpPhoneTasks);
	if (f_read(hFil, (LPVOID)&byJobCount, 1, &dwReadCount) == FR_OK)
	{
		int j;
		for (j = 0; j<byJobCount; j++)
		{
			CRemoteTask *lpPhone = rtgNew(&tm->m_lpPhoneTasks);
			if (f_read(hFil, (LPVOID)&lpPhone->m_Key, 4, &dwReadCount) == FR_OK) {
				if (f_read(hFil, (LPVOID)&lpPhone->m_funcValue, 4, &dwReadCount) == FR_OK) {
					CTaskJob *lpJob = rtGetTaskJob(lpPhone);
					tmLoadJobFromFile(tm, hFil, lpJob);
				}
			}
		}
	}
}
#endif

void tmLoadSpecModeFromFile(CTaskManager *tm, FIL * hFil)
{
	UINT	dwReadCount;
	if(f_read(hFil, (LPVOID)&tm->m_lpSpecMode, sizeof(CSpecialMode), &dwReadCount)==FR_OK)
		if(dwReadCount==sizeof(CSpecialMode)) return;
	memset(&tm->m_lpSpecMode, 0, sizeof(CSpecialMode));
}

BOOL tmLoadFromFile(CTaskManager *tm, const char* strFileName)
{
	BOOL bRes;

	FIL hFil;
	if( f_open(&hFil, strFileName, FA_READ )!=FR_OK)
	{
		// AfxMessageBox(_T("打开任务文件失败！"));
		return FALSE;
	}
	bRes= tmLoadHeaderFromFile(tm, &hFil);
	if(bRes)
	{
		tmLoadJobTaskFromFile(tm, &hFil);
		tmLoadTaskGroupsFromFile(tm, &hFil);
		tmLoadIntercutTaskFromFile(tm, &hFil);
#if (GC_ZR & GC_ZR_REMOTE)
		tmLoadYkTaskFromFile(tm, &hFil);
		tmLoadPhoneTaskFromFile(tm, &hFil);
#endif
		tmLoadSpecModeFromFile(tm, &hFil);
	}

	f_close(&hFil);
	return bRes;
}

BOOL tmSaveHeaderToFile(CTaskManager *tm, FIL * hFil)
{
	UINT	dwBuf[2]={TASK_HEAD_STR, TASK_VERSION}, dwWriteCount;
	FRESULT bRes= f_write(hFil, (LPVOID)dwBuf, sizeof(UINT)*2, &dwWriteCount);
	return (BOOL)((bRes==FR_OK) && (dwWriteCount==sizeof(UINT)*2));
}

BOOL tmSaveJobToFile(CTaskManager *tm, FIL * hFil, CTaskJob *lpJob)
{
	UINT dwWriteCount;
	f_write(hFil, (LPVOID)tjGetHeadBuffer(lpJob), tjGetHeadBufferSize(), &dwWriteCount);
	return (BOOL)(f_write(hFil, (LPVOID)&lpJob->mPortState, sizeof(lpJob->mPortState), &dwWriteCount)==FR_OK);
}

BOOL tmSaveTaskGroupToFile(CTaskManager *tm, FIL * hFil, CTaskGroup *lpGroup)
{
	UINT dwWriteCount;
	return (BOOL)(f_write(hFil, (LPVOID)tgGetHeadBuffer(lpGroup), tgGetHeadBufferSize(), &dwWriteCount)==FR_OK);
}

BOOL tmSaveJobTaskToFile(CTaskManager *tm, FIL * hFil)
{
	UINT dwWriteCount;
	BYTE  byGroupCount, byJobCount;
	FRESULT bRes;
	int i, j;
	byGroupCount= (BYTE)tjglGetCount(&tm->m_lpJobGroups);
	bRes= f_write(hFil, (LPVOID)&byGroupCount, 1, &dwWriteCount);
	if(bRes==FR_OK)
	{
		for(i=0;i<byGroupCount;i++)
		{
			CTaskJobGroup *lpGroup= tjglGetAt(&tm->m_lpJobGroups, i);
			BYTE byGroupID= (BYTE)tjgGetIndex(lpGroup);
			tjgSort(lpGroup);
			bRes= f_write(hFil, (LPVOID)&byGroupID, 1, &dwWriteCount);
			if(bRes==FR_OK)
			{
				byJobCount= tjgGetCount(lpGroup);
				bRes= f_write(hFil, (LPVOID)&byJobCount, 1, &dwWriteCount);
				if(bRes==FR_OK)
				{
					for(j=0;j<byJobCount;j++)
					{
						CTaskJob *lpJob= tjgGetAt(lpGroup, j);
						bRes= tmSaveJobToFile(tm, hFil, lpJob)?FR_OK:FR_TIMEOUT;
						if(bRes!=FR_OK)
							break;
					}
				}
			}
			if(bRes!=FR_OK)
				break;
		}
	}
	return (BOOL)(bRes==FR_OK);
}

BOOL tmSaveTaskGroupsToFile(CTaskManager *tm, FIL * hFil)
{
	UINT dwWriteCount;
	BYTE  byGroupCount;
	BOOL bRes;
	byGroupCount= (BYTE)tglGetCount(&tm->m_lpTaskGroups);
	bRes=(BOOL)(f_write(hFil, (LPVOID)&byGroupCount, 1, &dwWriteCount)==FR_OK);
	if(bRes)
	{
		int i;
		for(i=0;i<byGroupCount;i++)
		{
			CTaskGroup *lpGroup= tglGetAt(&tm->m_lpTaskGroups, i);
			bRes= tmSaveTaskGroupToFile(tm, hFil, lpGroup);
			if(!bRes)
				break;
		}
	}
	return bRes;
}

BOOL tmSaveIntercutTaskFromFile(CTaskManager *tm, FIL * hFil)
{
	UINT dwWriteCount;
	BYTE byJobCount= itgGetCount(&tm->m_lpIntercutTasks);
	BOOL bRes= (BOOL)(f_write(hFil, (LPVOID)&byJobCount, 1, &dwWriteCount)==FR_OK);
	if(bRes)
	{
		int j;
		for(j=0;j<byJobCount;j++)
		{
			CIntercutTask *lpInter= itgGetAt(&tm->m_lpIntercutTasks, j);
			bRes= (BOOL)(f_write(hFil, itGetDateBuffer(lpInter), itGetDateBufferSize(), &dwWriteCount)==FR_OK);
			if(bRes)
			{
				CTaskJob *lpJob= itGetTaskJob(lpInter);
				bRes= tmSaveJobToFile(tm, hFil, lpJob);
			}
			if(!bRes)
				break;
		}
	}
	return bRes;
}

#if (GC_ZR & GC_ZR_REMOTE)
BOOL mSaveYkTaskFromFile(CTaskManager *tm, FIL * hFil) {	
	UINT dwWriteCount;
	BYTE byJobCount= rtgGetCount(&tm->m_lpYkTasks);
	BOOL bRes= (BOOL)(f_write(hFil, (LPVOID)&byJobCount, 1, &dwWriteCount)==FR_OK);
	if(bRes)
	{
		int j;
		for(j=0;j<byJobCount;j++)
		{
			CRemoteTask *lpYk= rtgGetAt(&tm->m_lpYkTasks, j);
			bRes= (BOOL)(f_write(hFil, (LPVOID)&lpYk->m_Key, 4, &dwWriteCount)==FR_OK);
			if (!bRes)
				break;
			bRes = (BOOL)(f_write(hFil, (LPVOID)&lpYk->m_funcValue, 4, &dwWriteCount) == FR_OK);
			if(bRes)
			{
				CTaskJob *lpJob= rtGetTaskJob(lpYk);
				bRes= tmSaveJobToFile(tm, hFil, lpJob);
			}
			if(!bRes)
				break;
		}
	}
	return bRes;
}

BOOL mSavePhoneTaskFromFile(CTaskManager *tm, FIL * hFil) {
	UINT dwWriteCount;
	BYTE byJobCount = rtgGetCount(&tm->m_lpPhoneTasks);
	BOOL bRes = (BOOL)(f_write(hFil, (LPVOID)&byJobCount, 1, &dwWriteCount) == FR_OK);
	if (bRes)
	{
		int j;
		for (j = 0; j<byJobCount; j++)
		{
			CRemoteTask *lpPhone = rtgGetAt(&tm->m_lpPhoneTasks, j);
			bRes = (BOOL)(f_write(hFil, (LPVOID)&lpPhone->m_Key, 4, &dwWriteCount) == FR_OK);
			if (!bRes)
				break;
			bRes = (BOOL)(f_write(hFil, (LPVOID)&lpPhone->m_funcValue, 4, &dwWriteCount) == FR_OK);
			if (bRes)
			{
				CTaskJob *lpJob = rtGetTaskJob(lpPhone);
				bRes = tmSaveJobToFile(tm, hFil, lpJob);
			}
			if (!bRes)
				break;
		}
	}
	return bRes;
}
#endif
BOOL tmSaveSpecModeFromFile(CTaskManager *tm, FIL * hFil)
{
	UINT dwWriteCount;
	return (BOOL)(f_write(hFil, (LPVOID)&tm->m_lpSpecMode,  sizeof(CSpecialMode), &dwWriteCount)==FR_OK);
}

BOOL tmSaveToFile(CTaskManager *tm, const char *strFileName)
{
	FIL hFil;
	if(f_open(&hFil, strFileName, FA_READ | FA_WRITE | FA_CREATE_ALWAYS )!=FR_OK)
	{
		//str.Format(_T("建立文件 %s 错误 %d ！"), strFileName, i);
		//AfxMessageBox(str);
		return FALSE;
	}

	if(tmSaveHeaderToFile(tm, &hFil))
		if(tmSaveJobTaskToFile(tm, &hFil))
			if(tmSaveTaskGroupsToFile(tm, &hFil))
				if (tmSaveIntercutTaskFromFile(tm, &hFil))
#if (GC_ZR & GC_ZR_REMOTE)
				{
					if(mSaveYkTaskFromFile(tm, &hFil))
						if (mSavePhoneTaskFromFile(tm, &hFil))
							if (tmSaveSpecModeFromFile(tm, &hFil))
							{
								f_close(&hFil);
								return TRUE;
							}
				}
#else
					if (tmSaveSpecModeFromFile(tm, &hFil))
					{
						f_close(&hFil);
						return TRUE;
					}
#endif
	f_close(&hFil);

	return FALSE;
}

// =================================================================================
S8 CmpMyTime(PMyTime time1,PMyTime time2)
{
	U32 s1,s2;
	s1=MyTimeToSecond(time1);
	s2=MyTimeToSecond(time2);
	if(s1>s2)
		return 1;
	else
		if(s1<s2)
			return -1;
		else
			return 0;
}

S8 CmpShortDate(PShortDate date1,PShortDate date2)
{
	if(
		( (date1->byMonth>date2->byMonth) )||
		( (date1->byMonth==date2->byMonth)&&(date1->byDay>date2->byDay) )
		)
		return(1);
	else
		if( (date1->byMonth==date2->byMonth)&&(date1->byDay==date2->byDay) )
			return(0);
		else
			return(-1);
}

BOOL tmIsSpecialDate(CTaskManager *tm, PMyDate dt)
{
	return smValidDate(&tm->m_lpSpecMode, dt);
}

void tmGetTodayTaskGroup(CTaskManager *tm, CTaskJobGroup *lpGroup, PMyDate dt)
{
	CTaskGroup *lpTaskGroup;
	CIntercutTask *lpTask;
	CTaskJob *lpJob;
	int i;
	if(lpGroup==NULL) return;
	tjgClear(lpGroup);
	lpTaskGroup= tglGetTaskGroup(&tm->m_lpTaskGroups, dt);
	if(lpTaskGroup)
	{
		if(tmIsSpecialDate(tm, dt))
			tjgAssign( lpGroup, tmGetSpecialGroup(tm, lpTaskGroup));
		else
			tjgAssign(lpGroup, tmGetGroupOfDate(tm, lpTaskGroup, dt));
	}
	
	// 检查是否有插入任务
	for( i=0;i<itgGetCount(&tm->m_lpIntercutTasks);i++)
	{
		lpTask= itgGetAt(&tm->m_lpIntercutTasks, i);
		if(itValidDate(lpTask, dt))
		{
			lpJob= tjgNew(lpGroup);
			if(lpJob)
				tjAssign(lpJob, itGetTaskJob(lpTask));
		}
	}
	tjgSort(lpGroup);
}

void tmUpdateTodayTask(CTaskManager *tm, PMyDate dtm)
{
	tmGetTodayTaskGroup(tm, &tm->m_lpTodayTask.m_lpTodayTask, dtm);
}
// *******************************************************************************************************
U32  MyTimeToSecond(PMyTime time1)
{
	return (time1->byHour*3600+time1->byMinute*60+time1->bySecond);
}

void SecondToMyTime(U32 iSecond,PMyTime pt)
{
	pt->byHour= iSecond / 3600;
	iSecond= iSecond % 3600;
	pt->byMinute= iSecond / 60;
	pt->bySecond= iSecond % 60;

}

void  MyTimeAdd(PMyTime res,PMyTime tt,S32 iSecond)
{
	iSecond=iSecond+MyTimeToSecond(tt);
	SecondToMyTime(iSecond,res);
}

/****************************************************************/
/*	比较两个日期的大小 date1>date2=1 date1<date2=-1 date1=date2=0   */
/****************************************************************/
S8  CmpMyDate(PMyDate date1,PMyDate date2)
{
	if( (date1->wYear>date2->wYear) ||
		( (date1->wYear==date2->wYear)&&(date1->byMonth>date2->byMonth) )||
		( (date1->wYear==date2->wYear)&&(date1->byMonth==date2->byMonth)&&(date1->byDay>date2->byDay) )
		)
		return(1);
	else
		if( (date1->wYear==date2->wYear)&&(date1->byMonth==date2->byMonth)&&(date1->byDay==date2->byDay) )
			return(0);
		else
			return(-1);
}


U8 DaysOfMonth[12]={31,28,31,30,31,30,31,31,30,31,30,31};
BOOL DateIsValid(PMyDate lpDate)
{
	return (BOOL)((lpDate->byMonth>0)&&(lpDate->byMonth<13)&&(lpDate->byDay>0)&&(lpDate->byDay<=DaysOfMonth[lpDate->byMonth-1]));
}

// ----------------------------------------------------------------------------------------
// 计算从公元元年1月1号到tm日期经过的天数
u32 DateOfDays(u16 year, u8 month, u8 day)
{
	static int const days[13]={0,31,28,31,30,31,30,31,31,30,31,30,31};
	u32 sum=0;
	u32 i;
	if(month>12||day>31||month<=0||day<=0) 
		return 0;
	else
	{
		for (i=1;i<year;i++)
		{
			if((year%4==0&&year%100!=0)||year%400==0)
			{ 
				sum+= 366;
			}else
			{
				sum+= 365;
			}
		}
		for(i=1;i<month;i++)
		{
			if(year%4==0&&year%100!=0||year%400==0)
			{
				if(i==2)
					sum=sum+days[i]+1;
			}else
			{
				sum=sum+days[i];
			}
		}
		sum=sum+day;
	}
	return sum;	
}

u32 MyDateOfDays(PMyDate tm)
{
	return DateOfDays(tm->wYear, tm->byMonth, tm->byDay);
}

// ----------------------------------------------------------------------------------------
// 获取日期y-m-d的星期值 1-星期一， 2-星期二.....7-星期天
u8 DayOfWeek(u16 y, u8 m, u8 d)
{
	if(m==1||m==2) {
		m+=12;
		y--;
	}
	return (2UL*m+3UL*(m+1UL)/5+y+y/4-y/100+y/400+d)%7+1;
} 

u8 MyDateOfWeek(PMyDate tm)
{
	return DayOfWeek(tm->wYear, tm->byMonth, tm->byDay);
}
// ----------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------
// 计算从1月1号到tm日期经过的天数
u32 ShortDateOfDays(PShortDate tm)
{
	u32 dwRes=0;
	u8 i;
	for(i=0;i<tm->byMonth-1; i++)
		dwRes+= DaysOfMonth[i];
	dwRes += tm->byDay;
	if((tm->byMonth==2)&&(tm->byDay==29))
		dwRes--;
	return dwRes;
}

// 取得某年某月的天数
u8 GetDayCount(u16 wYear, u8 byMonth)
{
	u8 byRes= DaysOfMonth[byMonth-1];
	if(byMonth==2)
	{
		if((wYear!=0)&&(
			(((wYear % 100) !=0)&&((wYear % 4)==0)) ||
			(((wYear % 100) ==0)&&((wYear % 400)==0)) ))
			byRes +=1;
	}
	return byRes;
}

// -----------------------------------------------------------------------------------------
