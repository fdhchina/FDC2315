#ifndef _CONSTS_
#define _CONSTS_

#define SONG_MAX_COUNT			0xF7		//曲目总数包括Fix_Song目录和Ext_Song目录下的MP3文件
#define SONG_MAX_LENGTH			13		//曲目文件名的最大长度包括路径,此为短文件名
#define EXT_SOURCE_COUNT		4			//其它音源总数

#define VOLUME_PLAY_SYM		0		//静音符号的编号
#define VOLUME_PAUSE_SYM	1		//静音符号的编号
#define VOLUME_STOP_SYM		2		//静音符号的编号
#define VOLUME_PREV_SYM		3		//静音符号的编号
#define VOLUME_NEXT_SYM		4		//静音符号的编号
#define VOLUME_RECORD_SYM	5		//静音符号的编号
#define VOLUME_EJECT_SYM	6		//静音符号的编号
#define VOLUME_MUTE_SYM		7		//静音符号的编号


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
