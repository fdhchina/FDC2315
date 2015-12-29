#ifndef	_MPEGAUDIO_H_
#define _MPEGAUDIO_H_

#include "hal.h"

  // Xing/FhG VBR header data *
typedef struct  _VBRData {
    u8		Found;                                // True if VBR header found *
    char			ID[4];                   // Header ID: "Xing" or "VBRI" *
    int 			Frames;                                 // Total number of frames *
    int			Bytes;                                   // Total number of bytes *
    unsigned char	Scale;                                         // VBR scale (1..100) *
    char			VendorID[25];                                // Vendor ID (if present) *
}VBRData,*PVBRData;

  // MPEG frame header data*
typedef struct 	_FrameData	{
	u8			Found;                                     // True if frame found *
	int				Position;                           // Frame position in the file *
	unsigned short    	Size;                                          // Frame size (bytes) *
	u8			Xing;                                     // True if Xing encoder *
	unsigned char		Data[4];                 // The whole frame header data *
	unsigned char    	VersionID;                                        // MPEG version ID *
	unsigned char		LayerID;                                            // MPEG layer ID *
	u8			ProtectionBit;                        // True if protected by CRC *
	u16			BitRateID;                                            // Bit rate ID *
	u16			SampleRateID;                                      // Sample rate ID *
	u8			PaddingBit;                               // True if frame padded *
	u8			PrivateBit;                                  // Extra information *
	u8				ModeID;                                           // Channel mode ID *
	u8				ModeExtensionID;             // Mode extension ID (for Joint Stereo) *
	u8			CopyrightBit;                        // True if audio copyrighted *
	u8			OriginalBit;                            // True if original media *
	u8				EmphasisID;                                           // Emphasis ID *
}FrameData,*PFrameData;

#include "id3v1.h"
#include "id3v2.h"

// struct TMPEGaudio *
typedef struct  _TMPEGaudio {
// Private declarations *
	int 			FFileLength;
	char			FVendorID[25];
	VBRData		FVBR;
	FrameData	FFrame;
	u32		FDuration;
	TID3v1		FID3v1;
	TID3v2		FID3v2;
}TMPEGaudio,*PMPEGaudio;


u8 mp3_ReadFromFSFile(PMPEGaudio self, FIL *hFile);
u8 mp3_ReadFromFile( PMPEGaudio self, const char * FileName);

#endif
