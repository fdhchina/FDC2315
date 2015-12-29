
#include <string.h>
#include "hal.h"
#include "ff.h"
#include "wavfile.h"
  //  Used with ChannelModeID property *
#define	WAV_CM_MONO 			1                                      //  Index for mono mode *
#define	WAV_CM_STEREO 			2                                  //  Index for stereo mode *

  //  Channel mode names *
const char  WAV_MODE[3][10] = {
	"Unknown", 
	"Mono", 
	"Stereo"
};
/*
typedef struct 
{
   char szRiffID[4];   // 'R','I','F','F'
   u32 dwRiffSize;
   char szRiffFormat[4]; // 'W','A','V','E'
}RIFF_Head, *PRIFF_Head;

typedef struct 
{
   u16 wFormatTag;
   u16 wChannels;
   u32 dwSamplesPerSec;
   u32 dwAvgBytesPerSec;
   u16 wBlockAlign;
   u16 wBitsPerSample;
}WAVE_Format, *PWAVE_Format;

typedef struct 
{
   char   szFmtID[4]; // 'f','m','t',' '
   u32   dwFmtSize;
   WAVE_Format wavFormat;
}FMT_Block, *PFMT_Block;

typedef struct 
{
   char   szFactID[4]; // 'f','a','c','t'
   u32   dwFactSize;
}FACT_Block, *PFACT_Block;
*/
  //  Real structure of WAV file header *
typedef struct   _WAVRecord{
	//  RIFF file header *
	char			RIFFHeader[4];                        //  Must be "RIFF" *
	int			FileSize;                           //  Must be "RealFileSize - 8" *
	char			WAVEHeader[4];                        //  Must be "WAVE" *
	//  Format information *
	char			FormatHeader[4];                      //  Must be "fmt " *
	int			FormatSize;                               //  Must be 16 (decimal) *
	u16		FormatCode;                                             //  Must be 1 *
	u16		ChannelNumber;                                 //  Number of channels *
	int			SampleRate;                                   //  Sample rate (hz) *
	int			BytesPerSecond;                               //  Bytes per second *
	u16		BytesPerSample;                                  //  Bytes per sample *
	u16		BitsPerSample;                                    //  Bits per sample *
	//  Data area *
	char			DataHeader[4];                        //  Must be "data" *
	int			DataSize;                                            //  Data size *
}TWAVRecord,*PWAVRecord;



//  ********************* Auxiliary functions & procedures ******************** *

u8 ReadWAV(PWAVRecord WAVData,FIL *hFile)
{
	UINT Transferred;
	//  Set read-access and open file *
	if(!hFile) return FALSE;
	//  Read header *
	if(f_read(hFile, WAVData, sizeof(TWAVRecord), &Transferred)!=FR_OK)return FALSE;
	//  if transfer is not complete *
	return Transferred == 44;
}

//  --------------------------------------------------------------------------- *

u8 HeaderIsValid( PWAVRecord WAVData)
{
	u8		Result = TRUE;
	//  Validation *
	if(memcmp(WAVData->RIFFHeader,"RIFF", 4)!=0)	 Result = FALSE;
	if(memcmp(WAVData->WAVEHeader , "WAVE", 4)!=0) Result = FALSE;
	if(memcmp(WAVData->FormatHeader ,"fmt ", 4)!=0) Result = FALSE;
	if(	(WAVData->ChannelNumber != WAV_CM_MONO) &&
		(WAVData->ChannelNumber != WAV_CM_STEREO)	) Result = FALSE;
	return Result;
}


//  ********************** Private functions & procedures ********************* *

void wav_ResetData(PWAVfile self)
{
	memset(self,0,sizeof(TWAVfile));
/*	FValid = FALSE;
	FChannelModeID = 0;
	FSampleRate = 0;
	FBitsPerSample = 0;
	FFileSize = 0;
*/
}


//  --------------------------------------------------------------------------- *

char * wav_FGetChannelMode(PWAVfile self)
{
  return (char *)WAV_MODE[self->FChannelModeID];
}


//  --------------------------------------------------------------------------- *
void wav_GetDuration(PWAVfile self)
{
	u32	dwTmp;
	if( ( self->FValid) && self->FSampleRate && self->FBitsPerSample && self->FChannelModeID)
	{
		dwTmp= self->FSampleRate * self->FBitsPerSample * self->FChannelModeID;
		self->FDuration=((self->FFileSize - 44) * 8 +dwTmp) / dwTmp;
	}
	else
		self->FDuration=0;
}

//  --------------------------------------------------------------------------- *
u8 wav_ReadFromFSFile(PWAVfile self, FIL *hFile)
{
	TWAVRecord			WAVData;
	u8				Result;
	//  Reset and load header data from file to variable *
	wav_ResetData(self);
	if(!hFile)	return FALSE;
	Result = ReadWAV(&WAVData,hFile);
	//  Process data if loaded and header valid *
	if( Result && HeaderIsValid(&WAVData))
	{
		self->FValid = TRUE;
		//  Fill properties with header data *
		self->FChannelModeID = WAVData.ChannelNumber;
		self->FSampleRate = WAVData.SampleRate;
		self->FBitsPerSample = WAVData.BitsPerSample;
		self->FFileSize = WAVData.FileSize + 8;
		wav_GetDuration(self);
	}
	return Result;
}


u8 wav_ReadFromFile(PWAVfile self, const char *fname)
{
	FIL hFile;
	u8	Result;

	if(f_open(&hFile, fname, FA_READ)!=FR_OK) return FALSE;
	Result= wav_ReadFromFSFile(self, &hFile);
	f_close(&hFile);
	return Result;
}


