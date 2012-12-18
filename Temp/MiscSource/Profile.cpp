void ControlDLL Profile_Start();
void ControlDLL Profile_Finish();
void ControlDLL Profile_Begin(char *Name);
void ControlDLL Profile_End();

//--------------------------------------------------------------------------------------------------
__int64 TimeStampStart;
__int64 TimeStampStart_Last;
__int64 TimeStampStart_Total;
__int64 PerfCounter;
unsigned NumberOfHits;
char *m_Name;

__int64 Profile_GetTime(void)
{	LARGE_INTEGER Val1;
	QueryPerformanceCounter(&Val1);	
	return Val1.QuadPart;
}

void Profile_Start(void)
{	if (TimeStampStart_Last!=0) return;
	TimeStampStart_Last=Profile_GetTime();
	NumberOfHits++;
}

void Profile_Finish(void)
{	if (TimeStampStart_Last==0) return;
	TimeStampStart_Total+=Profile_GetTime()-TimeStampStart_Last;
	TimeStampStart_Last=0;
}


void Profile_Begin(char *Name)
{	TimeStampStart=Profile_GetTime();
	TimeStampStart_Last=0;
	TimeStampStart_Total=0;
	NumberOfHits=0;
	m_Name=NewTek_malloc(Name);
	LARGE_INTEGER Val1;
	QueryPerformanceFrequency(&Val1);
	PerfCounter=Val1.QuadPart;
	DebugOutput("Profile::Starting %s\n", m_Name);
}

void Profile_End(void)
{	
	if (TimeStampStart_Last!=0) Profile_Finish();
	__int64 TotalTime=Profile_GetTime()-TimeStampStart;
	DebugOutput("Profile::%s  Total Time=%f   Pc Time=%f\n",
					m_Name, 
					double(TotalTime)/double(PerfCounter),
					100.0*double(TimeStampStart_Total)/double(TotalTime) );
	DebugOutput("Profile::%s  Total Count=%I64d   Pc Count=%I64d\n",
					m_Name, 
					TotalTime,
					TimeStampStart_Total);
	DebugOutput("Number of hits %d\n",NumberOfHits);
	NewTek_free(m_Name);
}
//----------------------------------------------------------------------------------------------------
