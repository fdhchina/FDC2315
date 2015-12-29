#ifndef _CONSTS_
#define _CONSTS_

#define SONG_MAX_COUNT			0xF7		//��Ŀ��������Fix_SongĿ¼��Ext_SongĿ¼�µ�MP3�ļ�
#define SONG_MAX_LENGTH			13		//��Ŀ�ļ�������󳤶Ȱ���·��,��Ϊ���ļ���
#define EXT_SOURCE_COUNT		4			//������Դ����

#define VOLUME_PLAY_SYM		0		//�������ŵı��
#define VOLUME_PAUSE_SYM	1		//�������ŵı��
#define VOLUME_STOP_SYM		2		//�������ŵı��
#define VOLUME_PREV_SYM		3		//�������ŵı��
#define VOLUME_NEXT_SYM		4		//�������ŵı��
#define VOLUME_RECORD_SYM	5		//�������ŵı��
#define VOLUME_EJECT_SYM	6		//�������ŵı��
#define VOLUME_MUTE_SYM		7		//�������ŵı��


extern char const  sProduct[];
extern char const  sModal[];
extern char const  sMenuTitles[][9];
extern char const sSystemSets[][9];
extern char const  sTaskMenuTitles[][9];
extern const char *sSongViewMenus[2];
extern char const sSpecialModes[][11];
extern char const  sManualPlay[];
extern char const sSongPaths[][4];

extern char const  sTaskFile[];
extern char const  sCompany[];
extern char const  sStart[];
extern char const  sEnd[];
extern char const  sSource[];
extern char const  sWeek[];
extern char const  sWeekday[][3];

extern char const  sTaskEdit[];
extern char const  sTimeFmt[];
extern char const  sDateFmt[];
extern char const  sSourceName[][9];
extern char const  sNextTime[];
extern char const  sNoTask[];

extern char const  sTodayTask[];
extern char const  sPlayed[];

extern char const  sPlaying[];

extern char const  sAmplifer[];
extern char const  sTouYin[];
extern char const  sRecord[];

extern char const  sOpen[];
extern char const  sClose[];
extern char const sSelected[];
extern char const sUnselected[];
extern char const sSymbol_Play[];
extern char const sSymbol_Pause[];
extern char const sSymbol_Stop[];
extern char const sSymbol_Prev[];
extern char const sSymbol_Next[];
extern char const sSymbol_NoSingle[];
extern char const sSymbol_LittleSingle[];
extern char const sSymbol_BigSingle[];

extern char const  sEditMode[][5];
extern char const sSave[];
extern char const nExtPage[];
extern char const pRePage[];
extern char const sPort[];
extern char const sPrev[];
extern char const sNext[];
extern char const sTodayNoJob[];
extern char const sZeroTime[];
extern char const sCountFmt[];
extern char const sEdit[];
extern char const sAdd[];
extern char const sDelete[];
extern char const sSongPathOpenError[];
extern char const sNoSongFile[];

extern char const sNoSD[];
extern char const sSDReadError[];
extern char const sSelfCheck[];
extern char const  sExtSong[];
extern char const  sPrevPage[];
extern char const  sNextPage[];
extern char const sDate[];
extern char const sContent[];
extern char const sNoFunc[];
extern char const sSystemIniting[];

#if (GC_ZR)
extern char const sFenqu[][16];
extern char const sKey[];
extern char const sMusic[];
extern char const sFuncs[];
extern char const sKeyFuncs[];
extern char const sKeyFuncsValue[][10];
extern char const sBjMusic[];
extern char const sBjFcType[];
extern char const sTunerSet[];
#endif
#endif
