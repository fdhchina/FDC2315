
#include <string.h>
#include "hal.h"
#include "ff.h"
#include "mpegaudio.h"


#define  MAX_MUSIC_GENRES  			148                       // Max. number of music genres *
#define  DEFAULT_GENRE 		 		255                       // Index for default genre *

  // Used with VersionID property *
#define  TAG_VERSION_1_0 			1                           // Index for ID3v1.0 tag *
#define  TAG_VERSION_1_1 			2                           // Index for ID3v1.1 tag *

/*
static char  MusicGenre[MAX_MUSIC_GENRES][25]=
{
  // Standard genres *
		"Blues",
		"Classic Rock",
		"Country",
		"Dance",
		"Disco";
		"Funk";
		"Grunge";
		"Hip-Hop";
		"Jazz";
		"Metal";
		"New Age";
		"Oldies";
		"Other";
		"Pop";
		"R&B";
		"Rap";
		"Reggae";
		"Rock";
		"Techno";
		"Industrial";
		"Alternative";
		"Ska";
		"Death Metal";
		"Pranks";
		"Soundtrack";
		"Euro-Techno";
		"Ambient";
		"Trip-Hop";
		"Vocal";
		"Jazz+Funk";
		"Fusion";
		"Trance";
		"Classical";
		"Instrumental";
		"Acid";
		"House";
		"Game";
		"Sound Clip";
		"Gospel";
		"Noise";
		"AlternRock";
		"Bass";
		"Soul";
		"Punk";
		"Space";
		"Meditative";
		"Instrumental Pop";
		"Instrumental Rock";
		"Ethnic";
		"Gothic";
		"Darkwave";
		"Techno-Industrial";
		"Electronic";
		"Pop-Folk";
		"Eurodance";
		"Dream";
		"Southern Rock";
		"Comedy";
		"Cult";
		"Gangsta";
		"Top 40";
  MusicGenre[61] = "Christian Rap";
  MusicGenre[62] = "Pop/Funk";
  MusicGenre[63] = "Jungle";
  MusicGenre[64] = "Native American";
  MusicGenre[65] = "Cabaret";
  MusicGenre[66] = "New Wave";
  MusicGenre[67] = "Psychadelic";
  MusicGenre[68] = "Rave";
  MusicGenre[69] = "Showtunes";
  MusicGenre[70] = "Trailer";
  MusicGenre[71] = "Lo-Fi";
  MusicGenre[72] = "Tribal";
  MusicGenre[73] = "Acid Punk";
  MusicGenre[74] = "Acid Jazz";
  MusicGenre[75] = "Polka";
  MusicGenre[76] = "Retro";
  MusicGenre[77] = "Musical";
  MusicGenre[78] = "Rock & Roll";
  MusicGenre[79] = "Hard Rock";
  // Extended genres *
  MusicGenre[80] = "Folk";
  MusicGenre[81] = "Folk-Rock";
  MusicGenre[82] = "National Folk";
  MusicGenre[83] = "Swing";
  MusicGenre[84] = "Fast Fusion";
  MusicGenre[85] = "Bebob";
  MusicGenre[86] = "Latin";
  MusicGenre[87] = "Revival";
  MusicGenre[88] = "Celtic";
  MusicGenre[89] = "Bluegrass";
  MusicGenre[90] = "Avantgarde";
  MusicGenre[91] = "Gothic Rock";
  MusicGenre[92] = "Progessive Rock";
  MusicGenre[93] = "Psychedelic Rock";
  MusicGenre[94] = "Symphonic Rock";
  MusicGenre[95] = "Slow Rock";
  MusicGenre[96] = "Big Band";
  MusicGenre[97] = "Chorus";
  MusicGenre[98] = "Easy Listening";
  MusicGenre[99] = "Acoustic";
  MusicGenre[100]= "Humour";
  MusicGenre[101]= "Speech";
  MusicGenre[102]= "Chanson";
  MusicGenre[103]= "Opera";
  MusicGenre[104]= "Chamber Music";
  MusicGenre[105]= "Sonata";
  MusicGenre[106]= "Symphony";
  MusicGenre[107]= "Booty Bass";
  MusicGenre[108]= "Primus";
  MusicGenre[109]= "Porn Groove";
  MusicGenre[110]= "Satire";
  MusicGenre[111]= "Slow Jam";
  MusicGenre[112]= "Club";
  MusicGenre[113]= "Tango";
  MusicGenre[114]= "Samba";
  MusicGenre[115]= "Folklore";
  MusicGenre[116]= "Ballad";
  MusicGenre[117]= "Power Ballad";
  MusicGenre[118]= "Rhythmic Soul";
  MusicGenre[119]= "Freestyle";
  MusicGenre[120]= "Duet";
  MusicGenre[121]= "Punk Rock";
  MusicGenre[122]= "Drum Solo";
  MusicGenre[123]= "A capella";
  MusicGenre[124]= "Euro-House";
  MusicGenre[125]= "Dance Hall";
  MusicGenre[126]= "Goa";
  MusicGenre[127]= "Drum & Bass";
  MusicGenre[128]= "Club-House";
  MusicGenre[129]= "Hardcore";
  MusicGenre[130]= "Terror";
  MusicGenre[131]= "Indie";
  MusicGenre[132]= "BritPop";
  MusicGenre[133]= "Negerpunk";
  MusicGenre[134]= "Polsk Punk";
  MusicGenre[135]= "Beat";
  MusicGenre[136]= "Christian Gangsta Rap";
  MusicGenre[137]= "Heavy Metal";
  MusicGenre[138]= "Black Metal";
  MusicGenre[139]= "Crossover";
  MusicGenre[140]= "Contemporary Christian";
  MusicGenre[141]= "Christian Rock";
  MusicGenre[142]= "Merengue";
  MusicGenre[143]= "Salsa";
  MusicGenre[144]= "Trash Metal";
  MusicGenre[145]= "Anime";
  MusicGenre[146]= "JPop";
  MusicGenre[147]= "Synthpop";
};        // Genre names *
*/


  // Real structure of ID3v1 tag *
typedef struct  _TagRecord {
	char			Header[3];                // Tag header - must be "TAG" *
	char			Title[30];                                // Title data *
	char			Artist[30];                              // Artist data *
	char			Album[30];                                // Album data *
	char			Year[4];                                   // Year data *
	char			Comment[30];                            // Comment data *
	u8			Genre;                                                 // Genre data *
}TagRecord,*PTagRecord;


// ********************* Auxiliary functions & procedures ******************** *
u8 ReadTag(FIL *hFile, PTagRecord TagData)
{
	UINT br;
	// Set read-access and open file *
	if(hFile)
	{
		// Read tag *
		if(f_lseek(hFile, hFile->fptr+hFile->fsize - 128)!=FR_OK) return FALSE;
		if(f_read(hFile, TagData, 128, &br)!=FR_OK) return FALSE;
		return TRUE;
	}
	return FALSE;
}

/*
// --------------------------------------------------------------------------- *
u8 RemoveTag(const FileName: string): u8;
var
  hFile: file;
{
  try
    Result = TRUE;
    // Allow write-access and open file *
    FileSetAttr(FileName, 0);
    AssignFile(hFile, FileName);
    FileMode = 2;
    Reset(hFile, 1);
    // Delete tag *
    Seek(hFile, FileSize(hFile) - 128);
    Truncate(hFile);
    CloseFile(hFile);
  except
    // Error *
    Result = false;
  }
}


// --------------------------------------------------------------------------- *

function SaveTag(const FileName: string; TagData: TagRecord): u8;
var
  hFile: file;
{
  try
    Result = TRUE;
    // Allow write-access and open file *
    FileSetAttr(FileName, 0);
    AssignFile(hFile, FileName);
    FileMode = 2;
    Reset(hFile, 1);
    // Write tag *
    Seek(hFile, FileSize(hFile));
    BlockWrite(hFile, TagData, SizeOf(TagData));
    CloseFile(hFile);
  except
    // Error *
    Result = false;
  }
}
*/
// --------------------------------------------------------------------------- *

u8 GetTagVersion(const PTagRecord TagData)
{
	// Terms for ID3v1.1 *
	if( 	((TagData->Comment[28] ==0) && (TagData->Comment[29] != 0)) ||
		((TagData->Comment[28] == 32) && (TagData->Comment[29] != 32)) 	)
	    return TAG_VERSION_1_1;

	return TAG_VERSION_1_0;
}

/*
// ********************** Private functions & procedures ********************* *
procedure TID3v1.FSetTitle(const NewTitle: String30);
{
  FTitle = TrimRight(NewTitle);
}

// --------------------------------------------------------------------------- *

procedure TID3v1.FSetArtist(const NewArtist: String30);
{
  FArtist = TrimRight(NewArtist);
}

// --------------------------------------------------------------------------- *

procedure TID3v1.FSetAlbum(const NewAlbum: String30);
{
  FAlbum = TrimRight(NewAlbum);
}

// --------------------------------------------------------------------------- *

procedure TID3v1.FSetYear(const NewYear: String04);
{
  FYear = TrimRight(NewYear);
}

// --------------------------------------------------------------------------- *

procedure TID3v1.FSetComment(const NewComment: String30);
{
  FComment = TrimRight(NewComment);
}

// --------------------------------------------------------------------------- *

procedure TID3v1.FSetTrack(const NewTrack: Byte);
{
  FTrack = NewTrack;
}

// --------------------------------------------------------------------------- *

procedure TID3v1.FSetGenreID(const NewGenreID: Byte);
{
  FGenreID = NewGenreID;
}

// --------------------------------------------------------------------------- *

char * FGetGenre(PID3v1 self)
{
	// Return an empty string if the current GenreID is not valid *
	if ((self->FGenreID>=0) &&( self->FGenreID< MAX_MUSIC_GENRES) return  MusicGenre[FGenreID];
	return 0;
}

*/
// --------------------------------------------------------------------------- *
void ID3v1_ResetData(PID3v1 self)
{

	self->FExists = FALSE;
	self->FVersionID = TAG_VERSION_1_0;
	self->FTitle[0] = 0;
	self->FArtist[0] = 0;
	self->FAlbum[0] = 0;
	self->FYear[0] = 0;
	self->FComment[0] = 0;
	self->FTrack = 0;
	self->FGenreID = DEFAULT_GENRE;
}


// --------------------------------------------------------------------------- *

u8	ID3v1_ReadFromFile(PID3v1 self, FIL *hFile)
{
	TagRecord	TagData;
	u8	Result;

	// Reset and load tag data from file to variable *
	ID3v1_ResetData(self);
	Result = ReadTag(hFile, &TagData);
	// Process data if loaded and tag header OK *
	if( Result && (memcmp(TagData.Header, "TAG",3)==0))
	{
		self->FExists = TRUE;
		self->FVersionID = GetTagVersion(&TagData);
/*		
		// Fill properties with tag data *
		memcpy(self->FTitle,TagData.Title,30);
		memcpy(self->FArtist,TagData.Artist,30);
		memcpy(self->FAlbum , TagData.Album,30);
		memcpy(self->FYear, TagData.Year,4);
*/
		if(self->FVersionID == TAG_VERSION_1_0)
			memcpy(self->FComment,TagData.Comment,30);
		else
		{
			memcpy(self->FComment ,TagData.Comment,  28);
			self->FTrack = TagData.Comment[29];
		}

		self->FGenreID = TagData.Genre;
	}
	return Result;
}

/*
// --------------------------------------------------------------------------- *

function TID3v1.RemoveFromFile(const FileName: string): u8;
var
  TagData: TagRecord;
{
  // Find tag *
  Result = ReadTag(FileName, TagData);
  // Delete tag if loaded and tag header OK *
  if (Result) and (TagData.Header = "TAG") then Result = RemoveTag(FileName);
}

// --------------------------------------------------------------------------- *

function TID3v1.SaveToFile(const FileName: string): u8;
var
  TagData: TagRecord;
{
  // Prepare tag record *
  FillChar(TagData, SizeOf(TagData), 0);
  TagData.Header = "TAG";
  Move(FTitle[1], TagData.Title, Length(FTitle));
  Move(FArtist[1], TagData.Artist, Length(FArtist));
  Move(FAlbum[1], TagData.Album, Length(FAlbum));
  Move(FYear[1], TagData.Year, Length(FYear));
  Move(FComment[1], TagData.Comment, Length(FComment));
  if FTrack > 0 then
  {
    TagData.Comment[29] = #0;
    TagData.Comment[30] = Chr(FTrack);
  }
  TagData.Genre = FGenreID;
  // Delete old tag and write new tag *
  Result = (RemoveFromFile(FileName)) and (SaveTag(FileName, TagData));
}

*/
