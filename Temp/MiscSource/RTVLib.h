#ifndef __RTV__
#define __RTV__

// Useful types
#ifndef INT64
typedef hyper				INT64;
typedef unsigned hyper		UINT64;
#endif

// Predeclarations to avoid extra header files
class RTMFFileHeader;
class RTMFStreamHeader;
class RTVSubFormat;

// This is the number of frames per second to use if you do not know better
const float DefaultFieldsPerSecond=29.97f*2.0f;

// This is the default drive chunk size until it is contained in the header
const long DefaultChunkSize=512;

// This class can be used to read an RTV file
class RTVFile
{	public:		// The file pointer to read from
				HANDLE FileHandle;

				// The current file position
				__int64 FilePosition;
				
	private:	// The size of an on disk chunk
				DWORD ChunkSize;

				// The file header to read
				RTMFFileHeader *FH;

				// Store all the stram headers
				RTMFStreamHeader *StreamHeaders;

				// The streams numbers
				long NoAudioStreams;
				RTVSubFormat **AudioStreams;

				long NoVideoStreams;
				RTVSubFormat **VideoStreams;

				// The frames to play per second
				float FramesPerSecond;

	public:		// Get the number of frames a second that need to be played
				float GetFramesPerSecond(void) { return FramesPerSecond; }

				// Get the ammount of memory needed by a particular stream
				long GetVideoStreamMemorySize(long StreamNumber=0);
				long GetAudioStreamMemorySize(long StreamNumber=0);

				// Get a frame number for a particular video stream from memory
				// Returns success or failure
				void *GetRAWVideoStream(DWORD FieldNo,DWORD StreamNumber=0);
				void *GetRAWAudioStream(DWORD FieldNo,DWORD StreamNumber=0);

				bool PutRAWVideoStream(DWORD FieldNo,void *Mem,DWORD StreamNumber=0);
				bool PutRAWAudioStream(DWORD FieldNo,void *Mem,DWORD StreamNumber=0);
				bool PutRAWVideoStreamFast(DWORD FieldNo,void *Mem,DWORD StreamNumber=0);

				void *GetRAWVideoStreamMem(DWORD FieldNo,DWORD StreamNumber=0);
				void *GetRAWAudioStreamMem(DWORD FieldNo,DWORD StreamNumber=0);
				bool PutRAWVideoStream(DWORD FieldNo,DWORD StreamNumber=0);
				bool PutRAWAudioStream(DWORD FieldNo,DWORD StreamNumber=0);

				bool GetRAWVideoStream(DWORD FieldNo,void *Mem,DWORD StreamNumber=0);
				bool GetRAWAudioStream(DWORD FieldNo,void *Mem,DWORD StreamNumber=0);

				// Get the number of true streams in the file
				long GetNumberOfVideoStreams(void) { return NoVideoStreams>>1; }
				long GetNumberOfAudioStreams(void) { return NoAudioStreams>>1; }

				// Covert video data to BGRA format
				BOOL GetRAWVideoDataBGRA(void *MemBGRA,DWORD FieldNo,DWORD StreamNumber=0);
				BOOL PutRAWVideoDataBGRA(void *MemBGRA,DWORD FieldNo,DWORD StreamNumber=0);

				void GetRAWVideoDataResolution(DWORD &xres,DWORD &yres,DWORD StreamNumber=0);

				// get the number of frames in this RTV file
				UINT64 GetNoFrames(void);

				// Constructor
				RTVFile(char *Filename);

				// Destructor
				~RTVFile(void);
};

//**************************************************************************************************************************************
// This is how you build a RTV file header
void BuildRTVFile(	char *Filename,long VideoStreams=2,long AudioStreams=0,
					long xres=720,long yres=240,float FrameRate=29.97f);
																// Build an RTV file of the correct size, 
																//*** You should keep the default settings : ***
																// VideoStreams = 2
																// AudioStreams = 0	 <- We do not want to support audio in RTV files right now
																// xres=720
																// yres=240(NTSC) or 288(PAL)
																// FrameRate=29.97(NTSC) or 25.00(PAL)

// This gives you a quick answer whether a particular file is an RTV file or not
BOOL IsRTV(char *Filename);										// Verify that the file is actually an RTV file

// Get the resolution of an RTV file !
bool	GetRTVResolution(char *Name,int &ResX,int &ResY);		// This tells you the resolution of an RTV file

// Load RTV Files using cached name access
bool	ReadRTVFile(char *Name,DWORD Frame,void *Dest);			// This reads a raw frame, YUV422 data, U0Y0V0Y1 U0Y0V0Y1 U0Y0V0Y1
bool	ReadRTVFileBGRA(char *Name,DWORD Frame,void *Dest);		// This reads a frame in BGRA data	BGRA BGRA BGRA			** MMX'ed **
bool	ReadRTVFileYUV4444(char *Name,DWORD Frame,void *Dest);	// This reads a frame in YUVA data	YUVA YUVA YUVA
DWORD	ReadRTVNoFrames(char *Name);							// This gets 

bool	WriteRTVFile(char *Name,void *Dest);					// Dump out YUV422 data in a stream ... Below is faster
bool	WriteRTVFileFastest(char *Name,void *Dest);				// Dump out YUV422 data in a stream ...
bool	WriteRTVFileBGRA(char *Name,void *Dest);				// Dump out BGRA data										** MMX'ed **
bool	WriteRTVFileYUV4444(char *Name,void *Dest);				// Dump oiut YUVA4444 data

bool	CloseRTVFile(char *Name);								// Force a file to be closed
#endif