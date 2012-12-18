#ifndef __FileLocking__
#define __FileLocking__

//********************************************************************************************************************
class SBDDLL PlaybackEngine_DriveBlock
{	private:		// The handle to the event that will allow me to read !
					HANDLE DriveBlock;

					// The drive to lock for
					char LockFor[2];

	public:			// Call this when you want to start reading
					bool StartReading(unsigned TimeOut=INFINITE);
					void StopReading(void);

					void SetLockingFor(char *FN);
					void FreeLocking(void);

					// The constructor and destructor
					PlaybackEngine_DriveBlock(char *FN=NULL);
					~PlaybackEngine_DriveBlock(void);
};

#endif