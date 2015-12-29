#ifndef _WMAFILE_H_
#define _WMAFILE_H_


#include  "hal.h"

  //  Class TWMAfile *
typedef struct  _TWMAfile{
      //  Private declarations *
	u8			FValid;
	int				FFileSize;
	u8				FChannelModeID;
	int				FSampleRate;
	u32			FDuration;
	int				FBitRate;
	int				FTrack;
/*
	    FTitle: WideString;
      FArtist: WideString;
      FAlbum: WideString;
      FYear: WideString;
      FGenre: WideString;
      FComment: WideString;
*/
}TWMAfile,*PWMAfile;


u8 wma_ReadFromFSFile(PWMAfile self, FIL *hFile);
u8 wma_ReadFromFile(PWMAfile self, const char *FileName);

#endif

