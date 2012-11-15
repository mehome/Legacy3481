#ifndef AVIPARSER_H
#define AVIPARSER_H
// All objects headers
#include <AVIRIFF.h>

//Amiga version MakeID(a,b,c,d) ((a)<<24|(b)<<16|(c)<<8|(d))
//PC version
#define MakeID(d,c,b,a) ((a)<<24|(b)<<16|(c)<<8|(d))

#define ID_RIFF	MakeID('R','I','F','F')
#define ID_LIST	MakeID('L','I','S','T')
//Layer 1 RIFF
#define ID_AVI 	MakeID('A','V','I',' ') //Identifies the file as AVI RIFF file.
#define ID_AVIX	MakeID('A','V','I','X') //New
//Layer 2 AVI & AVIX
#define ID_hdrl	MakeID('h','d','r','l') //Identifies a chunk containing subchunks that define the format of the data.
#define ID_movi	MakeID('m','o','v','i') //Identifies a chunk containing subchunks used for the audio and video data.
#define ID_idx1	MakeID('i','d','x','1') //Identifies a chunk containing the file index. 
//Layer 3 hdrl
#define ID_avih	MakeID('a','v','i','h') //Identifies a chunk containing general information about the file. This includes the number of streams and the width and height of the AVI sequence.
#define ID_strl	MakeID('s','t','r','l') //Identifies a chunk containing subchunks that describe the streams in a file. This chunk exists for each stream.
#define ID_odml	MakeID('o','d','m','l') //New
//Layer 4 strl
#define ID_strh	MakeID('s','t','r','h') //Identifies a chunk containing a stream header. This includes the type of stream.
#define ID_strf	MakeID('s','t','r','f') //Identifies a chunk describing the format of the data in the stream. For video streams, the information in this chunk is a BITMAPINFO structure. It includes palette information if appropriate.
#define ID_strd	MakeID('s','t','r','d') //Identifies a chunk containing information used by compressor and decompressors. For video compressors and decompressors, this includes the state formation.
#define ID_strn	MakeID('s','t','r','n') //Identifies a chunk containing a zero-terminated string specifying the name of the stream.
#define ID_indx	MakeID('i','n','d','x') //New
//Layer 4 odml new
#define ID_dmlh	MakeID('d','m','l','h')
//Stream types
#define ID_iavs	MakeID('i','a','v','s')
#define ID_vids	MakeID('v','i','d','s')
#define ID_auds	MakeID('a','u','d','s')
//DV defines
#define ID_CDVC	MakeID('C','D','V','C')
#define ID_dvsd	MakeID('d','v','s','d')
#define ID_DVSD	MakeID('D','V','S','D')
//Misc
#define ID_VIDC	MakeID('V','I','D','C')
#define ID_rec		MakeID('r','e','c',' ')
//Troubled upsidedown YUV's
#define ID_cvid	MakeID('c','v','i','d')
//Troubled DIVX issues
#define ID_div4 MakeID('d','i','v','4')
#define ID_DIVX MakeID('D','I','V','X')

struct AVIMainHeader {
	DWORD		dwMicroSecPerFrame;	// frame display rate (or 0L)
	DWORD		dwMaxBytesPerSec;	// max. transfer rate
	DWORD		dwPaddingGranularity;	// pad to multiples of this
                                           // size; normally 2K.
	DWORD		dwFlags;		// the ever-present flags
	DWORD		dwTotalFrames;		// # frames in file
	DWORD		dwInitialFrames;
	DWORD		dwStreams;
	DWORD		dwSuggestedBufferSize;

	DWORD		dwWidth;		//Not always defined
	DWORD		dwHeight;	//Not always defined

	DWORD		dwReserved[4];
	};

struct AVIStreamHeader {
	FOURCC		fccType;
	FOURCC		fccHandler;
	DWORD		dwFlags;	/* Contains AVITF_* flags */
	WORD		wPriority;
	WORD		wLanguage;
	DWORD		dwInitialFrames;
	DWORD		dwScale;	
	DWORD		dwRate;	/* dwRate / dwScale == samples/second */
	DWORD		dwStart;
	DWORD		dwLength; /* In units above... */
	DWORD		dwSuggestedBufferSize;
	DWORD		dwQuality;
	DWORD		dwSampleSize;
	struct {
      short int left;
      short int top;
      short int right;
      short int bottom;
		} rcFrame;
	};

/*
typedef struct tag_DVINFO {
	DWORD dwDVAAuxSrc;
	DWORD dwDVAAuxCtl;
	DWORD dwDVAAuxSrc1;
	DWORD dwDVAAuxCtl1;
	DWORD dwDVVAuxSrc;
	DWORD dwDVVAuxCtl;
	DWORD dwDVReserved[2];
} DVINFO, *PDVINFO;
*/

/*
In the streamlist we'll have a pointer to list indexes which point to index chunks, or we could
have a pointer to the index chunk themselves as defined in the type
*/

struct superindexentries {
	DWORDLONG qwOffset;    // 64 bit offset to sub index chunk
	DWORD    dwSize;       // 32 bit size of sub index chunk
	DWORD    dwDuration;   // time span of subindex chunk (in stream ticks)
	};

struct cindexlist {
	struct cindexlist *next;
	AVISTDINDEX *cindexptr;
	DWORD dwDuration; //to use for subtracting the frame difference
	};

struct AVIstream {
	struct AVIstream *next;
	struct cindexlist *cindexsptr;
	struct superindexentries *sindexptr;
	DWORD nEntriesInUse;
	//These next values are needed to preserve and stream out each set of cindex entries
	struct superindexentries *sindex;
	struct cindexlist *cprev;
	DWORD currententry;
	};

struct avistreamlist {
	struct avistreamlist *next;
	struct AVIStreamHeader avistreamnode;
	union {
		char *strf;
		BITMAPINFOHEADER *bmi;
		WAVEFORMATEX *wave;
		DVINFO *dvinfo;
		//ULONG numofentrys; don't remember why this is here
		};
	};

//Global Vars
extern struct memlist *pmem;
extern struct nodevars *nodeobject;
extern char string[256];
//End Globals

// Here are the function prototypes
// End Global function prototypes
struct avivarlist {
	struct avivarlist *next;
	struct avivarlist *prev;
	class avihandle *avivar;
	};

struct pcmlist {
	struct pcmlist *next;
	struct pcmlist *prev;
	BYTE *audiopcm;
	ULONG samplesize;
	};

class avihandle {
	//Some other AVI variables
	public:
	HANDLE hfile;
	DWORD totalframes;
	long *KFtable;
	long *KFnumber;
	DWORD totalKF;

	struct AVIstream *AVIstreamhead;
	WAVEFORMATEX waveinfo;
	//these vars are for type2 VCM
	BITMAPINFOHEADER *bmi;
	int bmisize;
	FOURCC vcmhandler;
/**/
	//These group of vars are for multiple instances in StoryBoard
	LPDIRECTSOUNDBUFFER lpdsb;
	LPDIRECTSOUNDNOTIFY lpDsNotify;
	int lastnotify;
	struct avivarlist *avivarnode;
	struct pcmlist *pcmlisthead,*pcmlisttail;
	HIC hic;
	BITMAPINFOHEADER *outbmi;
	char *invideobuf,*outvideobuf;
	BYTE *audiopcm;
	BYTE *audiopcm2;
/**/
	char *filesource;
	double framerate;
	long lastframe;			//figure out if we hit a new frame
	LONG nextaudiobeg;
	//LONG nextaudiobeghalf;
	DWORD lastoffset;			//needed to put next stream in right place
	DWORD audiobuffersize;	//pulled suggested buffer size from avi
	DWORD videobuffersize;	//may not need this
	DWORD totalaudiosize;
	UWORD x,y;
	BOOL error;					//used to check status of constructor
	BOOL surroundsound;
	BOOL type1;
	BOOL hasaudio,hasvideo;
	UBYTE scalex,scaley,upsidedown;
	char pad;
	
	avihandle(char *filesourceparm);
	~avihandle();
	ULONG getframeoffset(ULONG framenum,struct AVIstream *stream,DWORDLONG *qwframeoffset,DWORD *dwOffset,DWORD *dwSize);
	long *makeKFtable(struct AVIstream *video,struct AVIstream *audio);
	BOOL openavi();
	void closeavi();

	private:
	struct AVIstream *sprev;
	struct AVIMainHeader *avih;
	struct avistreamlist *strh;
	struct avistreamlist *strhhead;
	struct nodevars *avitempmem;
	__int64 movipos; //used for idx1 backward compatability < 2GB
	BOOL idx1;
	BYTE indextype;
	char pad2;

	BOOL handleRIFF(ULONG *outskipped,ULONG size);
	BOOL handleLISThdrl(ULONG *outskipped,ULONG size);
	BOOL handleLISTstrl(ULONG *outskipped,ULONG size);
	BOOL handleLISTodml(ULONG *outskipped,ULONG size);
	BOOL handleLISTmovi(ULONG *outskipped,ULONG size);
	BOOL handleLISTrec(ULONG *outskipped,ULONG size);
	long getidvalue(DWORD *dwChunkId);
	void idx1toindx(char *source,ULONG size);
	void avihandle::addstream(AVIstream **slist);
	BOOL getcindex();
	};


#endif //AVIPARSER_H

