
#include "hal.h"

#include <string.h>
#include <stdlib.h>
#include "stringconst.h"
#include "vs1003.h"
#include "ff.h"
#include "player.h"


// =====================================================================
// δ����ʱ��·�������Լ�С��������
#include "audio.h"

#define player_Mute	audio_setSourceMute
// =====================================================================

// *****************************************************
// ����������
// *****************************************************
TPlayer Player;

static u8 mp3buf1[FILE_READ_BYTES], mp3buf2[FILE_READ_BYTES];
void player_Init()
{
	Player.FileHandle= NULL;
	Player.FileSize=0;
	Player.State= ERR_PLAY_STOPPED;
	Player.MP3_buf1= mp3buf1;
	Player.MP3_buf2= mp3buf2;
	Player.BufIndex=1;
	Player.RepeatMode= REPEAT_ALL;
	Player.IndexOfDir=0;
	Player.SongDir[0].byValid= 0;
	Player.SongDir[0].sPath= (char *)sSongPaths[0];
	Player.SongDir[1].byValid= 0;
	Player.SongDir[1].sPath= (char *)sSongPaths[1];
}

u8  player_getState()
{
	return Player.State;
}

u8 player_GetCurDevice()
{
	return Player.byCurDev;// dj.fs->drv;
}

// ====================================================================================

//		����c�Ƿ�������Ascii��
BOOL CharIsNum(char c)
{
	return (BOOL)( (c>0x2F) && (c<0x3A) );
}

// ��ȡ��������ǰ�ı���,����0��ʾ������Ч����������
u16	getSongno(char *fn)
{
	u16	wRes;
	if(CharIsNum(fn[0])&&CharIsNum(fn[1])&&CharIsNum(fn[2]))
	{
		wRes= (fn[0]-0x30) *100+(fn[1]-0x30)*10+fn[2]-0x30;
		return wRes;
	}
	return 0;
}

// Ŀ¼����Ŀ����
BOOL IsSongFile(char *fn)
{
	char *sExt= &fn[strlen(fn)-4];
	if(	(memcmp(sExt, ".MP3", 4)==0)
		|| (memcmp(sExt, ".WMA", 4)==0)
		|| (memcmp(sExt, ".WAV", 4)==0)
		// || (strcasecmp(sExt, "OGG")==0)
					)
		return TRUE;
	return FALSE;
}
///XXX
BOOL IsSongDir(char *fn)
{
	if(CharIsNum(fn[0])&&CharIsNum(fn[1])&&CharIsNum(fn[2]))
        return TRUE;
	return FALSE; 
}
extern BOOL ch375_GetDirectoryInfo(u16 *pFlCnt);
extern BOOL ch375_FindSongIndex(const char *szPath, char *szFile, u16 *index, struct _TAG_PLAYER *pPlayer, char *lfname);

BOOL GetDirectoryInfo(const char *sPath, BOOL isFix, u16 *pFdcnt, u16 *pFlCnt)
{//XXX
	DIR dj;
	FILINFO fno;
//	u16 nSongCnt;
//	char sNewpath[71];
	fno.lfname= 0;
	fno.lfsize= 0;
	if(pFdcnt)
		*pFdcnt= 0;
	if(pFlCnt)
		*pFlCnt= 0;
	if(f_opendir(&dj, sPath)==FR_OK)
	{
		while(f_readdir(&dj, &fno)==FR_OK)
		{
			if( fno.fname[0] == 0) break;
			if(fno.fname[0] == '.') continue;
			/*if(pFdcnt&&(fno.fattrib&AM_DIR))	// ��Ŀ¼
			{
				strcpy(sNewpath, sPath);
				strcat(sNewpath, fno.fname);
				nSongCnt=0;
				if(GetDirectoryInfo(sNewpath, FALSE, NULL, &nSongCnt))
					if((isFix==FALSE)||(isFix&&getSongno(fno.fname)))
						if(nSongCnt)
								(*pFdcnt)++;
				
			}else*/
				if((fno.fattrib&AM_DIR)==0)
				{
					if ( IsSongFile(fno.fname))	// �������ļ�
					{
						if((isFix==FALSE)||(isFix&&getSongno(fno.fname)))
							if(pFlCnt)
								(*pFlCnt)++;
					}
				}
				else
				{
					if(IsSongDir(fno.fname))
					{						
						if((isFix==FALSE)||(isFix&&getSongno(fno.fname)))
								if(pFlCnt)
									(*pFlCnt)++;
					}
				}
		}
		return TRUE;
	}
	return FALSE;
}

BOOL sd_FindSongIndex(const char *szPath, char *szFile, u16 *index, struct _TAG_PLAYER *pPlayer, char *lfname)
{
	DIR	dj;
	FILINFO fno;
	u16 nCnt=0xFFFF;
	char sLastFile[13], sFirstFile[13], lfn[51], lfnfirst[51];

	fno.lfname= lfn;
	fno.lfsize= 50;

  if(f_opendir(&dj, szPath)==FR_OK)
 	{
  	while(f_readdir(&dj, &fno)==FR_OK)
	  {
		  if( fno.fname[0] == 0) break;
 			if((fno.fattrib&AM_DIR)==0)
  		{
	  		if ( IsSongFile(fno.fname))	// �������ļ�
		  	{
 					nCnt++;
  				if(nCnt==0)
 					{
  					strncpy(sFirstFile, fno.fname, 13);
	  				if(lfname)
							strncpy(lfnfirst, lfn, 50);
		  		}
 					if(*index==0xFFFF)
  				{
 						strncpy(sLastFile, fno.fname, 13);
  					if(lfname)
 							strncpy(lfname, lfn, 50);
  				}else
	  				if(*index==nCnt)
		  			{
 							pPlayer->IndexOfDir= nCnt;
  						strncpy(szFile, fno.fname, 13);
 							if(lfname)
 								strncpy(lfname, lfn, 50);
  						return TRUE;
 						}	
  			}
 			}
  	}
 	}
	if((*index==0xFFFF)&&(nCnt!=0xFFFF))
	{
		pPlayer->IndexOfDir= nCnt;
		strncpy(szFile, sLastFile, 13);
		return TRUE;
	}
	if((*index>=nCnt)&&(pPlayer->RepeatMode>REPEAT_ONE))// index����������������ѭ��ȫ��ģʽ
	{
		pPlayer->IndexOfDir= 0;
		strncpy(szFile, sFirstFile, 13);
		if(lfname)
			strncpy(lfname, lfnfirst, 50);
		return TRUE;
	}
	return FALSE;
}

// ------------------------------------------------------------------------------------------------------------
// �ҵ�Ŀ¼������Ϊindex�ĸ����ļ�
static BOOL FindSongIndex(const char *szPath, char *szFile, u16 index, char *lfname)
{
	if(index==0xFFFF)
		index= Player.SongDir[Player.byCurDev].nCount-1;
	else
		if(index>=Player.SongDir[Player.byCurDev].nCount)
			index=0;
#if 0
	if(Player.byCurDev==USB)
	{
		return ch375_FindSongIndex("/", szFile, &index, &Player, lfname);
	}else
#endif
		return sd_FindSongIndex(Player.SongDir[Player.byCurDev].sPath, szFile, &index, &Player, lfname);
}

// �ҵ�Ŀ¼���ļ���ǰ��λ����Ϊno�ĸ����ļ�
BOOL FindSongno(const char *szPath, u16 no, BOOL isFix, char *szFile, char *lfname, u16 *pIndex)
{
	DIR	dj;
	FILINFO fno;
	u16 nCnt=0, wSongNo;
	char lfn[51];
	fno.lfname= lfn;
	fno.lfsize= 50;
	if(f_opendir(&dj, szPath)==FR_OK)
	{
		while(f_readdir(&dj, &fno)==FR_OK)
		{
			if( fno.fname[0] == 0) break;
			if((fno.fattrib&AM_DIR)==0)  //文件
			{
				if ( IsSongFile(fno.fname))	// �������ļ�
				{
					wSongNo= getSongno(fno.fname);
					if(wSongNo==no)
					{
						if(szFile)
						{
							// strcpy(szFile, szPath); 
							strncpy(szFile, fno.fname, 13);
						}
						if(pIndex)
							*pIndex= nCnt;
						if(lfname)
							strncpy(lfname, lfn, 50);
						return TRUE;
					}	
					if((isFix==FALSE)||(isFix&&wSongNo))
						nCnt++;
				}
			}
			///XXX
				else //文件夹
				{
					if(IsSongDir(fno.fname))//编号文件夹
					{
						wSongNo= getSongno(fno.fname);
						if(wSongNo == no)
						{
							if(szFile)
							{
								strncpy(szFile, fno.fname, 13);
							}
							if(pIndex)
							*pIndex= nCnt;
							if(lfname)
								strncpy(lfname, lfn, 50);
							return TRUE;
						}
						if((isFix==FALSE)||(isFix&&wSongNo))
							nCnt++;
					}
				}
			}
	}
	return FALSE;
}

// *****************************************************************************
//          ******************ȡ�����ļ�����************************
// *****************************************************************************
#include "mpegaudio.h"
#include "wavfile.h"
#include "wmafile.h"

u32 GetAudioLength(const char *fname)
{
	const char *ext;
	TMPEGaudio mp3;
	TWAVfile	wav;
	TWMAfile	wma;
	
	ext= &fname[strlen((char *)fname)-4];
	if(memcmp(ext,".MP3",4)==0)
	{
		mp3_ReadFromFile(&mp3,fname);
		return mp3.FDuration;
	}
	if(memcmp(ext,".WAV",4)==0)
	{
		wav_ReadFromFile(&wav,fname);
		return wav.FDuration;
	}
	if(memcmp(ext,".WMA",4)==0)
	{
		wma_ReadFromFile(&wma,fname);
		return wma.FDuration;
	}
	return 60;
}

// *****************************************************************************

// ====================================================================================

void player_Pause()
{
	if(Player.State==ERR_PLAY_STOPPED)return;
	if(Player.State==ERR_PLAY_PLAYING)
	{
		Player.State=ERR_PLAY_PAUSEED;
		player_Mute(TRUE);
	}
	else
	{
		Player.State=ERR_PLAY_PLAYING;
		player_Mute(FALSE);
	}
	// USB_Source_Ctrl(Player.State==ERR_PLAY_PLAYING);
}

void _stopPlay()
{
	player_Mute(TRUE);
	if(Player.FileHandle)
	{
		vs1003_ClearDataBuf();
		vs1003_ResetDecodeTime();
		// Player.PlayTime=0;
#if 0
		if(Player.byCurDev==USB)
			ch375_f_close(Player.FileHandle);
		else
#endif
			f_close(Player.FileHandle);
		Player.FileHandle= NULL;
		Player.BufIndex=0x1;
		Player.FileSize=0;
	}		
}

u8 player_ReadData()
{
	u8 * buf;
	UINT len;
	FRESULT res;
	
	if(Player.BufIndex==1)
	{
		buf = Player.MP3_buf1;
	}
	else
	{
		buf = Player.MP3_buf2;
	}
#if 0
	if(Player.byCurDev==USB)
		res= ch375_f_read(Player.FileHandle, buf, FILE_READ_BYTES, &len);
	else
#endif
		res= f_read(Player.FileHandle, buf, FILE_READ_BYTES, &len);
	if(res!=FR_OK)
	{
		if(Player.FileSize>0)
		{
			_stopPlay();
		}
		return (u8)-1;
  	}
	Player.SendNumPerSec[1-Player.BufIndex]=len;  // ��¼ʵ���Ѿ����������ݵķ��ʹ��� 
	return 0;
}

void ExtractFileName(u8 includeext, const char *path_name,char *name)
{
	char *len,*len1;
	
	if(includeext)
		len= (char *)path_name+strlen(path_name);
	else
		len= strrchr(path_name,'.');
	len1= strrchr(path_name,'\\');
	if(len1==NULL)	len1=(char *)path_name;
			else	len1++;
	if(len==NULL)	len=(char *)path_name+strlen(path_name);
			else	len--;
	strncpy(name,len1,(len-len1+1));
	name[len-len1+1]='\0';
}

BOOL player_OpenFile(const char *sfname, char *lfname)
{
	static FIL		__mp3File;
	char sfile[50], *filename;
	if(sfname)
		filename= (char *)sfname;
	else
	{
		strcpy(sfile, Player.SongDir[Player.byCurDev].sPath);
		filename= &sfile[strlen(sfile)];
		if((Player.mode==pmAuto)&&(Player.byCurDev==MMC))
		{
			if(FindSongno(Player.SongDir[Player.byCurDev].sPath, Player.IndexOfDir, FALSE, filename, lfname, &Player.IndexOfDir)==FALSE)
				return FALSE;
		}else
			if(FindSongIndex(Player.SongDir[Player.byCurDev].sPath, filename, Player.IndexOfDir, lfname)==FALSE)
				return FALSE;
		filename= sfile;
	}
	// _stopPlay();
#if 0
	if(Player.byCurDev==USB)
	{
		if( ch375_f_open(&__mp3File, filename, FA_OPEN_EXISTING|FA_READ)!=FR_OK)
			return FALSE;
	}else
#endif
	{
		if( f_open(&__mp3File, filename, FA_OPEN_EXISTING|FA_READ)!=FR_OK)
			return FALSE;
	}
	if(lfname)
		if(*lfname==0)
			ExtractFileName(1, filename, lfname);
	Player.FileHandle= &__mp3File;
	Player.FileSize= Player.FileHandle->fsize;
	if(Player.FileSize<512) return FALSE;
	vs1003_SoftReset();
	Player.BufIndex=1;
	if(player_ReadData())
  {
    _stopPlay();
    return FALSE;
  }
	Player.BufIndex=0;
	if(player_ReadData())
  {
    _stopPlay();
    return FALSE;
  }
	Player.Data_Index=0;
	
	vs1003_SetVolume(0x0000);
	return TRUE;
}

void player_Stop()
{
	_stopPlay();
	Player.State=ERR_PLAY_STOPPED;
}

u8 player_PlayDo()
{
	u16 j;
	u8 ir=ERR_PLAY_PLAYING;
	u8 * buffer;
	if((Player.State>ERR_PLAY_PAUSEED))
	{
		if ( Player.FileSize) {
			buffer=(Player.BufIndex)?Player.MP3_buf2:Player.MP3_buf1;
			for(j=0;(j<14) && VS1003_DREQ;j++)
			{
				if(Player.Data_Index<Player.SendNumPerSec[Player.BufIndex])
				{
					vs1003_WriteData(&buffer[Player.Data_Index], 32);
					Player.Data_Index+=32;
				}else
				{
					Player.FileSize-=(Player.SendNumPerSec[Player.BufIndex]);
					if (Player.FileSize<=0) {  // ʵ�ʶ������ַ�������Ҫ���������ַ���,˵���Ѿ����ļ��Ľ�β 
						Player.FileSize=0;
						return ERR_PLAY_STOPPED;
					}
					buffer=(Player.BufIndex)?Player.MP3_buf1:Player.MP3_buf2;
					
					Player.BufIndex=!Player.BufIndex; 
					if(player_ReadData())
          			{
			            _stopPlay();
            			return ERR_PLAY_ERROR;
          			}
					Player.Data_Index=0;
				}
			}
		}else
			if(Player.State==ERR_PLAY_PLAYING)
				ir = ERR_PLAY_STOPPED;
	}else
		if(Player.State==ERR_PLAY_PLAYING)
			ir = ERR_PLAY_STOPPED;
	return ir;
}


void player_Play()
{
	Player.State=ERR_PLAY_PLAYING;
	player_Mute(FALSE);
}

void player_UpdateSongDir(u8 byDev)
{
	u16	nCount;
	Player.SongDir[byDev].byValid= 1;
#if 0
	if(byDev==USB)
	{
		ch375_GetDirectoryInfo(&nCount);
		Player.SongDir[USB].nCount= nCount &0x7FFF;
		return ;
	}
#endif
	GetDirectoryInfo(Player.SongDir[byDev].sPath, FALSE, NULL, &nCount);
	Player.SongDir[byDev].nCount= nCount &0x7FFF;
}

void player_RemoveSongDir(u8 byDev)
{
	Player.SongDir[byDev].byValid= 0;
  Player.SongDir[byDev].nCount= 0;
}

// �����ڴ�Ŀ¼����Ŀ����Ϊindex������,index=0xffff ���ڲ���������
void _playmemsong(u16 index, char *lfname)
{
	Player.byCurDev= MMC;
	if(Player.SongDir[MMC].byValid==0)
	{
		player_UpdateSongDir(MMC);
	}
	if(index!= 0xFFFF)
		Player.IndexOfDir= index;
	if(player_OpenFile(NULL, lfname))
		player_Play();
}

void player_PlayMem(u16 index, char *lfname)
{
	Player.mode= pmManual;
	Player.RepeatMode= REPEAT_ALL;
	_playmemsong(index, lfname);
}

#if (GC_ZR & GC_ZR_PLAY)
void player_PlayMemBj(u16 index, char *lfname)
{
	Player.mode = pmManual;
	Player.RepeatMode = REPEAT_ONE;
	_playmemsong(index, lfname);
}
#endif

void _playusbsong(u16 index, char *lfname)
{
	Player.byCurDev= USB;
	if(Player.SongDir[USB].byValid==0)
	{
		player_UpdateSongDir(USB);
	}
	if(Player.SongDir[USB].nCount==0) return ;
	if(index==0xFFFF)
		Player.IndexOfDir= random(Player.SongDir[USB].nCount)-1;
	else
    if(index>Player.SongDir[USB].nCount-1)
      Player.IndexOfDir= 0;
    else
  		Player.IndexOfDir= index;
	Player.RepeatMode= REPEAT_ALL;
	if(player_OpenFile(NULL, lfname))
	{
		SaveUSBPlayPosition(Player.IndexOfDir);
		player_Play();
	}
}

// =============================================================================
char USB_PLAYPOSITION_FILENAME[]= "1:\\fdc_orde.bin";

u16 LoadUSBPlayPosition()
{
	FIL		tmpFile;
	u32		dwReadCnt;
	u16		nPos=0;
	if(f_open(&tmpFile, USB_PLAYPOSITION_FILENAME, FA_OPEN_EXISTING|FA_READ)==FR_OK)
	{
		if(f_read(&tmpFile, &nPos, sizeof(u16), &dwReadCnt)==FR_OK)
		{
			if(dwReadCnt!=sizeof(u16))
				nPos= 0;
		}
		f_close(&tmpFile);
	}
	return nPos;
}

void SaveUSBPlayPosition(u16 nPos)
{
	FIL		tmpFile;
	u32		dwWriteCnt;
	if(f_open(&tmpFile, USB_PLAYPOSITION_FILENAME, FA_OPEN_ALWAYS | FA_WRITE)==FR_OK)
	{
		f_write(&tmpFile, &nPos, sizeof(u16), &dwWriteCnt);
		f_close(&tmpFile);
	}
}
// =============================================================================

void player_PlayUSB(u16 index, char *lfname)
{
	Player.mode= pmManual;
	_playusbsong(index, lfname);
}

void player_AutoPlayMem(u16 nSongNo, char *lfname)
{
	Player.mode= pmAuto;
	Player.RepeatMode= REPEAT_NONE;
	_playmemsong(nSongNo, lfname);
}


void player_AutoPlayUSB(char *lfname)
{
	Player.mode= pmAuto;
	Player.RepeatMode= REPEAT_NONE;
	_playusbsong(LoadUSBPlayPosition(), lfname);	// ���Ŵ�U���ϴ����ڲ��ŵ���Ŀ
	// _playusbsong(0xFFFF, lfname);
}



int	random(int maxvalue)
{
	if(!maxvalue) return 0;
	srand(dwTickCount);
	return rand() % maxvalue;
}

void next_song(char *lfname, signed char de)
{
	_stopPlay();
	Player.IndexOfDir += de;
	if(player_OpenFile(NULL, lfname))
	{
		if(Player.State==ERR_PLAY_PLAYING)
		{
			player_Play();
			if (Player.byCurDev==USB)
				SaveUSBPlayPosition(Player.IndexOfDir);
			return ;
		}
	}
	Player.State=ERR_PLAY_STOPPED;
	player_Stop();
}

void player_Next(char *lfname)
{
	next_song(lfname, Player.RepeatMode>REPEAT_ONE);
}

void player_Prev(char *lfname)
{
	next_song(lfname, -1);
}

u16 player_getPlayIndex()
{
	return Player.IndexOfDir;
}


