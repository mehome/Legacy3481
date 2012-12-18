#ifndef __FileIOWrapperH__
#define __FileIOWrapperH__

#define BYTESPERSECTOR 512

inline void *mallocAligned(SIZE_T size) {
	return VirtualAlloc(NULL,size,MEM_COMMIT,PAGE_READWRITE);
	}

inline void freeAligned(void *memblock) {
	VirtualFree(memblock,0,MEM_RELEASE);
	}

//Dos lowlever file IO API
HANDLE OpenReadSeq(char *filename);
__int64 mySeek64 (HANDLE hf,__int64 distance,DWORD MoveMethod);
__int64 myTell64 (HANDLE hf);
DWORD myTell (HANDLE hf);
DWORD mySeek (HANDLE hf,DWORD distance,DWORD MoveMethod);
int myRead(HANDLE hf,void *buf,DWORD count);
int myClose(HANDLE hf);
DWORD myGetFileSize(HANDLE hf,LPDWORD HighPart);
void DebugOutput(char *format, ... );

int myRead(HANDLE hf,char **OutLowBuf,unsigned *SizeLowBuf,char **OutHiBuf,unsigned *SizeHiBuf,unsigned count);
void TransferToSingleBuffer(char *DestBuf,char *OutLowBuf,unsigned SizeLowBuf,char *OutHiBuf,unsigned SizeHiBuf);

class FileIOWrapper {
	public:
		FileIOWrapper(unsigned BufferSize=512*256);
		virtual ~FileIOWrapper();

		FileIOWrapper *OpenReadSeq(char *filename);
		__int64 mySeek64 (__int64 distance,DWORD MoveMethod);
		__int64 myTell64 ();
		DWORD myTell ();
		DWORD mySeek (DWORD distance,DWORD MoveMethod);
		int myRead(void *buf,DWORD count);
		int myClose();

		int myRead(char **OutLowBuf,unsigned *SizeLowBuf,char **OutHiBuf,unsigned *SizeHiBuf,unsigned count);
		HANDLE Get_hf() {return m_hf;}
	private:
		__int64 m_FilePointer;
		__int64 m_LoLastReadFP;		//Records the last disk read of LoBuffer (which could be greater)
		__int64 m_HiLastReadFP;		//If UseHighFirst is true.. and this is for HiBuffer Respectively
		__int64 m_FileSize;
		unsigned m_BufferSize;		//The Read will make sure not to exceed the buffersize

		char *m_LoFileBuffer;
		char *m_HiFileBuffer;
		HANDLE m_hf;
		OVERLAPPED m_FileOverLapped;	//Used for asynchronous Control
		bool m_LoBufferReading,m_HiBufferReading;		//Check for when read has completed
	};

#endif __FileIOWrapperH__
