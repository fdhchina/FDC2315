#ifndef _ID3V2_H_
#define _ID3V2_H_

  //  Class TID3v2 *
typedef struct  _TID3v2 {

      //  Private declarations *
	u8			FExists;
	u8				FVersionID;
	int				FSize;
	char				FTitle[30];
	char				FArtist[30];
	char				FAlbum[30];
	u16			FTrack;
	char				FTrackString[30];
	char				FYear[4];
	char				FGenre[30];
	char				FComment[30];
	char				FComposer[30];
	char				FEncoder[30];
	char				FCopyright[30];
	char				FLanguage[30];
	char				FLink[30];
}TID3v2,*PID3v2;


void ID3v2_ResetData(PID3v2 self);
u8 ID3v2_ReadFromFile(PID3v2 self , FIL *hFile);

#endif
