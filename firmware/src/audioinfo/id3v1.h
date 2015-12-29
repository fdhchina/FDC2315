#ifndef _ID3V1_H_
#define _ID3V1_H_

typedef  struct 	_TID3v1{
      // Private declarations *
	u8		FExists;
	u8			FVersionID;
	char			FTitle[30];
	char			FArtist[30];
	char			FAlbum[30];
	char			FYear[4];
	char			FComment[30];
	u8			FTrack;
	u8			FGenreID;
}TID3v1,*PID3v1;

void ID3v1_ResetData(PID3v1 self);
u8	ID3v1_ReadFromFile(PID3v1 self, FIL	*hFile);
#endif
