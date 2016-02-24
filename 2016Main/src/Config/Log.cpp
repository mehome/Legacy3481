/****************************** Header ******************************\
Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/
#include <string>
#include <ctime>
#include <iostream>
#include <fstream>
#include <cstdio>

#include "Log.h"

using namespace std;

bool Log::DestroyLog(Logs log)
{
	if(logFile==NULL)
		logFile="log.txt";
	if(longLogFile==NULL)
		longLogFile="long_log.txt";

	bool clean = true;
	bool clean2 = true;

	switch(log)
	{
	case Logs::kLog:
	 	 if(remove(logFile) != 0)
	 		 clean = false;
	 	 break;
	case Logs::kLongLong:
		 if(remove(longLogFile) != 0)
			 clean2 = false;
		 break;
	case Logs::kBoth:
		 if(remove(logFile) != 0)
			 clean = false;
		 if(remove(longLogFile) != 0)
			 clean2 = false;
		 break;
	}
	return clean&&clean2;
}

void Log::Append(string str)
{
	  string getTime(bool = false);

	  ofstream log;
	  log.open (logFile);
	  log << str << "-[" << getTime() << "]\n";
	  log.close();

	  if(isUsingLongLog)
	  {
		  ofstream log;
		  log.open (longLogFile);
		  log << str << "-[" << getTime(true) << "]\n";
		  log.close();
	  }
}

void Log::AppendNoReturn(string str)
{
	  string getTime(bool = false);

	  ofstream log;
	  log.open (logFile);
	  log << str << " ";
	  log.close();

	  if(isUsingLongLog)
	  {
		  ofstream log;
		  log.open (longLogFile);
		  log << str << " ";
		  log.close();
	  }
}

string getTime(bool includeDate = false)
{
	  time_t rawtime;
	  struct tm * timeinfo;
	  char buffer[80];

	  time (&rawtime);
	  timeinfo = localtime(&rawtime);

	  if(!includeDate)
		  strftime(buffer,80,"%I:%M:%S",timeinfo);
	  else
		  strftime(buffer,80,"%d:%m:%Y : %I:%M:%S",timeinfo);
	  std::string str(buffer);
 return str;
}

