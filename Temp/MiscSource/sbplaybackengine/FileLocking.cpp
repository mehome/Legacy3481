#include "StdAfx.h"

void PlaybackEngine_DriveBlock::SetLockingFor(char *FN)
{	// Free any previous locking
	FreeLocking();

	// pedantic
	if (!FN) return;
	
	// Which drive is it ?
	char MyDrive[2]= { FN[0], 0 };

	// We always work in uppercase letters
	_strupr(MyDrive);

	// Get the named event
	char Temp[256];
	sprintf(Temp,"NewTek:DriveBlock:%s",MyDrive);

	// Get the file lock on this drive
	DriveBlock=CreateEvent(NULL,false,true,Temp);
	if (!DriveBlock) _throw("PlaybackEngine_DriveBlock::PlaybackEngine_DriveBlock unable to find interprocess file blocker.");
}

void PlaybackEngine_DriveBlock::FreeLocking(void)
{	if (DriveBlock) CloseHandle(DriveBlock);
	DriveBlock=NULL;
}

// Constructor
PlaybackEngine_DriveBlock::PlaybackEngine_DriveBlock(char *FN)
{	DriveBlock=NULL;
	SetLockingFor(FN);
}

// Destructor
PlaybackEngine_DriveBlock::~PlaybackEngine_DriveBlock(void)
{	FreeLocking();
}

bool PlaybackEngine_DriveBlock::StartReading(unsigned TimeOut)
{	// Pedantic
	if (!DriveBlock) return false;
	
	// Wait until someone has triggered the event
	if (WaitForSingleObject(DriveBlock,INFINITE)==WAIT_TIMEOUT) return false;
	return true;
}

void PlaybackEngine_DriveBlock::StopReading(void)
{	// Pedantic
	if (!DriveBlock) return;
	
	// We now trigger the event, signalling that I have loaded all my fields already !s
	SetEvent(DriveBlock);
}

