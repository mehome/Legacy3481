#include "../stdafx.h"

#define __USE_NO_BUFFERING__

#ifdef __USE_NO_BUFFERING__
#define BYTESPERSECTOR 512

inline __int64 AlignBPS64(__int64 number) {
	//add number up and mask out bits
	return ((number+BYTESPERSECTOR-1)&(~(BYTESPERSECTOR-1)));
	}

inline DWORD AlignBPS(DWORD number) {
	//add number up and mask out bits
	return ((number+BYTESPERSECTOR-1)&(~(BYTESPERSECTOR-1)));
	}

struct myFileHandle /*Is Cool! ;)*/ {
	HANDLE BufferedHandle;
	HANDLE UnBuffedHandle;
	};

HANDLE OpenReadSeq(char *filename) {
	myFileHandle *myhf=new myFileHandle();
	HANDLE Ret=(void *)-1;

	do {
		HANDLE hf;

		hf=CreateFile(filename,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			//FILE_ATTRIBUTE_NORMAL,
			FILE_FLAG_SEQUENTIAL_SCAN,
			NULL);
		myhf->BufferedHandle=hf;
		if (hf==INVALID_HANDLE_VALUE) break;

		hf=CreateFile(filename,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_NO_BUFFERING|FILE_FLAG_SEQUENTIAL_SCAN,
			NULL);
		myhf->UnBuffedHandle=hf;

		if (hf==INVALID_HANDLE_VALUE) break;

		//success
		Ret=(void *)myhf;
		} while(false);

	if (Ret==(void *)-1) {
		delete myhf;
		}

	return (Ret);
	}

__int64 mySeek64 (HANDLE hf,__int64 distance,DWORD MoveMethod) {
	__int64 Ret=-1;
	LARGE_INTEGER li;
	li.QuadPart=distance;

	do {
		if (hf==(void *)-1) {
			throw "OpenReadSeq Invalid";
			break;
			}
		myFileHandle *myhf=(myFileHandle *)hf;
		HANDLE TempHF=myhf->BufferedHandle;

		li.LowPart = SetFilePointer (TempHF,li.LowPart,&li.HighPart,MoveMethod);
		if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) break;

		//success
		Ret = distance;
		} while(false);

	return Ret;
	}


__int64 myTell64 (HANDLE hf) {
	myFileHandle *myhf=(myFileHandle *)hf;
	HANDLE TempHF=myhf->BufferedHandle;
	LARGE_INTEGER li;
	li.QuadPart = 0;
	li.LowPart = SetFilePointer (TempHF,li.LowPart,&li.HighPart,FILE_CURRENT);
	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
		li.QuadPart = -1;
		}
	return li.QuadPart;
	}

DWORD myTell (HANDLE hf) {
	myFileHandle *myhf=(myFileHandle *)hf;
	HANDLE TempHF=myhf->BufferedHandle;
	DWORD distance;
	distance = SetFilePointer (TempHF,0,NULL,FILE_CURRENT);
	if (distance == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
		distance = -1;
		}
	return distance;
	}

DWORD mySeek (HANDLE hf,DWORD distance,DWORD MoveMethod) {
	DWORD Ret=-1;
	LARGE_INTEGER li;
	li.LowPart=distance;
	li.HighPart=0;

	do {
		if (hf==(void *)-1) {
			throw "OpenReadSeq Invalid";
			break;
			}
		myFileHandle *myhf=(myFileHandle *)hf;
		HANDLE TempHF=myhf->BufferedHandle;

		li.LowPart = SetFilePointer (TempHF,li.LowPart,&li.HighPart,MoveMethod);
		if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) break;

		//success
		Ret = distance;
		} while(false);

	return Ret;
	}
 
int myRead(HANDLE hf,void *buf,DWORD count) {
	int Ret=-1;

	do {
		if (hf==(void *)-1) {
			throw "OpenReadSeq Invalid";
			break;
			}
		myFileHandle *myhf=(myFileHandle *)hf;
		//Lets figure how many bytes the buffered and unbuffered will take
		long buf1Count,unbufCount,buf2Count;

		//We'll ensure the Unbuffered file pointer is aligned on a sector and find out where the buffered file pointer
		//is at the same time!
		LARGE_INTEGER li, bufferedli;
		li.QuadPart=0;
		if (count>=BYTESPERSECTOR) {
			HANDLE TempHF=myhf->BufferedHandle;

			li.LowPart = SetFilePointer (TempHF,li.LowPart,&li.HighPart,FILE_CURRENT);
			if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) break;

			//save this position
			bufferedli.QuadPart=li.QuadPart;
			//Note li now is equal to the new position
			//Align li up to the nearest sector
			li.QuadPart=AlignBPS64(li.QuadPart);
			TempHF=myhf->UnBuffedHandle;
			//now to explicitly set the unbuffered pointer to a block aligned position
			li.LowPart = SetFilePointer (TempHF,li.LowPart,&li.HighPart,FILE_BEGIN);
			if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) break;


			//Now we know the distance from the buffered to the unbuffered
			buf1Count=min((DWORD)(li.QuadPart-bufferedli.QuadPart),count); //Note: this value should always be less than 512!
			//how many can we read in the unbuffered?
			unbufCount=(count-buf1Count)&(~(BYTESPERSECTOR-1));
			//now for the remainder
			buf2Count=count-(buf1Count+unbufCount);
			}
		else {
			buf1Count=count;
			unbufCount=buf2Count=0;
			}

		//Now the filepointers are all set and we know the amount to adjust each
		DWORD bytes_read,total_bytes_read=0;
		HANDLE bufferedHF=myhf->BufferedHandle;
		HANDLE unbuffedHF=myhf->UnBuffedHandle;

		if (buf1Count>0) {
			if (!(ReadFile(bufferedHF,buf,buf1Count,&bytes_read,NULL))) break;
			total_bytes_read+=bytes_read;
			buf=((char *)buf+bytes_read);
			}
		if (unbufCount>0) {
			if (!(ReadFile(unbuffedHF,buf,unbufCount,&bytes_read,NULL))) break;

			//We also need to advance the filepointer of the buffered handle by this amount
			li.HighPart=0;
			li.LowPart=bytes_read;
			li.LowPart = SetFilePointer (bufferedHF,li.LowPart,&li.HighPart,FILE_CURRENT);
			if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) break;

			total_bytes_read+=bytes_read;
			buf=((char *)buf+bytes_read);
			}
		if (buf2Count>0) {
			if (!(ReadFile(bufferedHF,buf,buf2Count,&bytes_read,NULL))) break;
			total_bytes_read+=bytes_read;
			buf=((char *)buf+bytes_read);
			}

		//Success
		Ret=total_bytes_read;
		} while(false);

	return Ret;
	}




int myClose(HANDLE hf) {
	int Ret=0;
	if (hf!=(void *)-1) {
		myFileHandle *myhf=(myFileHandle *)hf;
		HANDLE TempHF=myhf->BufferedHandle;

		if (!(CloseHandle(TempHF))) Ret=-1;

		TempHF=myhf->UnBuffedHandle;
		if (!(CloseHandle(TempHF))) Ret=-1;

		delete myhf;
		}
	else {
		throw "OpenReadSeq Invalid";
		Ret=-1;
		}
	return Ret;
	}





#endif __USE_NO_BUFFERING__

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

