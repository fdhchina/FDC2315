#ifndef _FILEVIEW_H_
#define _FILEVIEW_H_

#define	FIX_SONG_MAX	0xF7
#define	EXT_SONG_MAX	256

typedef struct _DIRNAME_BUF
{
	U16 count;
	char path[NANDFLASH_PARTITION_MAXUNIT+1][50];
	U8	path_index[EXT_SONG_MAX];
	U32	dirpos[EXT_SONG_MAX];
	char filelist[EXT_SONG_MAX][13];
}DirName_Buf,*PDirName_Buf;

extern DirName_Buf	udisk_audiolist;


u8 ReadTaskFile(void);
u8 SaveTaskFile(void);

U16  GetUdiskRootFileList(void);
void GetRealName(char *pEntryName, const char *pOrgName);
void ExtractFileName(u8 includeext, const char *path_name,char *name);
int GetLongFileName(char *shortname,char *longname,u8 getlen,unsigned long dirpos);
U8 GetFileList(char *path,PDirName_Buf list,U8 pathindex, u8 addpath);
void GetPathToFileList(char *path, PDirName_Buf fixlist, PDirName_Buf extlist, U8 pathindex, u8 addpath);
void Show_Directory(char *path, S8 inc);
void Show_AudioList(DirName_Buf* list,S8 inc);
void Show_NullFileName(void);
void ClearDirNameBuf(void);
u8 CopyFile(char *source,char *des);
void CopySongFromUDisk(void);




#endif
