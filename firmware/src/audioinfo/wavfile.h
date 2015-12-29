#ifndef _WAVFILE_H_
#define _WAVFILE_H_


#include "hal.h"

  //  Class TWAVfile *
typedef struct  _TWAVfile{
	//  Private declarations *
	u8			FValid;
	u8				FChannelModeID;
 	u16			FSampleRate;
	u8				FBitsPerSample;
	u32			FFileSize;
	u32			FDuration;
}TWAVfile,*PWAVfile;


u8 wav_ReadFromFSFile(PWAVfile self, FIL *hFile);
u8 wav_ReadFromFile(PWAVfile self, const char *filename);

#endif
