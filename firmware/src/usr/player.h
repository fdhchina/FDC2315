#ifndef _PLAYER_H_
#define _PLAYER_H_

// ----------------------------------------------------------------------------------
// ����Ŀ¼����
#define SONGPATH_SD_INDEX		0
#define SONGPATH_UDISK_INDEX		1
#define SONGPATH_RECORD_INDEX		2

//����״̬
#define ERR_PLAY_STOPPED		1		//ֹͣ״̬
#define ERR_PLAY_START			2		//��ʼ����
#define ERR_PLAY_PAUSEED		3		//��ͣ
#define ERR_PLAY_PLAYING		4		//���ڲ���
#define ERR_PLAY_ERROR			0xFF	//��ȡU�����ݴ���

//¼��״̬
#define ERR_REC_START			1		//��ʼ¼��
#define ERR_REC_RECORDING		2		//����¼��
#define	ERR_REC_PAUSEED			3		//��ͣ
#define	ERR_REC_STOPPED			8		//ֹͣ״̬
#define	ERR_REC_ERROR			0xFF	//д��U�����ݴ���

//����ͼ����ʾ��
#define PLAYER_PIC_PLAY			0		
#define PLAYER_PIC_PAUSE		1
#define PLAYER_PIC_STOP			2
#define PLAYER_PIC_PREV			3
#define PLAYER_PIC_NEXT			4
#define PLAYER_PIC_EJECT		5
#define PLAYER_PIC_RECORD		6

// =====================================================================================

#define FILE_READ_BYTES			0x800	//ÿ�ζ��ļ����ֽ���

typedef enum{
	REPEAT_NONE=0,
	REPEAT_ONE=1,
	REPEAT_ALL=2,
	REPEAT_LIST=3
}TPlayerRepeatMode; 

typedef enum{
	pmManual=0, //�ֶ�
	pmAuto     //�Զ�
}TPlayerMode;

// ����Ŀ¼
typedef struct {
	char *sPath;			// ����·��
	u16 nCount:15;		// ��������
	u8 byValid:1;				// �Ѿ���ȡ��������
}SongDir_t;

typedef struct _TAG_PLAYER
{
	FIL *FileHandle;
	volatile s32 FileSize;
	volatile u8	State:4;
	TPlayerMode	mode: 2;
	TPlayerRepeatMode RepeatMode:2;

	u8 BufIndex;
	u32 SendNumPerSec[2];
	u32 Data_Index;
	u8 *MP3_buf1, *MP3_buf2;

	u8  byCurDev;     // ��ǰ���ŵ��豸����
	u8  byRes;
	SongDir_t SongDir[2];
	DIR	dj;					// ��ǰ���ŵ�Ŀ¼
	u16 IndexOfDir;			// �����ļ�����
}TPlayer, *PPlayer;

extern TPlayer Player;
u8	player_GetCurDevice(void);	// ���ز����豸����
BOOL IsSongFile(char *fn);
u16	getSongno(char *fn);
// ��ȡĿ¼����������Ŀ¼�����������ļ�������isFix��ʾ�Ƿ�ֻ��ѯ�����ŵĸ�����Ŀ¼
BOOL GetDirectoryInfo(const char *sPath, BOOL isFix, u16 *pFdcnt, u16 *pFlCnt);
// �ҵ�Ŀ¼���ļ���ǰ��λ����Ϊno�ĸ����ļ�
BOOL FindSongno(const char *szPath, u16 no, BOOL isFix, char *szFile, char *lfname, u16 *pIndex);
u32 GetAudioLength(const char *fname);
void player_UpdateSongDir(u8 byDev);
void player_RemoveSongDir(u8 byDev);
int	random(int maxvalue);	// ��ȡһ��1��maxvalue��������
void player_Init(void);
u8  player_getState(void);
void player_PlayMem(u16 index, char *lfname);
#if (GC_ZR & GC_ZR_PLAY)
void player_PlayMemBj(u16 index, char *lfname);
#endif
void player_AutoPlayMem(u16 nSongNo, char *lfname);
void player_PlayUSB(u16 index, char *lfname);
void player_AutoPlayUSB(char *lfname);
BOOL player_OpenFile(const char *fname, char *lfname);
void player_Stop(void);
void player_Pause(void);
void player_Play(void);
void player_Next(char *lfname);
void player_Prev(char *lfname);
u16 player_getPlayIndex(void);
u8 	player_PlayDo(void);
///
BOOL IsSongDir(char *fn);
void SaveUSBPlayPosition(u16 nPos);
u16 LoadUSBPlayPosition(void);
#endif

