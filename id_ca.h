// ID_CA.H
//===========================================================================

#define NUM_EPISODES			1
#define MAPS_PER_EPISODE	25
#define MAPS_WITH_STATS		20

#define NUMMAPS				NUM_EPISODES*MAPS_PER_EPISODE
#define MAPPLANES	2

void UNCACHEGRCHUNK(unsigned short chunk);

#define THREEBYTEGRSTARTS

#ifdef THREEBYTEGRSTARTS
#define FILEPOSSIZE	3
#else
#define FILEPOSSIZE	4
#endif

//===========================================================================

typedef	struct
{
	long		planestart[3];
	unsigned short	planelength[3];
	unsigned short	width,height;
	char		name[16];
} maptype;

typedef struct
{
  unsigned short bit0,bit1;	// 0-255 is a character, > is a pointer to a node
} huffnode;

typedef struct
{
	unsigned short	RLEWtag;
	long		headeroffsets[100];
//	byte		tileinfo[];
} mapfiletype;

//===========================================================================

extern	char		audioname[13];

extern	byte 		*tinf;
extern	short			mapon;

extern	unsigned short	*mapsegs[MAPPLANES];
extern	maptype		*mapheaderseg[NUMMAPS];
extern	byte		*audiosegs[NUMSNDCHUNKS];
extern	void		*grsegs[NUMCHUNKS];

extern	byte		grneeded[NUMCHUNKS];
extern	byte		ca_levelbit,ca_levelnum;

extern	char		*titleptr[8];

extern	short			profilehandle,debughandle;

extern	char		extension[5],
			gheadname[10],
			gfilename[10],
			gdictname[10],
			mheadname[10],
			mfilename[10],
			aheadname[10],
			afilename[10];

extern long		*grstarts;	// array of offsets in egagraph, -1 for sparse
extern long		*audiostarts;	// array of offsets in audio / audiot
//
// hooks for custom cache dialogs
//
extern	void	(*drawcachebox)		(char *title, unsigned short numcache);
extern	void	(*updatecachebox)	(void);
extern	void	(*finishcachebox)	(void);

extern short			grhandle;		// handle to EGAGRAPH
extern short			maphandle;		// handle to MAPTEMP / GAMEMAPS
extern short			audiohandle;	// handle to AUDIOT / AUDIO
extern long		chunkcomplen,chunkexplen;

#ifdef GRHEADERLINKED
extern huffnode	*grhuffman;
#else
extern huffnode	grhuffman[255];
#endif

//===========================================================================

// just for the score box reshifting

void CAL_ShiftSprite (unsigned short segment,unsigned short source,unsigned short dest,
	unsigned short width, unsigned short height, unsigned short pixshift);

//===========================================================================

void CA_OpenDebug (void);
void CA_CloseDebug (void);
boolean CA_FarRead (short handle, byte *dest, long length);
boolean CA_FarWrite (short handle, byte *source, long length);
boolean CA_ReadFile (char *filename, void** ptr);
boolean CA_LoadFile (char *filename, void** ptr);
boolean CA_WriteFile (char *filename, void *ptr, long length);

long CA_RLEWCompress (unsigned short *source, long length, unsigned short *dest,
  unsigned short rlewtag);

void CA_RLEWexpand (unsigned short *source, unsigned short *dest,long length,
  unsigned short rlewtag);

void CA_Startup (void);
void CA_Shutdown (void);

void CA_SetGrPurge (void);
void CA_CacheAudioChunk (short chunk);
void CA_LoadAllSounds (void);

void CA_UpLevel (void);
void CA_DownLevel (void);

void CA_SetAllPurge (void);

void CA_ClearMarks (void);
void CA_ClearAllMarks (void);

#define CA_MarkGrChunk(chunk)	grneeded[chunk]|=ca_levelbit

void CA_CacheGrChunk (short chunk);
void CA_CacheMap (short mapnum);

void CA_CacheMarks (void);

void CAL_SetupAudioFile (void);
void CAL_SetupGrFile (void);
void CAL_SetupMapFile (void);
void CAL_HuffExpand (byte *source, byte *dest,
  long length,huffnode *hufftable, boolean screenhack);

void CloseGrFile(void);
void OpenGrFile(void);