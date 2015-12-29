#include "hal.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "ff.h"
#include "wmafile.h"

  //  Channel modes *
#define	WMA_CM_UNKNOWN 			0                                               //  Unknown *
#define	WMA_CM_MONO				1                                               //  Mono *
#define	WMA_CM_STEREO 			2                                               //  Stereo *

  //  Channel mode names *
char		WMA_MODE[3][10] = 
{
	"Unknown", 
	"Mono", 
	"Stereo"
};


  //  Object IDs *
const u8  WMA_HEADER_ID[16]=
{48,38,178,117,142,102,207,17,166,217,0,170,0,98,206,108};
  
const u8  WMA_FILE_PROPERTIES_ID[16] =
{161,220,171,140,71,169,207,17,142,228,0,192,12,32,83,101};
const u8  WMA_STREAM_PROPERTIES_ID[16] =
{145,7,220,183,183,169,207,17,142,230,0,192,12,32,83,101};
const u8  WMA_CONTENT_DESCRIPTION_ID[16] =
{51,38,178,117,142,102,207,17,166,217,0,170,0,98,206,108};
const u8  WMA_EXTENDED_CONTENT_DESCRIPTION_ID[16] =
{64,164,208,210,7,227,210,17,151,240,0,160,201,94,168,80};

  //  Max. number of supported comment fields *
#define	WMA_FIELD_COUNT 		7

  //  Names of supported comment fields *
const char		WMA_FIELD_NAME[WMA_FIELD_COUNT][30] =
{
	{'W',0,'M',0,'/',0,'T',0,'I',0,'T',0,'L',0,'E',0}, 
	{'W',0,'M',0,'/',0,'A',0,'U',0,'T',0,'H',0,'O',0,'R',0}, 
	{'W',0,'M',0,'/',0,'A',0,'L',0,'B',0,'U',0,'M',0,'T',0,'I',0,'T',0,'L',0,'E',0}, 
	{'W',0,'M',0,'/',0,'T',0,'R',0,'A',0,'C',0,'K',0}, 
	{'W',0,'M',0,'/',0,'Y',0,'E',0,'A',0,'R',0},
	{'W',0,'M',0,'/',0,'G',0,'E',0,'N',0,'R',0,'E',0}, 
	{'W',0,'M',0,'/',0,'D',0,'E',0,'S',0,'C',0,'R',0,'I',0,'P',0,'T',0,'I',0,'O',0,'N',0}
};

  //  Max. number of characters in tag field *
#define	WMA_MAX_STRING_SIZE 		250


  //  Object ID *
typedef struct _ObjectID {
	char obj[16];
}ObjectID,*PObjectID;
  //  Tag data *
 typedef struct _TagData{
 	char	tag[2];//WMA_FIELD_COUNT][WMA_MAX_STRING_SIZE*2];
}TagData,*PTagData;

  //  File data - for internal use *
typedef struct  _FileData{
	int			FileSize;                                    //  File size (u8s) *
	int			MaxBitRate;                                //  Max. bit rate (bps) *
	u16		Channels;                                      //  Number of channels *
	int			SampleRate;                                   //  Sample rate (hz) *
	int			ByteRate;                                            //  Byte rate *
	TagData		Tag;                                       //  WMA tag information *
}FileData,*PFileData;
/*
//  ********************* Auxiliary functions & procedures ******************** *
void ReadFieldString( FIL *hFile,u16 DataSize,char *Result)
{
	UINT		br;
	int			StringSize;
//	u8			FieldData[WMA_MAX_STRING_SIZE * 2];

	//  Read field data and convert to unicode string *
	*Result = 0;
	StringSize = DataSize / 2;
	if( StringSize > WMA_MAX_STRING_SIZE) StringSize = WMA_MAX_STRING_SIZE;
	if(f_read(hFile, Result, StringSize * 2, &br)!=FR_OK)return ;
	f_lseek(hFile, hFile->fptr+DataSize - StringSize * 2);
}
*/
//  --------------------------------------------------------------------------- *
void ReadTagStandard( FIL *hFile, PTagData tag)
{
	UINT		br;
  	int			Iterator;
	u16		FieldSize[5];
	// char					FieldName[WMA_MAX_STRING_SIZE * 2];

	//  Read standard tag data *
	if(f_read(hFile, FieldSize, 10, &br)!=FR_OK)return;
	for( Iterator = 0;Iterator<5;Iterator++)
		if( FieldSize[Iterator] > 0)
		{
			f_lseek(hFile, hFile->fptr+FieldSize[Iterator]);
			
			/* 
			ReadFieldString(hFile, FieldSize[Iterator], FieldName);
		    switch(Iterator)
			{
			        case 0: memcpy(tag->tag[0], FieldName, FieldSize[Iterator]);break;
			        case 1: memcpy(tag->tag[1], FieldName, FieldSize[Iterator]);break;
			        case 3: memcpy(tag->tag[6], FieldName, FieldSize[Iterator]);break;
			}*/
		}
}


//  --------------------------------------------------------------------------- *

void ReadTagExtended( FIL *hFile,PTagData Tag)
{
	UINT				br;
	u16				Iterator1, FieldCount, DataSize, DataType;
	//char					FieldName[WMA_MAX_STRING_SIZE * 2],
	//					FieldValue[WMA_MAX_STRING_SIZE * 2];

	//  Read extended tag data *
	if(f_read(hFile, &FieldCount, 2, &br)!=FR_OK)return ;
	for( Iterator1 = 0;Iterator1<FieldCount;Iterator1++)
	{
		//  Read field name *
		if(f_read(hFile, &DataSize, 2, &br)!=FR_OK) return;
		
		if(f_lseek(hFile, hFile->fptr+DataSize)!=FR_OK)return;
		// ReadFieldString(hFile, DataSize,FieldName);
		
		//  Read value data type *
		if(f_read(hFile, &DataType, 2, &br)!=FR_OK) return;
		//  Read field value only if string *
		if( DataType == 0)
		{
			if(f_read(hFile, &DataSize, 2, &br)!=FR_OK) return ;
			if(f_lseek(hFile, hFile->fptr+DataSize)!=FR_OK)return;
			// ReadFieldString(hFile, DataSize,FieldValue);
		}    else
		      if(f_lseek(hFile, hFile->fptr+DataSize)!=FR_OK)return;
		//  Set corresponding tag field if supported *
		/* for( Iterator2 = 0;Iterator2<WMA_FIELD_COUNT;Iterator2++)
			if( toupper(FieldName) = WMA_FIELD_NAME[Iterator2] then
			        memcpy(Tag->tag[Iterator2] , FieldValue,WMA_MAX_STRING_SIZE * 2);
		*/
  	}
}

//  --------------------------------------------------------------------------- *

void ReadObject( PObjectID ID, FIL *hFile, PFileData Data)
{
	UINT br;
	//  Read data from header object if supported *
	if(memcmp(ID->obj, WMA_FILE_PROPERTIES_ID,16)==0)
	{
		//  Read file properties *
		if(f_lseek(hFile, hFile->fptr+80)!=FR_OK) return;
		if(f_read(hFile, &Data->MaxBitRate, 4, &br)!=FR_OK) return ;
	}
	if(memcmp(ID->obj, WMA_STREAM_PROPERTIES_ID,16)==0)
	{
		//  Read stream properties *
		if(f_lseek(hFile, hFile->fptr+60)!= FR_OK) return;
		if(f_read(hFile, &Data->Channels, 2 ,&br)!=FR_OK) return;
		if(f_read(hFile, &Data->SampleRate, 4, &br)!=FR_OK) return;
		if(f_read(hFile, &Data->ByteRate, 4, &br)!=FR_OK) return;
	}
	if(memcmp( ID->obj, WMA_CONTENT_DESCRIPTION_ID,16)==0)
	{
		//  Read standard tag data *
		if(f_lseek(hFile, hFile->fptr+4)!=FR_OK)return ;
		ReadTagStandard(hFile, &Data->Tag);
	}
	if(memcmp( ID->obj, WMA_EXTENDED_CONTENT_DESCRIPTION_ID,16 )==0)
	{
	 	//  Read extended tag data *
		if(f_lseek(hFile, hFile->fptr+4)!=FR_OK)return;
		ReadTagExtended(hFile, &Data->Tag);
	}
}


//  --------------------------------------------------------------------------- *

u8 ReadData( FIL *hFile, PFileData Data)
{
	ObjectID				ID;
	UINT Iterator, ObjectCount, ObjectSize, Position, br;

	//  Read file data *

	Data->FileSize = hFile->fsize;
	//  Check for existing header *
	if(f_read(hFile, &ID, sizeof(ID), &Position)!=FR_OK) return FALSE;
	if(Position!= sizeof(ID)) return FALSE;
	if(memcmp( ID.obj, WMA_HEADER_ID,16)==0)
	{
		if(f_lseek(hFile, hFile->fptr+8)!=FR_OK) return FALSE;
		if(f_read(hFile, &ObjectCount,  sizeof(ObjectCount), &br)!=FR_OK) return FALSE;
		if(f_lseek(hFile, hFile->fptr+2)!=FR_OK) return FALSE;
		//  Read all objects in header and get needed data *
		for( Iterator = 0; Iterator<ObjectCount;Iterator++)
		{
			Position = hFile->fptr;
			if(f_read(hFile, &ID, sizeof(ID), &br)!=FR_OK)return FALSE;
			if(f_read(hFile, &ObjectSize, sizeof(ObjectSize), &br)!=FR_OK) return FALSE;
			ReadObject(&ID, hFile, Data);
			if(f_lseek(hFile, Position + ObjectSize)!=FR_OK) return FALSE;
		}
	}
	return TRUE;
}


//  --------------------------------------------------------------------------- *

u8 IsValid(PFileData Data)
{
	//  Check for data validity *
	return	(Data->MaxBitRate > 0) && (Data->MaxBitRate < 320000) &&
			((Data->Channels == WMA_CM_MONO) || (Data->Channels == WMA_CM_STEREO)) &&
			(Data->SampleRate >= 8000) && (Data->SampleRate <= 96000) &&
			(Data->ByteRate > 0) && (Data->ByteRate < 40000);
}

//  --------------------------------------------------------------------------- *

int ExtractTrack(char *TrackString)
{
	//int		Value, Code;

	//  Extract track from string *
	return atoi(TrackString);

}

//  ********************** Private functions & procedures ********************* *

void wma_ResetData(PWMAfile self)
{
	memset(self,0,sizeof(TWMAfile));
}

//  --------------------------------------------------------------------------- *
char * wma_GetChannelMode(PWMAfile self)
{
	//  Get channel mode name *
	return WMA_MODE[self->FChannelModeID];
}

//  --------------------------------------------------------------------------- *

u8 wma_ReadFromFSFile(PWMAfile self,FIL *hFile)
{
	FileData			Data;
	u8			Result;

	//  Reset variables and load file data *
	wma_ResetData(self);
	memset(&Data, 0, sizeof(FileData));
	Result = ReadData(hFile, &Data);
	//  Process data if loaded and valid *
	if( Result && IsValid(&Data))
	{
		self->FValid = TRUE;
		//  Fill properties with loaded data *
		self->FFileSize = Data.FileSize;
		self->FChannelModeID = Data.Channels;
		self->FSampleRate = Data.SampleRate;
		self->FDuration = (Data.FileSize * 8 +Data.MaxBitRate-1)/ Data.MaxBitRate;
		self->FBitRate = Data.ByteRate * 8 / 1000;
		// self->FTrack = ExtractTrack(Data.Tag.tag[3]);
/*		self->FTitle = Data.Tag[1];
		self->FArtist = Data.Tag[2];
		self->FAlbum = Trim(Data.Tag[3]);
		self->FYear = Trim(Data.Tag[5]);
		self->FGenre = Trim(Data.Tag[6]);
		self->FComment = Trim(Data.Tag[7]);
*/
	}
	return Result;
}

u8 wma_ReadFromFile(PWMAfile self, const char *fname)
{
	FIL 		hFile;
	u8		res;
	
	if(f_open(&hFile, fname, FA_READ)!=FR_OK) return FALSE;
	res= wma_ReadFromFSFile(self, &hFile);
	f_close(&hFile);
	return res;
}
