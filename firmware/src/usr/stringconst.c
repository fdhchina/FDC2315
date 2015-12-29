#include "hal.h"

#include "stringconst.h"


char const sProduct[] = "������������";
char const sCompany[]= "�Ĵ���ɽ�����������ι�˾";
char const sModal[]= "�ƣģã�������";
char const sMenuTitles[][9]={
				"�������",
				"��Ŀ�鿴",
				"ϵͳ����",
				"����ģʽ",
				};
char const sSystemSets[][9]= {
	"ʱ������",
	"��������",
#if (GC_ZR & GC_ZR_BJ)
	"��������"
#endif
};
char const sTaskMenuTitles[][9]={
				"����鿴",
				"�����¼�",
				"��������",
				"�岥����",
#if (GC_ZR & GC_ZR_REMOTE)
				"ң������",
				"�绰����",
#endif
				};
char const sManualPlay[]=	"�ֶ�����";

char const sSpecialModes[][11]= {
	"��������Ч",
	"�ӽ��쿪ʼ",
	"�����쿪ʼ",
	"�Ӻ��쿪ʼ"
};
	
char const sSongPaths[][4]=	{ //0:SD��  1:U��
	"0:\\",
	"1:\\",
};

char const  sTaskFile[]= "0:/FDC2315.TGL";

char const  sStart[]	= "��ʼ";
char const  sEnd[] = "����";
char const  sSource[]	= "��Դ";
char const sPort[]= "��������";
char const sPrev[]="��һ��";
char const sNext[]="��һ��";
char const sTodayNoJob[]= "�����޲�������";
char const sZeroTime[]="00:00:00";
char const sCountFmt[]="[%03d/%03d]";
char const sEdit[]="�༭";
char const sAdd[]="���";
char const sDelete[]="ɾ��";
char const sSongPathOpenError[]="����·���򿪳���";
char const sNoSongFile[]="û����Ŀ�ļ���";
char const  sWeek[]= "����";
char const  sWeekday[][3]={
				"һ",
				"��",
				"��",
				"��",
				"��",
				"��",
				"��"	};

char const  sTimeFmt[]="%02d:%02d:%02d";
char const  sDateFmt[]="%02d-%02d-%02d";
char const  sSourceName[][9]={
                    "����ͷ",
                    "�ڴ�����",
                    "����    ",
                    "��������",
                    "��Ͳ����",
#if (GC_ZR & GC_ZR_PLAY)
					"��������",
					"��������"
#endif
                    };

const char * sSongViewMenus[2]= {
	sSourceName[1],
	sSourceName[3]
};

char const  sNextTime[]="����ʱ��";
char const  sNoTask[] =	"�������";
char const  sTodayTask[] =	"��������";
char const  sPlayed[]	=	"�Ѳ�";
char const  sPlaying[]	= "���ڲ���";

char const  sAmplifer[]=	"���ŵ�Դ";
char const  sTouYin[] = "ͶӰ";
char const  sRecord[]= "¼��״̬";

char const  sOpen[]	=	"�� �� ��";	//"��";
char const  sClose[]	=	"�� �� ��";	//"��";
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
	"�鿴",
	"����",
	"�޸�"};

char const sNoSD[]="SD���Ѳ�����";
char const sSDReadError[]="SD����ȡʧ�ܣ�";
char const sSelfCheck[]="���飡";
char const sSave[]="����";		
char const  sExtSong[]="��������";
char const sPrevPage[]="��";
char const sNextPage[]="��";
char const sDate[]="����";
char const sContent[]="��������";
char const sNoFunc[]="�˹��ܻ�δ��ɣ�";
char const sSystemIniting[]="ϵͳ���ڳ�ʼ��......";
//char const  sManual[]	=	"�ֶ�����";
//char const  sTaskView[]	=	"����鿴";
//char const  sTimeSet[]	=	"ʱ������";
//char const  sUDiskView[]	=	"���̲鿴";

#if (GC_ZR)
char const sFenqu[][16] = {
	"����1",
	"����2",
	"����3",
	"����4",
	"����5",
	"����6",
	"����7",
	"����8",
	"����9",
	"����10",
	"����11",
	"����12",
	"����13",
	"����14",
	"����15",
	"����16",
};
char const sKey[] = "��������";
char const sMusic[] = "����";
char const sFuncs[] = "����";
char const sKeyFuncs[] = "��������";
char const sKeyFuncsValue[][10] = {
	"����/��ͣ",
	"ֹͣ     ",
	"��һ��   ",
	"��һ��   ",
	"����+    ",
	"����-    ",
	"��Ч���� ",
};
char const sBjMusic[] = "��������";
char const sBjFcType[] = "��������";
char const sTunerSet[] = "��ѡ���̨Ƶ��";
#endif
