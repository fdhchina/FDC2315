#include "hal.h"
#include	<string.h>
#include "ff.h"
#include "mpegaudio.h"

const unsigned short  MPEG_BIT_RATE[4][4][16] =
{
	// For MPEG 2.5 
	{	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0},
		{0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0},
		{0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0}
	},
	// Reserved 
	{	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	},
	// For MPEG 2 
	{	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0},
		{0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0},
		{0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0}
	},
	// For MPEG 1 
	{	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0},
		{0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 0},
		{0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0}
	}
};

// Sample rate codes 
#define  MPEG_SAMPLE_RATE_LEVEL_3  		0                                     // Level 3 
#define  MPEG_SAMPLE_RATE_LEVEL_2 		1                                     // Level 2 
#define  MPEG_SAMPLE_RATE_LEVEL_1 		2                                     // Level 1 
#define  MPEG_SAMPLE_RATE_UNKNOWN 		3 	                              // Unknown value 

// Table for sample rates 
const unsigned short  MPEG_SAMPLE_RATE[4][4] =
{
    {11025, 12000, 8000, 0},                                   	// For MPEG 2.5 
    {0, 0, 0, 0},                                                  	// Reserved 
    {22050, 24000, 16000, 0},                                   // For MPEG 2 
    {44100, 48000, 32000, 0}                                    // For MPEG 1 
};

// VBR header ID for Xing/FhG 
#define  VBR_ID_XING 	 	"Xing"                                         // Xing VBR ID
#define  VBR_ID_FHG 		 "VBRI"                                        // FhG VBR ID 

 // MPEG version codes 
#define   MPEG_VERSION_2_5			0                            // MPEG 2.5 
#define  MPEG_VERSION_UNKNOWN  		1                                 // Unknown version *
#define  MPEG_VERSION_2 				2                                                // MPEG 2 *
#define  MPEG_VERSION_1 				3                                                // MPEG 1 *

// MPEG version names *
const char  MPEG_VERSION[4][10] =
{
	"MPEG 2.5", 
	"MPEG ?", 
	"MPEG 2", 
	"MPEG 1"
};

  // MPEG layer codes *
#define  MPEG_LAYER_UNKNOWN 		0                                     // Unknown layer *
#define  MPEG_LAYER_III 				1                                     // Layer III *
#define  MPEG_LAYER_II 				2                                     // Layer II *
#define  MPEG_LAYER_I 				3                                     // Layer I *

  // MPEG layer names *
const char  MPEG_LAYER[4][10] =
{
	"Layer ?", 
	"Layer III", 
	"Layer II", 
	"Layer I"
};

  // Channel mode codes *
#define  MPEG_CM_STEREO 				0                                    // Stereo *
#define  MPEG_CM_JOINT_STEREO 		1                                    // Joint Stereo *
#define  MPEG_CM_DUAL_CHANNEL 		2                                    // Dual Channel *
#define  MPEG_CM_MONO				3                                    // Mono *
#define  MPEG_CM_UNKNOWN 			4                                    // Unknown mode *

  // Channel mode names *
  
const char  MPEG_CM_MODE[5][15] =
{
	"Stereo", 
	"Joint Stereo", 
	"Dual Channel", 
	"Mono", 
	"Unknown"
};

  // Extension mode codes (for Joint Stereo) *
#define  MPEG_CM_EXTENSION_OFF 		0                        // IS and MS modes set off *
#define  MPEG_CM_EXTENSION_IS 		1                        // Only IS mode set on *
#define  MPEG_CM_EXTENSION_MS 		2                        // Only MS mode set on *
#define  MPEG_CM_EXTENSION_ON 		3                        // IS and MS modes set on *
#define  MPEG_CM_EXTENSION_UNKNOWN 	4	                   // Unknown extension mode *

  // Emphasis mode codes *
#define  MPEG_EMPHASIS_NONE 			0                         // None *
#define  MPEG_EMPHASIS_5015 			1                         // 50/15 ms *
#define  MPEG_EMPHASIS_UNKNOWN 		2                         // Unknown emphasis *
#define  MPEG_EMPHASIS_CCIT 			3                         // CCIT J.17 *

  // Emphasis names *
const char  MPEG_EMPHASIS[4][10] =
{
	"None", 
	"50/15 ms", 
	"Unknown", 
	"CCIT J.17"
};

  // Encoder codes *
#define  MPEG_ENCODER_UNKNOWN 		0                                 // Unknown encoder *
#define  MPEG_ENCODER_XING 			1                                 // Xing *
#define  MPEG_ENCODER_FHG			2                                 // FhG *
#define  MPEG_ENCODER_LAME			3                                 // LAME *
#define  MPEG_ENCODER_BLADE			4                                 // Blade *
#define  MPEG_ENCODER_GOGO			5                                 // GoGo *
#define  MPEG_ENCODER_SHINE			6                                 // Shine *
#define  MPEG_ENCODER_QDESIGN		7                                 // QDesign *

  // Encoder names *
const char  MPEG_ENCODER[8][10] =
{
	"Unknown", 
	"Xing", 
	"FhG", 
	"LAME", 
	"Blade",
	"GoGo", 
	"Shine", 
	"QDesign"
};

  // Limitation ants *
#define  MAX_MPEG_FRAME_LENGTH 	1729                      // Max. MPEG frame length *
#define  MIN_MPEG_BIT_RATE 			8                                // Min. bit rate value *
#define  MAX_MPEG_BIT_RATE 		448                              // Max. bit rate value *
#define  MIN_ALLOWED_DURATION 	0.1                      // Min. song duration value *

  // VBR V}or ID strings *
#define  VENDOR_ID_LAME 	 		"LAME"                                // For LAME *
#define  VENDOR_ID_GOGO_NEW  	"GOGO"                              // For GoGo (New) *
#define  VENDOR_ID_GOGO_OLD 	"MPGE"                               // For GoGo (Old) *

// ********************* Auxiliary functions & procedures ******************** *
static u8 IsFrameHeader(u8 HeaderData[])
{
	// Check for valid frame header 
	if (	((HeaderData[0] & 0xFF) != 0xFF) 	||
	    	((HeaderData[1] & 0xE0) != 0xE0) 	||
    		(((HeaderData[1] >> 3) & 3) == 1) 	||
		(((HeaderData[1] >> 1) & 3)== 0)	||
		((HeaderData[2] & 0xF0) == 0xF0) 	||
		((HeaderData[2] & 0xF0) == 0) 		||
		(((HeaderData[2] >> 2) & 3)== 3)	||
		((HeaderData[3] & 3) == 2) 	)

		return FALSE;
	else
		return TRUE;
}

// --------------------------------------------------------------------------- *
static void DecodeHeader( u8 HeaderData[], FrameData *Frame)
{
	// Decode frame header data 
	memcpy(Frame->Data,HeaderData, sizeof(Frame->Data));
	Frame->VersionID = (HeaderData[1] >> 3) & 3;
	Frame->LayerID = (HeaderData[1] >> 1)& 3;
	Frame->ProtectionBit = (HeaderData[1] & 1) != 1;
	Frame->BitRateID = HeaderData[2] >> 4;
	Frame->SampleRateID = (HeaderData[2] >> 2) & 3;
	Frame->PaddingBit = ((HeaderData[2] >> 1) & 1) ==1;
	Frame->PrivateBit = (HeaderData[2] & 1) == 1;
	Frame->ModeID = (HeaderData[3] >> 6) & 3;
	Frame->ModeExtensionID = (HeaderData[3] >> 4) & 3;
	Frame->CopyrightBit = ((HeaderData[3] >> 3) & 1) == 1;
	Frame->OriginalBit = ((HeaderData[3] >> 2) & 1) == 1;
	Frame->EmphasisID = HeaderData[3] & 3;
}

// --------------------------------------------------------------------------- *
static u8  ValidFrameAt( u16 Index,u8 Data[])
{
	u8	  HeaderData[4];
	
	
	// Check for frame at given position 
	HeaderData[0] = Data[Index];
	HeaderData[1] = Data[Index + 1];
	HeaderData[2] = Data[Index + 2];
	HeaderData[3] = Data[Index + 3];
	return	IsFrameHeader(HeaderData);
}

// --------------------------------------------------------------------------- *
u8 GetCoefficient(PFrameData Frame)
{
	u8 Rt;
	// Get frame size coefficient *
	if (Frame->VersionID == MPEG_VERSION_1)
		if (Frame->LayerID == MPEG_LAYER_I) Rt = 48;
		else Rt = 144;
	else
		if (Frame->LayerID == MPEG_LAYER_I)	Rt = 24;
		else if (Frame->LayerID == MPEG_LAYER_II) Rt = 144;
			else Rt = 72;
	return Rt;
}

// --------------------------------------------------------------------------- *
u16 GetBitRate( PFrameData Frame)
{
	// Get bit rate 
	return MPEG_BIT_RATE[Frame->VersionID][Frame->LayerID][Frame->BitRateID];
}

// --------------------------------------------------------------------------- 

u16 GetSampleRate( PFrameData Frame)
{
	// Get sample rate *
	return MPEG_SAMPLE_RATE[Frame->VersionID][Frame->SampleRateID];
}

// --------------------------------------------------------------------------- *
u8 GetPadding( PFrameData Frame)
{
	u8	Rt;
	// Get frame padding *
	if (Frame->PaddingBit )
		if(Frame->LayerID == MPEG_LAYER_I) Rt = 4;
		else Rt = 1;
	else Rt = 0;

	return Rt;
}

// --------------------------------------------------------------------------- *
u16 GetFrameLength( PFrameData Frame)
{

	u16  Coefficient, BitRate, SampleRate, Padding;

	// Calculate MPEG frame length *
	Coefficient = GetCoefficient(Frame);
	BitRate = GetBitRate(Frame);
	SampleRate = GetSampleRate(Frame);
	Padding = GetPadding(Frame);
	return Coefficient * BitRate * 1000 / SampleRate + Padding;
}

// --------------------------------------------------------------------------- *
u8 IsXing( u16 Index, u8 * Data)
{
	// Get true if Xing encoder *
	return
		    (Data[Index] == 0) &&
		    (Data[Index + 1] == 0) &&
		    (Data[Index + 2] == 0) &&
		    (Data[Index + 3] == 0) &&
		    (Data[Index + 4] == 0) &&
		    (Data[Index + 5] == 0);
}

// --------------------------------------------------------------------------- *
void GetXingInfo( u16 Index, u8 *Data, PVBRData Result)
{
	// Extract Xing VBR info at given position 
	memset(Result,0, sizeof(VBRData));
	Result->Found = TRUE;
	memcpy(Result->ID , VBR_ID_XING,4);
	Result->Frames =
		((u32)Data[Index + 8] <<24)+
		(Data[Index + 9] <<16) +
		(Data[Index + 10] <<8) +
		Data[Index + 11];
	Result->Bytes =
		((u32)Data[Index + 12] << 24)+
		(Data[Index + 13] <<16)+
		(Data[Index + 14]<<8)+
		Data[Index + 15];
	Result->Scale = Data[Index + 119];
	// Vendor ID can be not present *
	Result->VendorID[0] =	Data[Index + 120];
	Result->VendorID[1] =	Data[Index + 121];
	Result->VendorID[2] =	Data[Index + 122];
	Result->VendorID[3] =	Data[Index + 123];
	Result->VendorID[4] =	Data[Index + 124];
	Result->VendorID[5] =	Data[Index + 125];
	Result->VendorID[6] =	Data[Index + 126];
	Result->VendorID[7] =	Data[Index + 127];
	Result->VendorID[8] = 0;
}

// --------------------------------------------------------------------------- *
void GetFhGInfo( u16 Index, u8 *Data,PVBRData Result)
{
	// Extract FhG VBR info at given position *
	memset(Result, 0,sizeof(VBRData));
	Result->Found = TRUE;
	memcpy(Result->ID , VBR_ID_FHG,4);
	Result->Scale = Data[Index + 9];
	Result->Bytes =
		((u32)Data[Index + 10] <<24) +
		(Data[Index + 11]<< 16) +
		(Data[Index + 12] <<8) +
		Data[Index + 13];
	Result->Frames =
		((u32)Data[Index + 14] << 24) +
		(Data[Index + 15] << 16) +
		(Data[Index + 16] <<8) +
		Data[Index + 17];
}


// --------------------------------------------------------------------------- *
void FindVBR( u16 Index, u8 *Data, PVBRData Result)
{
	// Check for VBR header at given position 
	memset(Result, 0,sizeof(VBRData));
	if(memcmp((u8 *)&Data[Index], VBR_ID_XING,4)==0) GetXingInfo(Index, Data,Result);
	if(memcmp((u8 *)&Data[Index], VBR_ID_FHG,4)==0)  GetFhGInfo(Index, Data,Result);
}

// --------------------------------------------------------------------------- *
u8 GetVBRDeviation( PFrameData Frame)
{
	u8 Result;
	// Calculate VBR deviation *
	if (Frame->VersionID == MPEG_VERSION_1)
		if( Frame->ModeID != MPEG_CM_MONO)  Result = 36;
		else Result = 21;
	else
		if( Frame->ModeID != MPEG_CM_MONO) Result = 21;
		else Result = 13;
	return Result;
}

// --------------------------------------------------------------------------- *
void FindFrame( u8 *Data,PVBRData VBR,PFrameData Result)
{

	u8  HeaderData[4];
	int  Iterator;

	 // Search for valid frame *
	memset(Result, 0,sizeof(FrameData));
	memcpy( HeaderData, Data, 4);
	for( Iterator = 0 ;Iterator<MAX_MPEG_FRAME_LENGTH;Iterator++)
	{
		// Decode data if frame header found *
		if(IsFrameHeader(HeaderData))
		{
			DecodeHeader(HeaderData, Result);
			// Check for next frame and try to find VBR header 
			if( ValidFrameAt(Iterator + GetFrameLength(Result), Data))
		      {
				Result->Found = TRUE;
				Result->Position = Iterator;
				Result->Size = GetFrameLength(Result);
				Result->Xing = IsXing(Iterator + sizeof(HeaderData), Data);
				FindVBR(Iterator + GetVBRDeviation(Result), Data,VBR);
			        break;
		      }
		}
		// Prepare next data block *
		HeaderData[0] = HeaderData[1];
		HeaderData[1] = HeaderData[2];
		HeaderData[2] = HeaderData[3];
		HeaderData[3] = Data[Iterator + sizeof(HeaderData)];
	}
}


// --------------------------------------------------------------------------- *
/*
void FindVendorID( Data: array of Byte; Size: u16): string;
var
  Iterator: Integer;
  V}orID: string;
{
  // Search for v}or ID *
  Result = "";
  if (sizeof(Data) - Size - 8) < 0 then Size = sizeof(Data) - 8;
  for Iterator = 0 to Size do
  {
    V}orID =
      Chr(Data[sizeof(Data) - Iterator - 8]) +
      Chr(Data[sizeof(Data) - Iterator - 7]) +
      Chr(Data[sizeof(Data) - Iterator - 6]) +
      Chr(Data[sizeof(Data) - Iterator - 5]);
    if V}orID = V}OR_ID_LAME then
    {
      Result = V}orID +
        Chr(Data[sizeof(Data) - Iterator - 4]) +
        Chr(Data[sizeof(Data) - Iterator - 3]) +
        Chr(Data[sizeof(Data) - Iterator - 2]) +
        Chr(Data[sizeof(Data) - Iterator - 1]);
      break;
    };
    if V}orID = V}OR_ID_GOGO_NEW then
    {
      Result = V}orID;
      break;
    };
  };
};
*/

// ********************** Private functions & procedures ********************* *
void FResetData(PMPEGaudio self)
{
	// Reset all variables *
	self->FFileLength = 0;
	self->FVendorID[0] = 0;
	memset(&self->FVBR, 0,sizeof(VBRData));
	memset(&self->FFrame, 0,sizeof(FrameData));
	self->FFrame.VersionID = MPEG_VERSION_UNKNOWN;
	self->FFrame.SampleRateID = MPEG_SAMPLE_RATE_UNKNOWN;
	self->FFrame.ModeID = MPEG_CM_UNKNOWN;
	self->FFrame.ModeExtensionID = MPEG_CM_EXTENSION_UNKNOWN;
	self->FFrame.EmphasisID = MPEG_EMPHASIS_UNKNOWN;
	self->FDuration=0;
	ID3v1_ResetData(&self->FID3v1);
	ID3v2_ResetData(&self->FID3v2);
}


// --------------------------------------------------------------------------- *

char * FGetVersion(PMPEGaudio self)
{
  	// Get MPEG version name *
	return (char *)MPEG_VERSION[self->FFrame.VersionID];
}


// --------------------------------------------------------------------------- *
char *FGetLayer(PMPEGaudio self)
{
	// Get MPEG layer name *
	return (char *)MPEG_LAYER[self->FFrame.LayerID];
}

// --------------------------------------------------------------------------- *

u16 FGetBitRate(PMPEGaudio self)
{
	u16 Result;
	// Get bit rate, calculate average bit rate if VBR header found *
	if ((self->FVBR.Found) && (self->FVBR.Frames > 0))
		Result = (self->FVBR.Bytes / self->FVBR.Frames - GetPadding(&self->FFrame)) *
		      GetSampleRate(&self->FFrame) / GetCoefficient(&self->FFrame) / 1000;
	else
		Result = GetBitRate(&self->FFrame);
	return Result;
}

// --------------------------------------------------------------------------- *
u16 FGetSampleRate(PMPEGaudio self)
{
	// Get sample rate *
	return GetSampleRate(&self->FFrame);
}


// --------------------------------------------------------------------------- *

char * mp3_FGetChannelMode(PMPEGaudio self)
{
	// Get channel mode name *
	return (char *)MPEG_CM_MODE[self->FFrame.ModeID];
}


// --------------------------------------------------------------------------- *

char * FGetEmphasis(PMPEGaudio self)  
{
  // Get emphasis name *
  return (char *)MPEG_EMPHASIS[self->FFrame.EmphasisID];
}

// --------------------------------------------------------------------------- *

int FGetFrames(PMPEGaudio self)
{

	int  MPEGSize,Result;

	// Get total number of frames, calculate if VBR header not found *
	if(self->FVBR.Found)
		Result = self->FVBR.Frames;
	else
	{
		if( self->FID3v1.FExists)  MPEGSize = self->FFileLength - self->FID3v2.FSize- 128;
		else MPEGSize = self->FFileLength - self->FID3v2.FSize;
		Result = (MPEGSize - self->FFrame.Position) / GetFrameLength(&self->FFrame);
	}
	return Result;
}


/*
// --------------------------------------------------------------------------- *

function TMPEGaudio.FGetVBREncoderID: Byte;
{
  // Guess VBR encoder and get ID *
  Result = 0;
  if Copy(FVBR.V}orID, 1, 4) = V}OR_ID_LAME then
    Result = MPEG_ENCODER_LAME;
  if Copy(FVBR.V}orID, 1, 4) = V}OR_ID_GOGO_NEW then
    Result = MPEG_ENCODER_GOGO;
  if Copy(FVBR.V}orID, 1, 4) = V}OR_ID_GOGO_OLD then
    Result = MPEG_ENCODER_GOGO;
  if (FVBR.ID = VBR_ID_XING) and
    (Copy(FVBR.V}orID, 1, 4) <> V}OR_ID_LAME) and
    (Copy(FVBR.V}orID, 1, 4) <> V}OR_ID_GOGO_NEW) and
    (Copy(FVBR.V}orID, 1, 4) <> V}OR_ID_GOGO_OLD) then
    Result = MPEG_ENCODER_XING;
  if FVBR.ID = VBR_ID_FHG then
    Result = MPEG_ENCODER_FHG;
};

// --------------------------------------------------------------------------- *

function TMPEGaudio.FGetCBREncoderID: Byte;
{
  // Guess CBR encoder and get ID *
  Result = MPEG_ENCODER_FHG;
  if (FFrame.OriginalBit) and
    (FFrame.ProtectionBit) then
    Result = MPEG_ENCODER_LAME;
  if (GetBitRate(FFrame) <= 160) and
    (FFrame.ModeID = MPEG_CM_STEREO) then
    Result = MPEG_ENCODER_BLADE;
  if (FFrame.CopyrightBit) and
    (FFrame.OriginalBit) and
    (not FFrame.ProtectionBit) then
    Result = MPEG_ENCODER_XING;
  if (FFrame.Xing) and
    (FFrame.OriginalBit) then
    Result = MPEG_ENCODER_XING;
  if FFrame.LayerID = MPEG_LAYER_II then
    Result = MPEG_ENCODER_QDESIGN;
  if (FFrame.ModeID = MPEG_CM_DUAL_CHANNEL) and
    (FFrame.ProtectionBit) then
    Result = MPEG_ENCODER_SHINE;
  if Copy(FV}orID, 1, 4) = V}OR_ID_LAME then
    Result = MPEG_ENCODER_LAME;
  if Copy(FV}orID, 1, 4) = V}OR_ID_GOGO_NEW then
    Result = MPEG_ENCODER_GOGO;
};

// --------------------------------------------------------------------------- *

function TMPEGaudio.FGetEncoderID: Byte;
{
  // Get guessed encoder ID *
  if FFrame.Found then
    if FVBR.Found then Result = FGetVBREncoderID
    else Result = FGetCBREncoderID
  else
    Result = 0;
};

// --------------------------------------------------------------------------- *

function TMPEGaudio.FGetEncoder: string;
var
  V}orID: string;
{
  // Get guessed encoder name and encoder version for LAME *
  Result = MPEG_ENCODER[FGetEncoderID];
  if FVBR.V}orID <> "" then V}orID = FVBR.V}orID;
  if FV}orID <> "" then V}orID = FV}orID;
  if (FGetEncoderID = MPEG_ENCODER_LAME) and
    (Length(V}orID) >= 8) and
    (V}orID[5] in ["0".."9"]) and
    (V}orID[6] = ".") and
    (V}orID[7] in ["0".."9"]) and
    (V}orID[8] in ["0".."9"]) then
    Result =
      Result + #32 +
      V}orID[5] +
      V}orID[6] +
      V}orID[7] +
      V}orID[8];
};

// --------------------------------------------------------------------------- *

function TMPEGaudio.FGetValid: u8;
{
  // Check for right MPEG file data *
  Result =
    (FFrame.Found) and
    (FGetBitRate >= MIN_MPEG_BIT_RATE) and
    (FGetBitRate <= MAX_MPEG_BIT_RATE) and
    (FGetDuration >= MIN_ALLOWED_DURATION);
};

// ********************** Public functions & procedures ********************** *

ructor TMPEGaudio.Create;
{
  // Object ructor *
  inherited;
  FID3v1 = TID3v1.Create;
  FID3v2 = TID3v2.Create;
  FResetData;
};

// --------------------------------------------------------------------------- *

destructor TMPEGaudio.Destroy;
{
  // Object destructor *
  FID3v1.Free;
  FID3v2.Free;
  inherited;
};
*/

void mp3_GetDuration(PMPEGaudio self)
{
	int				MPEGSize;
	u32	dwTmp;

	self->FDuration=0;
	if (self->FFrame.Found )
	{
		if( (self->FVBR.Found) && (self->FVBR.Frames > 0))
		{
			dwTmp= GetSampleRate(&self->FFrame);
			if(dwTmp)
				self->FDuration=(self->FVBR.Frames * GetCoefficient(&self->FFrame) * 8+dwTmp-1)/   dwTmp;
		}else
		{
			if(self->FID3v1.FExists) 
				MPEGSize = self->FFileLength - self->FID3v2.FSize - 128;
			else 
			  	MPEGSize = self->FFileLength - self->FID3v2.FSize;
			dwTmp= GetBitRate(&self->FFrame) * 1000;
			if(dwTmp)
				self->FDuration= ((MPEGSize - self->FFrame.Position) * 8+dwTmp-1)/ dwTmp;
		}
	}
}

// --------------------------------------------------------------------------- *
u8 mp3_ReadFromFSFile(PMPEGaudio self, FIL *hFile)
{
	u8			Data[MAX_MPEG_FRAME_LENGTH * 2];
	UINT			Transferred;
	u8 		Result=FALSE;

	FResetData(self);
	if(!hFile) return FALSE;
	// At first search for tags, then search for a MPEG frame and VBR data *
	if(ID3v2_ReadFromFile(&self->FID3v2, hFile) && ID3v1_ReadFromFile(&self->FID3v1, hFile))
	{
	      // Open file, read first block of data and search for a frame *
		self->FFileLength = hFile->fsize;
		f_lseek(hFile, self->FID3v2.FSize);
		if(f_read(hFile, Data, sizeof(Data), &Transferred)!=FR_OK) return FALSE;
		FindFrame(Data, &self->FVBR,&self->FFrame);
		// Try to search in the middle if no frame at the {ning found *
		if( (! self->FFrame.Found) && (Transferred == sizeof(Data)))
		{
			f_lseek(hFile, (self->FFileLength - self->FID3v2.FSize) / 2);
			f_read(hFile, Data, sizeof(Data), &Transferred);
			FindFrame(Data, &self->FVBR,&self->FFrame);
		}
		// Search for v}or ID at the } if CBR encoded *
		if( (self->FFrame.Found) && (! self->FVBR.Found) )
		{
			if( ! self->FID3v1.FExists)  
				f_lseek(hFile, self->FFileLength - sizeof(Data));
			else 
				f_lseek(hFile, self->FFileLength - sizeof(Data) - 128);
			if(f_read(hFile, Data, sizeof(Data), &Transferred)!=FR_OK) return FALSE;
//		        self->FVendorID = FindVendorID(Data, FFrame.Size * 5);
		}
	      Result = TRUE;
	}
	if(Result)	mp3_GetDuration(self);
	if(! self->FFrame.Found)	FResetData(self);
	return Result;
}


// --------------------------------------------------------------------------- *
u8 mp3_ReadFromFile(PMPEGaudio self, const char *fname)
{
	FIL			hFile;
	u8			res=FALSE;

	if(f_open(&hFile, fname, FA_READ)!=FR_OK) return FALSE;
	res= mp3_ReadFromFSFile(self, &hFile);
	f_close(&hFile);
	return res;
}


