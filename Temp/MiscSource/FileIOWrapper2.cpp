#include "../stdafx.h"

//#define __USE_NO_BUFFERING__

inline __int64 AlignBPSUp64(__int64 number) {
	//add number up and mask out bits
	return ((number+BYTESPERSECTOR-1)&(~(BYTESPERSECTOR-1)));
	}

inline __int64 AlignBPSDown64(__int64 number) {
	//add number up and mask out bits
	return (number&(~(BYTESPERSECTOR-1)));
	}

inline DWORD AlignBPS(DWORD number) {
	//add number up and mask out bits
	return ((number+BYTESPERSECTOR-1)&(~(BYTESPERSECTOR-1)));
	}


#ifdef __USE_NO_BUFFERING__

HANDLE OpenReadSeq(char *filename) {
	HANDLE Ret;
	FileIOWrapper *hf=new FileIOWrapper();
	if (!(hf->OpenReadSeq(filename))) Ret=(void *)-1;
	Ret=(void *)hf;
	return Ret;
	}


__int64 mySeek64 (HANDLE hf,__int64 distance,DWORD MoveMethod) {
	__int64 Ret = -1;

	if (hf!=(void *)-1) {
		FileIOWrapper *fio=(FileIOWrapper *)hf;
		Ret=fio->mySeek64(distance,MoveMethod);
		}
	return Ret;
	}


__int64 myTell64 (HANDLE hf) {
	__int64 Ret = -1;

	if (hf!=(void *)-1) {
		FileIOWrapper *fio=(FileIOWrapper *)hf;
		Ret=fio->myTell64();
		}
	return Ret;
	}


DWORD myTell (HANDLE hf) {
	DWORD Ret = -1;

	if (hf!=(void *)-1) {
		FileIOWrapper *fio=(FileIOWrapper *)hf;
		Ret=fio->myTell();
		}
	return Ret;
	}


DWORD mySeek (HANDLE hf,DWORD distance,DWORD MoveMethod) {
	DWORD Ret = -1;

	if (hf!=(void *)-1) {
		FileIOWrapper *fio=(FileIOWrapper *)hf;
		Ret=fio->mySeek(distance,MoveMethod);
		}
	return Ret;
	}


int myRead(HANDLE hf,void *buf,DWORD count) {
	int Ret = -1;

	if (hf!=(void *)-1) {
		FileIOWrapper *fio=(FileIOWrapper *)hf;
		Ret=fio->myRead(buf,count);
		}
	return Ret;
	}


int myClose(HANDLE hf) {
	int Ret = -1;

	if (hf!=(void *)-1) {
		FileIOWrapper *fio=(FileIOWrapper *)hf;
		Ret=fio->myClose();
		delete fio;
		}
	return Ret;
	}

DWORD myGetFileSize(HANDLE hf,LPDWORD HighPart) {
	DWORD Ret = INVALID_FILE_SIZE;
	if (hf!=(void *)-1) {
		FileIOWrapper *fio=(FileIOWrapper *)hf;
		Ret = GetFileSize(fio->Get_hf(),HighPart);
		}
	return Ret;
	}

#endif __USE_NO_BUFFERING__


void TransferToSingleBuffer(char *DestBuf,char *OutLowBuf,unsigned SizeLowBuf,char *OutHiBuf,unsigned SizeHiBuf) {
	if ((!DestBuf)||(!OutLowBuf)||(!OutHiBuf)) return;
	memcpy(DestBuf,OutLowBuf,SizeLowBuf);
	DestBuf+=SizeLowBuf;
	memcpy(DestBuf,OutHiBuf,SizeHiBuf);
	}


int myRead(HANDLE hf,char **OutLowBuf,unsigned *SizeLowBuf,char **OutHiBuf,unsigned *SizeHiBuf,unsigned count) {
	int Ret = -1;

	if (hf!=(void *)-1) {
		FileIOWrapper *fio=(FileIOWrapper *)hf;
		Ret=fio->myRead(OutLowBuf,SizeLowBuf,OutHiBuf,SizeHiBuf,count);
		}
	return Ret;
	}


FileIOWrapper::FileIOWrapper(unsigned BufferSize) {
	BufferSize=1024*1024;
	m_LoFileBuffer=(char *)mallocAligned(BufferSize);
	m_HiFileBuffer=(char *)mallocAligned(BufferSize);
	m_BufferSize=BufferSize;
	m_LoLastReadFP=-1;
	m_HiLastReadFP=-1;
	m_FilePointer=0;
	m_hf=(void *)-1;
	m_FileSize=0;
	m_LoBufferReading=m_HiBufferReading=false;
	}

FileIOWrapper::~FileIOWrapper() {
	if (m_LoFileBuffer)
		freeAligned(m_LoFileBuffer);
	if (m_HiFileBuffer)
		freeAligned(m_HiFileBuffer);
	}


FileIOWrapper *FileIOWrapper::OpenReadSeq(char *filename) {
	HANDLE hf;

	m_LoLastReadFP=-1;
	m_HiLastReadFP=-1;
	m_FilePointer=0;
	m_hf=(void *)-1;
	m_FileSize=0;
	m_LoBufferReading=m_HiBufferReading=false;

	FileIOWrapper *Ret=NULL;

	do {
		hf=CreateFile(filename,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			//FILE_ATTRIBUTE_NORMAL,
			FILE_FLAG_NO_BUFFERING|FILE_FLAG_OVERLAPPED,
			NULL);
		if (hf==INVALID_HANDLE_VALUE) break;

		//Get the File Size
		LARGE_INTEGER li;
		DWORD LowPart,HighPart;
		LowPart=GetFileSize(hf,&HighPart);
		if (LowPart==INVALID_FILE_SIZE) break;
		li.LowPart=(unsigned)LowPart;
		li.HighPart=(unsigned)HighPart;

		//success
		Ret=this;
		m_hf=hf;
		m_FileSize=li.QuadPart;
		} while (false);

	return (Ret);
	}

__int64 FileIOWrapper::mySeek64 (__int64 distance,DWORD MoveMethod) {
	switch (MoveMethod) {
		case FILE_BEGIN:
			m_FilePointer=distance;
			break;
		case FILE_CURRENT:
			m_FilePointer+=distance;
			break;
		case FILE_END:
			m_FilePointer=m_FileSize+distance;
			break;
		default:
			throw "FileIOWrapper::mySeek64 unknown MoveMethod";
		}
	return m_FilePointer;
	}

__int64 FileIOWrapper::myTell64 () {
	return m_FilePointer;
	}

DWORD FileIOWrapper::myTell () {
	return (DWORD)m_FilePointer;
	}

DWORD FileIOWrapper::mySeek (DWORD distance,DWORD MoveMethod) {
	return (DWORD)mySeek64((__int64)distance,MoveMethod);
	}


int FileIOWrapper::myRead(void *buf,DWORD count) {
	char *OutLowBuf;
	unsigned SizeLowBuf;
	char *OutHiBuf;
	unsigned SizeHiBuf;
	int Ret;
	Ret=myRead(&OutLowBuf,&SizeLowBuf,&OutHiBuf,&SizeHiBuf,count);
	if (Ret!=-1)
		TransferToSingleBuffer((char *)buf,OutLowBuf,SizeLowBuf,OutHiBuf,SizeHiBuf);
	return Ret;
	}


int FileIOWrapper::myRead(char **OutLowBuf,unsigned *SizeLowBuf,char **OutHiBuf,unsigned *SizeHiBuf,unsigned count) {
	int Ret=-1;

	do {
		HANDLE hf=m_hf;
		if (count>m_BufferSize)
			throw "TODO augment the filebuffer count>buffersize";

		unsigned BufferSize=m_BufferSize;
		__int64 FilePointer_Start,FilePointer_End;
		FilePointer_Start=FilePointer_End=m_FilePointer;
		FilePointer_End+=count;

		__int64 LoLastReadFP_Start,LoLastReadFP_End;
		LoLastReadFP_Start=LoLastReadFP_End=m_LoLastReadFP;
		LoLastReadFP_End+=BufferSize;

		__int64 HiLastReadFP_Start,HiLastReadFP_End;
		HiLastReadFP_Start=HiLastReadFP_End=m_HiLastReadFP;
		HiLastReadFP_End+=BufferSize;

		unsigned StartMarker,EndMarker;
		bool FoundStartMarker=false;
		bool FoundEndMarker=false;
		bool StartMarkerInHiBuf;
		bool EndMarkerInHiBuf;

		//Try to find a StartMarker
		if (LoLastReadFP_Start!=-1) {
			if (FilePointer_Start>=LoLastReadFP_Start) {
				if (FilePointer_Start<=LoLastReadFP_End) {
					FoundStartMarker=true;
					StartMarkerInHiBuf=false;
					StartMarker=(unsigned)(FilePointer_Start-LoLastReadFP_Start);
					}
				}
			}
		//Maybe the StartMarker is in the HiBuffer
		if ((!FoundStartMarker)&&(HiLastReadFP_Start!=-1)) {
			if (FilePointer_Start>=HiLastReadFP_Start) {
				if (FilePointer_Start<=HiLastReadFP_End) {
					FoundStartMarker=true;
					StartMarkerInHiBuf=true;
					StartMarker=(unsigned)(FilePointer_Start-HiLastReadFP_Start);
					}
				}
			}

		//Try to find the EndMarker
		if (LoLastReadFP_Start!=-1) {
			if (FilePointer_End>=LoLastReadFP_Start) {
				if (FilePointer_End<=LoLastReadFP_End) {
					FoundEndMarker=true;
					EndMarkerInHiBuf=false;
					EndMarker=(unsigned)(FilePointer_End-LoLastReadFP_Start);
					}
				}
			}
		//Maybe the EndMarker is in the HiBuffer
		if ((!FoundEndMarker)&&(HiLastReadFP_Start!=-1))  {
			if (FilePointer_End>=HiLastReadFP_Start) {
				if (FilePointer_End<=HiLastReadFP_End) {
					FoundEndMarker=true;
					EndMarkerInHiBuf=true;
					EndMarker=(unsigned)(FilePointer_End-HiLastReadFP_Start);
					}
				}
			}


		unsigned LoCount=0,LoSourceOffset=0,LoDestOffset=0;

		//See what our LoBuffer can do
		if ((FoundStartMarker)&&(StartMarkerInHiBuf==false)) {
			LoSourceOffset=StartMarker;
			}

		if ((FoundEndMarker)&&(EndMarkerInHiBuf==false)) {
			//This is either the entire range or the end half
			LoCount=EndMarker-LoSourceOffset;
			//If this is the end half then we'll have a destoffset
			if ((!FoundStartMarker)||
				((FoundStartMarker)&&(StartMarkerInHiBuf==true))) {
				LoDestOffset=count-LoCount;
				}
			}
		else { //Checking for a Beginning half
			if ((FoundStartMarker)&&(StartMarkerInHiBuf==false)) {
				LoCount=BufferSize-LoSourceOffset;
				}
			}

		unsigned HiCount=0,HiSourceOffset=0,HiDestOffset=0;

		//See what our HiBuffer can do
		if ((FoundStartMarker)&&(StartMarkerInHiBuf==true)) {
			HiSourceOffset=StartMarker;
			}

		if ((FoundEndMarker)&&(EndMarkerInHiBuf==true)) {
			//This is either the entire range or the end half
			HiCount=EndMarker-HiSourceOffset;
			//If this is the end half then we'll have a destoffset
			if ((!FoundStartMarker)||
				((FoundStartMarker)&&(StartMarkerInHiBuf==false))) {
				HiDestOffset=count-HiCount;
				}
			}
		else { //Checking for a Beginning half
			if ((FoundStartMarker)&&(StartMarkerInHiBuf==true)) {
				HiCount=BufferSize-HiSourceOffset;
				}
			}

		//Now to Figure out if we need to Stream to the Buffers....
		{
			bool SomethingNeedsUpdated;
			bool GetNextBlock=false;
			bool GetPrevBlock=false;
			bool HiBufferNeedsUpdated=true;
			if ((HiCount+LoCount) < count) {

				SomethingNeedsUpdated=true;
				if (LoCount&&HiCount)
					throw "FileIOWrapper::myRead Something happened real wrong";
				if (LoCount==0) {
					//Either HiBuffer Has first half Last Half or nothing
					if (HiCount) {
						if ((FoundStartMarker)&&(StartMarkerInHiBuf==true)) {
							//HiBuffer has beginning half
							GetNextBlock=true;
							HiBufferNeedsUpdated=false;
							//Now to fill in the end half
							LoCount=count-HiCount;
							LoDestOffset=HiCount;
							}
						else if ((FoundEndMarker)&&(EndMarkerInHiBuf==true)) {
							//HiBuffer has end half
							GetPrevBlock=true;
							HiBufferNeedsUpdated=false;
							//Now to fill in the beginning half
							LoCount=count-HiCount;
							LoSourceOffset=BufferSize-LoCount;
							}
						}
					}
				if (HiCount==0) {
					//Either LoBuffer Has first half Last Half or nothing
					if (LoCount) {
						if ((FoundStartMarker)&&(StartMarkerInHiBuf==false)) {
							//LoBuffer has beginning half
							GetNextBlock=true;
							HiBufferNeedsUpdated=true;
							//Now to fill in the end half
							HiCount=count-LoCount;
							HiDestOffset=LoCount;
							}
						else if ((FoundEndMarker)&&(StartMarkerInHiBuf==false)) {
							//LoBuffer has end half
							GetPrevBlock=true;
							HiBufferNeedsUpdated=true;
							//Now to fill in the beginning half
							HiCount=count-LoCount;
							HiSourceOffset=BufferSize-HiCount;
							}
						}
					else { //Neither Buffer has anything
						HiBufferNeedsUpdated=false;
						//we'll have to define a starting point
						LoSourceOffset=(unsigned)(FilePointer_Start-(AlignBPSDown64(FilePointer_Start)));
						//since offset is less that 512 bytes the count should always fit in this case
						LoCount=count;
						}
					}
				}
			else { //Prefetch Sequential
				SomethingNeedsUpdated=false;
				GetNextBlock=true;
				// only do this if both markers are in one buffer!
				if ((FoundStartMarker)&&(FoundEndMarker)) {
					if ((StartMarkerInHiBuf==true)&&(EndMarkerInHiBuf==true)) {
						if (LoLastReadFP_Start<HiLastReadFP_Start) {
							HiBufferNeedsUpdated=false;
							SomethingNeedsUpdated=true;
							}
						}
					if ((StartMarkerInHiBuf==false)&&(EndMarkerInHiBuf==false)) {
						if (HiLastReadFP_Start<LoLastReadFP_Start) {
							HiBufferNeedsUpdated=true;
							SomethingNeedsUpdated=true;
							}
						}
					}
				}

			if (SomethingNeedsUpdated) {
				//Seek to the closest position of the filepointer
				__int64 LastReadFP=AlignBPSDown64(FilePointer_Start);

				if ((GetNextBlock)||(GetPrevBlock)) {
					LastReadFP=HiBufferNeedsUpdated?m_LoLastReadFP:m_HiLastReadFP;
					if (GetNextBlock)
						LastReadFP+=BufferSize;
					if (GetPrevBlock)
						LastReadFP-=BufferSize;
					}
				else {
					//Make sure our buffer doesn't overlap with the other
					//get the other buffer's range
					__int64 OtherBufferStart=HiBufferNeedsUpdated?m_LoLastReadFP:m_HiLastReadFP;
					__int64 OtherBufferEnd=OtherBufferStart+BufferSize;
					//Are we next or prev to this buffer
					if (LastReadFP<OtherBufferStart) {
						//we are previous to it
						if ((OtherBufferStart-LastReadFP)<BufferSize) {
							//if it does will make the other invalid
							if (HiBufferNeedsUpdated) m_LoLastReadFP=-1;
							else m_HiLastReadFP=-1;
							}
						}
					}

				//Now to prep the actual file pointer
				LARGE_INTEGER li;
				li.QuadPart = LastReadFP;
				OVERLAPPED *overlap=&m_FileOverLapped;
				overlap->Offset=li.LowPart;				
				overlap->OffsetHigh=li.HighPart;
				overlap->hEvent=0;

				//now to fill our buffer
				char *FileBuffer;

				if (HiBufferNeedsUpdated) {
					FileBuffer=m_HiFileBuffer;
					m_HiLastReadFP=LastReadFP;
					}
				else {
					FileBuffer=m_LoFileBuffer;
					m_LoLastReadFP=LastReadFP;
					}

				if ((m_LoBufferReading)&&(HiBufferNeedsUpdated)) {
					//we can't have two reads together
					while (!(HasOverlappedIoCompleted(&m_FileOverLapped))) {
						DebugOutput("FileIOWrapper::myRead Stall (2 reads together)\n");
						Sleep(1);
						}
					m_LoBufferReading=false;
					}
				else {
					//We are going to reuse this buffer now... so cancel the read from last time
					CancelIo(hf);
					}

				if ((m_HiBufferReading)&&(!HiBufferNeedsUpdated)) {
					//we can't have two reads together
					while (!(HasOverlappedIoCompleted(&m_FileOverLapped))) {
						DebugOutput("FileIOWrapper::myRead Stall (2 reads together)\n");
						Sleep(1);
						}
					m_HiBufferReading=false;
					}
				else {
					//We are going to reuse this buffer now... so cancel the read from last time
					CancelIo(hf);
					}

				DWORD bytes_read;
				ReadFile(hf,FileBuffer,m_BufferSize,&bytes_read,overlap);

				if (GetLastError()==ERROR_IO_PENDING) {
					if (HiBufferNeedsUpdated) 
						m_HiBufferReading=true;
					else
						m_LoBufferReading=true;
					}
				}
			}

		if (m_LoBufferReading && LoCount) {
			//If we didn't prefetch this read we'll wait for it to complete;
			while (!(HasOverlappedIoCompleted(&m_FileOverLapped))) {
				Sleep(1);
				}
			DebugOutput("FileIOWrapper::myRead Stall LoBufferReading %lx\n",m_FilePointer);
			m_LoBufferReading=false;
			}
		
		if (m_HiBufferReading && HiCount) {
			//If we didn't prefetch this read we'll wait for it to complete;
			while (!(HasOverlappedIoCompleted(&m_FileOverLapped))) {
				Sleep(1);
				}
			DebugOutput("FileIOWrapper::myRead Stall HiBufferReading %lx\n",m_FilePointer);
			m_HiBufferReading=false;
			}

		//Let parent know where the memory has been read
		if (LoDestOffset==0) {
			*OutLowBuf=m_LoFileBuffer+LoSourceOffset;
			*SizeLowBuf=LoCount;
			*OutHiBuf=m_HiFileBuffer+HiSourceOffset;
			*SizeHiBuf=HiCount;
			}
		else {
			*OutLowBuf=m_HiFileBuffer+HiSourceOffset;
			*SizeLowBuf=HiCount;
			*OutHiBuf=m_LoFileBuffer+LoSourceOffset;
			*SizeHiBuf=LoCount;
			}

		m_FilePointer+=count;
		Ret=(int)(min(count,max(m_FileSize-m_FilePointer,0)));
		//success
		} while(false);

	return Ret;
	}


int FileIOWrapper::myClose() {
	HANDLE hf=m_hf;
	int value=-1;
	if (CloseHandle(hf)) value=0;
	return (value);
	}







//Use Buffering (Kept here for benchmark purposes)
#ifndef __USE_NO_BUFFERING__
HANDLE OpenReadSeq(char *filename) {
	HANDLE hf;
	hf=CreateFile(filename,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		//FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);
	if (hf==INVALID_HANDLE_VALUE) hf=(void *)-1;
	return (hf);
	}

__int64 mySeek64 (HANDLE hf,__int64 distance,DWORD MoveMethod) {
   LARGE_INTEGER li;
   li.QuadPart = distance;
   li.LowPart = SetFilePointer (hf,li.LowPart,&li.HighPart,MoveMethod);
	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
		li.QuadPart = -1;
		}
	return li.QuadPart;
	}

__int64 myTell64 (HANDLE hf) {
   LARGE_INTEGER li;
   li.QuadPart = 0;
   li.LowPart = SetFilePointer (hf,li.LowPart,&li.HighPart,FILE_CURRENT);
	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
		li.QuadPart = -1;
		}
	return li.QuadPart;
	}

DWORD myTell (HANDLE hf) {
	DWORD distance;
   distance = SetFilePointer (hf,0,NULL,FILE_CURRENT);
	if (distance == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
		distance = -1;
		}
	return distance;
	}

DWORD mySeek (HANDLE hf,DWORD distance,DWORD MoveMethod) {
	DWORD bytes_read;
   bytes_read=SetFilePointer (hf,distance,NULL,MoveMethod);
	if (bytes_read == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
		bytes_read = -1;
		}
	return (bytes_read);
	}
 
int myRead(HANDLE hf,void *buf,DWORD count) {
	DWORD bytes_read;
	if (!(ReadFile(hf,buf,count,&bytes_read,NULL))) bytes_read=-1;
	return ((int)bytes_read);
	}

int myClose(HANDLE hf) {
	int value=-1;
	if (CloseHandle(hf)) value=0;
	return (value);
	}

DWORD myGetFileSize(HANDLE hf,LPDWORD HighPart) {
	return GetFileSize(hf,HighPart);
	}

#endif __USE_NO_BUFFERING__





void DebugOutput(char *format, ... )
{	char Temp[4096];
	va_list marker;
	va_start(marker,format);
		vsprintf(Temp,format,marker);
		OutputDebugString(Temp);
		//APIDebugOutput(Temp);
	va_end(marker); 
}

