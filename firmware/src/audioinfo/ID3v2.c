
#include <string.h>
#include "hal.h"
#include "ff.h"
#include "mpegaudio.h"

#define  	TAG_VERSION_2_2 		2                               //  Code for ID3v2.2.x tag *
#define  	TAG_VERSION_2_3 		3                               //  Code for ID3v2.3.x tag *
#define	TAG_VERSION_2_4		4                               //  Code for ID3v2.4.x tag *


  //  Class TID3v2 *

//  ID3v2 tag ID *
#define		ID3V2_ID 	 "ID3"

//  Max. number of supported tag frames *
#define		ID3V2_FRAME_COUNT  16

//  Names of supported tag frames (ID3v2.3.x & ID3v2.4.x) *
char  ID3V2_FRAME_NEW[ID3V2_FRAME_COUNT][5]=
{	"TIT2", "TPE1", "TALB", "TRCK", "TYER", "TCON", "COMM", "TCOM", "TENC",
     "TCOP", "TLAN", "WXXX", "TDRC", "TOPE", "TIT1", "TOAL"};

  //  Names of supported tag frames (ID3v2.2.x) *
char  ID3V2_FRAME_OLD[ID3V2_FRAME_COUNT][4]=
{"TT2", "TP1", "TAL", "TRK", "TYE", "TCO", "COM", "TCM", "TEN",
     "TCR", "TLA", "WXX", "TOR", "TOA", "TT1", "TOT"};

  //  Max. tag size for saving *
#define  ID3V2_MAX_SIZE 		4096

  //  Unicode ID *
#define		UNICODE_ID		1

  //  Frame header (ID3v2.3.x & ID3v2.4.x) *
typedef struct  _FrameHeaderNew{
	char    	ID[4];                                      //  Frame ID *
	int		Size;                                    //  Size excluding header *
	u16	Flags;                                                      //  Flags *
  }FrameHeaderNew,*PFrameHeaderNew;

  //  Frame header (ID3v2.2.x) *
typedef struct  _FrameHeaderOld{
	char     	ID[3];                                      //  Frame ID *
	u8		Size[3];                       //  Size excluding header *
}FrameHeaderOld,*PFrameHeaderOld;

  //  ID3v2 header data - for internal use *
typedef struct  _TagInfo {
	//  Real structure of ID3v2 header *
	char			ID[3];                                  //  Always "ID3" *
	u8			Version;                                           //  Version number *
	u8			Revision;                                         //  Revision number *
	u8			Flags;                                               //  Flags of tag *
	u8			Size[4];                   //  Tag size excluding header *
	//  Extended data *
	int			FileSize;                                    //  File size (u8s) *
	char			Frame[ID3V2_FRAME_COUNT-1][15];  //  Information from frames *
	u8		NeedRewrite;                           //  Tag should be rewritten *
	int			PaddingSize;                              //  Padding size (u8s) *
}TagInfo,*PTagInfo;


//  ********************* Auxiliary functions & procedures ******************** *
u8 ReadHeader( FIL *hFile, PTagInfo Tag)
{
	UINT iLen;
	//  Set read-access and open file *
	if(hFile)
	{
		//  Read header and get file size *
		if(f_read(hFile, Tag, 10, &iLen)!=FR_OK) return FALSE;
		Tag->FileSize = hFile->fsize;
		//  if transfer is not complete *
		if (iLen < 10) return FALSE;
		return TRUE;
	}
	return FALSE;
}


//  --------------------------------------------------------------------------- *
int GetTagSize( PTagInfo Tag)
{
	int Result;
	//  Get total tag size *
	Result=  Tag->Size[0] * 0x200000 +
		    Tag->Size[1] * 0x4000 +
		    Tag->Size[2] * 0x80 +
		    Tag->Size[3] + 10;
	if ((Tag->Flags & 0x10) == 0x10) Result+=10;
	if( Result > Tag->FileSize) Result = 0;

	return Result;
}


//  --------------------------------------------------------------------------- *
/*
void SetTagItem(char * ID, char *Data, PTagInfo Tag)
{
	u8			Iterator;
	char			FrameID[25];

	//  Set tag item if supported frame found *
	for( Iterator = 0;Iterator<ID3V2_FRAME_COUNT;Iterator++)
	{
		if( Tag->Version > TAG_VERSION_2_2)
			FrameID = ID3V2_FRAME_NEW[Iterator];
		else
			FrameID = ID3V2_FRAME_OLD[Iterator];
		if( (memcmp(FrameID ,ID)==0) && (Data[0] <= UNICODE_ID))
			memcpy(Tag->Frame[Iterator], Data,14);
	}
}
*/

//  --------------------------------------------------------------------------- *

int Swap32( int Figure)
{

	TSize size,size2;

	size.l=Figure;
	size2.b[0]= size.b[3];
	size2.b[1]= size.b[2];
	size2.b[2]= size.b[1];
	size2.b[3]= size.b[0];
	//  Swap 4 bytes *
 	return    size2.l;
}

//  --------------------------------------------------------------------------- *

void ReadFramesNew( FIL *hFile, PTagInfo Tag)
{
	FrameHeaderNew		Frame;
	char 				Data[500];
	UINT	DataPosition, DataSize, br;
	//  Get information from frames (ID3v2.3.x & ID3v2.4.x) *
	//  Set read-access, open file *
	if(!hFile) return;
	f_lseek(hFile, 10);
	while( (hFile->fptr< GetTagSize(Tag)) && (hFile->fptr<hFile->fsize))
	{
		memset(Data,  0, 500);
		//  Read frame header and check frame ID *
		if(f_read(hFile, &Frame, 10, &br)!=FR_OK) return;
		if((Frame.ID[0]<'A') ||(Frame.ID[0]>'Z')) break;
		//  Note data position and determine significant data size *
		DataPosition = hFile->fptr;
		DataSize= Swap32(Frame.Size);
		if(DataSize > 500) DataSize = 500;
		//  Read frame data and set tag item if frame supported *
		if(f_read(hFile, Data, DataSize, &br)!=FR_OK) return ;
		//if(( Frame.Flags & 0x8000) != 0x8000)  SetTagItem(Frame.ID, Data, Tag);
		f_lseek(hFile, hFile->fptr+DataPosition + Swap32(Frame.Size));
	}
}

//  --------------------------------------------------------------------------- *

void ReadFramesOld( FIL *hFile, PTagInfo Tag)
{
	FrameHeaderOld		Frame;
	char 				Data[500];
	UINT				DataPosition, DataSize,FrameSize;
	UINT				br;

	//  Get information from frames (ID3v2.2.x) *
	//  Set read-access, open file *
	if(!hFile) return;
	f_lseek(hFile, 10);
	while( (hFile->fptr< GetTagSize(Tag)) && (hFile->fptr<hFile->fsize))
	{
		memset(Data,  0, 500);
		//  Read frame header and check frame ID *
		if(f_read(hFile, &Frame, 6, &br)!=FR_OK) return ;
		if((Frame.ID[0]<'A') ||(Frame.ID[0]>'Z')) break;
		//  Note data position and determine significant data size *
		DataPosition = hFile->fptr;
		FrameSize = ((u32)Frame.Size[0] << 16) + (Frame.Size[1] << 8) + Frame.Size[2];
		DataSize=( FrameSize > 500)?500:FrameSize;
		//  Read frame data and set tag item if frame supported *
		if(f_read(hFile, Data, DataSize, &br)!=FR_OK) return;
		//SetTagItem(Frame.ID, Data, Tag);
		f_lseek(hFile, hFile->fptr+DataPosition + FrameSize);
	}
}

/*
//  --------------------------------------------------------------------------- *
void GetANSI( char *Source,char *Result)
{

	int				Index;
	Byte				FirstByte, SecondByte;
	  UnicodeChar: WideChar;
{
  //  Convert string from unicode if needed and trim spaces *
  if (Length(Source) > 0) and (Source[1] = UNICODE_ID) then
  {
    Result = "";
    for Index = 1 to ((Length(Source) - 1) div 2) do
    {
      FirstByte = Ord(Source[Index * 2]);
      SecondByte = Ord(Source[Index * 2 + 1]);
      UnicodeChar = WideChar(FirstByte or (SecondByte shl 8));
      if UnicodeChar = #0 then break;
      if FirstByte < $FF then Result = Result + UnicodeChar;
    }
    Result = Trim(Result);
  }
  else
    Result = Trim(Source);
}

//  --------------------------------------------------------------------------- *

function GetContent( Content1, Content2: string): string;
{
  //  Get content preferring the first content *
  Result = GetANSI(Content1);
  if Result = "" then Result = GetANSI(Content2);
}

//  --------------------------------------------------------------------------- *

function ExtractTrack( TrackString: string): u16;
var
  Track: string;
  Index, Value, Code: Integer;
{
  //  Extract track from string *
  Track = GetANSI(TrackString);
  Index = Pos("/", Track);
  if Index = 0 then Val(Track, Value, Code)
  else Val(Copy(Track, 1, Index - 1), Value, Code);
  if Code = 0 then Result = Value
  else Result = 0;
}

//  --------------------------------------------------------------------------- *

function ExtractYear( YearString, DateString: string): string;
{
  //  Extract year from strings *
  Result = GetANSI(YearString);
  if Result = "" then Result = Copy(GetANSI(DateString), 1, 4);
}

//  --------------------------------------------------------------------------- *

function ExtractGenre( GenreString: string): string;
{
  //  Extract genre from string *
  Result = GetANSI(GenreString);
  if Pos(")", Result) > 0 then Delete(Result, 1, LastDelimiter(")", Result));
}

//  --------------------------------------------------------------------------- *

function ExtractText( SourceString: string; LanguageID: u8): string;
var
  Source, Separator: string;
  EncodingID: Char;
{
  //  Extract significant text data from a complex field *
  Source = SourceString;
  Result = "";
  if Length(Source) > 0 then
  {
    EncodingID = Source[1];
    if EncodingID = UNICODE_ID then Separator = #0#0
    else Separator = #0;
    if LanguageID then  Delete(Source, 1, 4)
    else Delete(Source, 1, 1);
    Delete(Source, 1, Pos(Separator, Source) + Length(Separator) - 1);
    Result = GetANSI(EncodingID + Source);
  }
}

//  --------------------------------------------------------------------------- *

procedure BuildHeader(var Tag: TagInfo);
var
  Iterator, TagSize: Integer;
{
  //  Calculate new tag size (without padding) *
  TagSize = 10;
  for Iterator = 1 to ID3V2_FRAME_COUNT do
    if Tag.Frame[Iterator] <> "" then
      Inc(TagSize, Length(Tag.Frame[Iterator]) + 11);
  //  Check for ability to change existing tag *
  Tag.NeedRewrite =
    (Tag.ID <> ID3V2_ID) or
    (GetTagSize(Tag) < TagSize) or
    (GetTagSize(Tag) > ID3V2_MAX_SIZE);
  //  Calculate padding size and set padded tag size *
  if Tag.NeedRewrite then Tag.PaddingSize = ID3V2_MAX_SIZE - TagSize
  else Tag.PaddingSize = GetTagSize(Tag) - TagSize;
  if Tag.PaddingSize > 0 then Inc(TagSize, Tag.PaddingSize);
  //  Build tag header *
  Tag.ID = ID3V2_ID;
  Tag.Version = TAG_VERSION_2_3;
  Tag.Revision = 0;
  Tag.Flags = 0;
  //  Convert tag size *
  for Iterator = 1 to 4 do
    Tag.Size[Iterator] = ((TagSize - 10) shr ((4 - Iterator) * 7)) and $7F;
}

//  --------------------------------------------------------------------------- *

function ReplaceTag( FileName: string; TagData: TStream): u8;
var
  Destination: TFileStream;
{
  //  Replace old tag with new tag data *
  Result = FALSE;
  if (not FileExists(FileName)) or (FileSetAttr(FileName, 0) <> 0) then exit;
  try
    TagData.Position = 0;
    Destination = TFileStream.Create(FileName, fmOpenReadWrite);
    Destination.CopyFrom(TagData, TagData.Size);
    Destination.Free;
    Result = TRUE;
  except
    //  Access error *
  }
}

//  --------------------------------------------------------------------------- *

function RebuildFile( FileName: string; TagData: TStream): u8;
var
  Tag: TagInfo;
  Source, Destination: TFileStream;
  BufferName: string;
{
  //  Rebuild file with old file data and new tag data (optional) *
  Result = FALSE;
  if (not FileExists(FileName)) or (FileSetAttr(FileName, 0) <> 0) then exit;
  if not ReadHeader(FileName, Tag) then exit;
  if (TagData = nil) and (Tag.ID <> ID3V2_ID) then exit;
  try
    //  Create file streams *
    BufferName = FileName + "~";
    Source = TFileStream.Create(FileName, fmOpenRead);
    Destination = TFileStream.Create(BufferName, fmCreate);
    //  Copy data blocks *
    if Tag.ID = ID3V2_ID then Source.Seek(GetTagSize(Tag), soFrom{ning);
    if TagData <> nil then Destination.CopyFrom(TagData, 0);
    Destination.CopyFrom(Source, Source.Size - Source.Position);
    //  Free resources *
    Source.Free;
    Destination.Free;
    //  Replace old file and delete temporary file *
    if (DeleteFile(FileName)) and (RenameFile(BufferName, FileName)) then
      Result = TRUE
    else
      raise Exception.Create("");
  except
    //  Access error *
    if FileExists(BufferName) then DeleteFile(BufferName);
  }
}

//  --------------------------------------------------------------------------- *

function SaveTag( FileName: string; Tag: TagInfo): u8;
var
  TagData: TStringStream;
  Iterator, FrameSize: Integer;
  Padding: array [1..ID3V2_MAX_SIZE] of Byte;
{
  //  Build and write tag header and frames to stream *
  TagData = TStringStream.Create("");
  BuildHeader(Tag);
  TagData.Write(Tag, 10);
  for Iterator = 1 to ID3V2_FRAME_COUNT do
    if Tag.Frame[Iterator] <> "" then
    {
      TagData.WriteString(ID3V2_FRAME_NEW[Iterator]);
      FrameSize = Swap32(Length(Tag.Frame[Iterator]) + 1);
      TagData.Write(FrameSize, SizeOf(FrameSize));
      TagData.WriteString(#0#0#0 + Tag.Frame[Iterator]);
    }
  //  Add padding *
  FillChar(Padding, SizeOf(Padding), 0);
  if Tag.PaddingSize > 0 then TagData.Write(Padding, Tag.PaddingSize);
  //  Rebuild file or replace tag with new tag data *
  if Tag.NeedRewrite then Result = RebuildFile(FileName, TagData)
  else Result = ReplaceTag(FileName, TagData);
  TagData.Free;
}

//  ********************** Private functions & procedures ********************* *

procedure TID3v2.FSetTitle( NewTitle: string);
{
  //  Set song title *
  FTitle = Trim(NewTitle);
}

//  --------------------------------------------------------------------------- *

procedure TID3v2.FSetArtist( NewArtist: string);
{
  //  Set artist name *
  FArtist = Trim(NewArtist);
}

//  --------------------------------------------------------------------------- *

procedure TID3v2.FSetAlbum( NewAlbum: string);
{
  //  Set album title *
  FAlbum = Trim(NewAlbum);
}

//  --------------------------------------------------------------------------- *

procedure TID3v2.FSetTrack( NewTrack: u16);
{
  //  Set track number *
  FTrack = NewTrack;
}

//  --------------------------------------------------------------------------- *

procedure TID3v2.FSetYear( NewYear: string);
{
  //  Set release year *
  FYear = Trim(NewYear);
}

//  --------------------------------------------------------------------------- *

procedure TID3v2.FSetGenre( NewGenre: string);
{
  //  Set genre name *
  FGenre = Trim(NewGenre);
}

//  --------------------------------------------------------------------------- *

procedure TID3v2.FSetComment( NewComment: string);
{
  //  Set comment *
  FComment = Trim(NewComment);
}

//  --------------------------------------------------------------------------- *

procedure TID3v2.FSetComposer( NewComposer: string);
{
  //  Set composer name *
  FComposer = Trim(NewComposer);
}

//  --------------------------------------------------------------------------- *

procedure TID3v2.FSetEncoder( NewEncoder: string);
{
  //  Set encoder name *
  FEncoder = Trim(NewEncoder);
}

//  --------------------------------------------------------------------------- *

procedure TID3v2.FSetCopyright( NewCopyright: string);
{
  //  Set copyright information *
  FCopyright = Trim(NewCopyright);
}

//  --------------------------------------------------------------------------- *

procedure TID3v2.FSetLanguage( NewLanguage: string);
{
  //  Set language *
  FLanguage = Trim(NewLanguage);
}

//  --------------------------------------------------------------------------- *

procedure TID3v2.FSetLink( NewLink: string);
{
  //  Set URL link *
  FLink = Trim(NewLink);
}

//  ********************** Public functions & procedures ********************** *

ructor TID3v2.Create;
{
  //  Create object *
  inherited;
  ResetData;
}
*/

//  --------------------------------------------------------------------------- *
void ID3v2_ResetData(PID3v2 self)
{
	//  Reset all variables *
	memset(self,0,sizeof(TID3v2));
}

//  --------------------------------------------------------------------------- *

u8 ID3v2_ReadFromFile(PID3v2 self ,FIL *hFile)
{
	TagInfo	  	Tag;
	u8		Result;
	//  Reset data and load header from file to variable *
	ID3v2_ResetData(self);
	Result = ReadHeader(hFile, &Tag);
	//  Process data if loaded and header valid *
	if( Result && (memcmp(Tag.ID , ID3V2_ID,3)==0))
	{
		self->FExists = TRUE;
		//  Fill properties with header data *
		self->FVersionID = Tag.Version;
		self->FSize = GetTagSize(&Tag);
/*
		//  Get information from frames if version supported *
		if( (self->FVersionID >=TAG_VERSION_2_2)&&(self->FVersionID<=TAG_VERSION_2_4) && (self->FSize > 0))
		{
			if( self->FVersionID > TAG_VERSION_2_2 )
				ReadFramesNew(hFile, &Tag);
		      else 
			  	ReadFramesOld(hFile, &Tag);
*/
//		      FTitle = GetContent(Tag.Frame[1], Tag.Frame[15]);
//		      FArtist = GetContent(Tag.Frame[2], Tag.Frame[14]);
//		      FAlbum = GetContent(Tag.Frame[3], Tag.Frame[16]);
//		      FTrack = ExtractTrack(Tag.Frame[4]);
//		      FTrackString = GetANSI(Tag.Frame[4]);
//		      FYear = ExtractYear(Tag.Frame[5], Tag.Frame[13]);
//		      FGenre = ExtractGenre(Tag.Frame[6]);
//		      FComment = ExtractText(Tag.Frame[7], TRUE);
//		      FComposer = GetANSI(Tag.Frame[8]);
//		      FEncoder = GetANSI(Tag.Frame[9]);
//		      FCopyright = GetANSI(Tag.Frame[10]);
//		      FLanguage = GetANSI(Tag.Frame[11]);
//		      FLink = ExtractText(Tag.Frame[12], FALSE);
//		}
	}
	return Result;
}

/*
//  --------------------------------------------------------------------------- *

function TID3v2.SaveToFile( FileName: string): u8;
var
  Tag: TagInfo;
{
  //  Check for existing tag *
  FillChar(Tag, SizeOf(Tag), 0);
  ReadHeader(FileName, Tag);
  //  Prepare tag data and save to file *
  Tag.Frame[1] = FTitle;
  Tag.Frame[2] = FArtist;
  Tag.Frame[3] = FAlbum;
  if FTrack > 0 then Tag.Frame[4] = IntToStr(FTrack);
  Tag.Frame[5] = FYear;
  Tag.Frame[6] = FGenre;
  if FComment <> "" then Tag.Frame[7] = "eng" + #0 + FComment;
  Tag.Frame[8] = FComposer;
  Tag.Frame[9] = FEncoder;
  Tag.Frame[10] = FCopyright;
  Tag.Frame[11] = FLanguage;
  if FLink <> "" then Tag.Frame[12] = #0 + FLink;
  Result = SaveTag(FileName, Tag);
}

//  --------------------------------------------------------------------------- *

function TID3v2.RemoveFromFile( FileName: string): u8;
{
  //  Remove tag from file *
  Result = RebuildFile(FileName, nil);
}

*/
