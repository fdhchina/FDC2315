#include "hal.h"

#include "stringconst.h"


char const sProduct[] = "数码智能中心";
char const sCompany[]= "四川湖山电器有限责任公司";
char const sModal[]= "ＦＤＣ２３１５";
char const sMenuTitles[][9]={
				"任务管理",
				"节目查看",
				"系统设置",
				"雨天模式",
				};
char const sSystemSets[][9]= {
	"时间设置",
	"密码设置",
#if (GC_ZR & GC_ZR_BJ)
	"报警设置"
#endif
};
char const sTaskMenuTitles[][9]={
				"任务查看",
				"播音事件",
				"常规任务",
				"插播任务",
#if (GC_ZR & GC_ZR_REMOTE)
				"遥控任务",
				"电话任务",
#endif
				};
char const sManualPlay[]=	"手动播音";

char const sSpecialModes[][11]= {
	"　　　无效",
	"从今天开始",
	"从明天开始",
	"从后天开始"
};
	
char const sSongPaths[][4]=	{ //0:SD卡  1:U盘
	"0:\\",
	"1:\\",
};

char const  sTaskFile[]= "0:/FDC2315.TGL";

char const  sStart[]	= "开始";
char const  sEnd[] = "结束";
char const  sSource[]	= "音源";
char const sPort[]= "分区控制";
char const sPrev[]="上一条";
char const sNext[]="下一条";
char const sTodayNoJob[]= "当日无播音任务！";
char const sZeroTime[]="00:00:00";
char const sCountFmt[]="[%03d/%03d]";
char const sEdit[]="编辑";
char const sAdd[]="添加";
char const sDelete[]="删除";
char const sSongPathOpenError[]="乐曲路径打开出错！";
char const sNoSongFile[]="没有曲目文件！";
char const  sWeek[]= "星期";
char const  sWeekday[][3]={
				"一",
				"二",
				"三",
				"四",
				"五",
				"六",
				"日"	};

char const  sTimeFmt[]="%02d:%02d:%02d";
char const  sDateFmt[]="%02d-%02d-%02d";
char const  sSourceName[][9]={
                    "收音头",
                    "内存乐曲",
                    "辅助    ",
                    "Ｕ盘乐曲",
                    "话筒播音",
#if (GC_ZR & GC_ZR_PLAY)
					"紧急播音",
					"报警播音"
#endif
                    };

const char * sSongViewMenus[2]= {
	sSourceName[1],
	sSourceName[3]
};

char const  sNextTime[]="播音时间";
char const  sNoTask[] =	"播音完毕";
char const  sTodayTask[] =	"今日任务";
char const  sPlayed[]	=	"已播";
char const  sPlaying[]	= "正在播放";

char const  sAmplifer[]=	"功放电源";
char const  sTouYin[] = "投影";
char const  sRecord[]= "录音状态";

char const  sOpen[]	=	"【 开 】";	//"◎";
char const  sClose[]	=	"【 关 】";	//"○";
char const sSelected[]={0xA2, 0xF0, 0};
char const sUnselected[]={0xA1, 0xF5, 0};

char const nExtPage[]={0xA1, 0xFA, 0};
char const pRePage[]={0xA1, 0xFB, 0};

char const sSymbol_Play[]={0xA2, 0xAB, 0};
char const sSymbol_Pause[]={0xA2, 0xAC, 0};
char const sSymbol_Stop[]={0xA2, 0xAD, 0};
char const sSymbol_Prev[]={0xA2, 0xAE, 0};
char const sSymbol_Next[]={0xA2, 0xAF, 0};
char const sSymbol_NoSingle[]={0xA2, 0xE3, 0};
char const sSymbol_LittleSingle[]={0xA2, 0xE4, 0};
char const sSymbol_BigSingle[]={0xA2, 0xEF, 0};

char const  sEditMode[][5]={
	"查看",
	"新增",
	"修改"};

char const sNoSD[]="SD卡已拨出！";
char const sSDReadError[]="SD卡读取失败，";
char const sSelfCheck[]="请检查！";
char const sSave[]="保存";		
char const  sExtSong[]="扩充乐曲";
char const sPrevPage[]="↑";
char const sNextPage[]="↓";
char const sDate[]="日期";
char const sContent[]="播音内容";
char const sNoFunc[]="此功能还未完成！";
char const sSystemIniting[]="系统正在初始化......";
//char const  sManual[]	=	"手动播音";
//char const  sTaskView[]	=	"任务查看";
//char const  sTimeSet[]	=	"时间设置";
//char const  sUDiskView[]	=	"Ｕ盘查看";

#if (GC_ZR)
char const sFenqu[][16] = {
	"分区1",
	"分区2",
	"分区3",
	"分区4",
	"分区5",
	"分区6",
	"分区7",
	"分区8",
	"分区9",
	"分区10",
	"分区11",
	"分区12",
	"分区13",
	"分区14",
	"分区15",
	"分区16",
};
char const sKey[] = "按键编码";
char const sMusic[] = "任务";
char const sFuncs[] = "功能";
char const sKeyFuncs[] = "按键功能";
char const sKeyFuncsValue[][10] = {
	"播放/暂停",
	"停止     ",
	"上一曲   ",
	"下一曲   ",
	"音量+    ",
	"音量-    ",
	"无效功能 ",
};
char const sBjMusic[] = "报警音乐";
char const sBjFcType[] = "独立功放";
char const sTunerSet[] = "请选择存台频道";
#endif
