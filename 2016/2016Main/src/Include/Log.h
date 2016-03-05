/****************************** Header ******************************\
Class Name:  Log
Summary: 	 Static logging class that manages the runtime log
Project:     FRC2016
Copyright (c) BroncBotz.
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\***************************************************************************/
#ifndef SRC_CONFIG_LOG_H_
#define SRC_CONFIG_LOG_H_

#include <string>

#include "Singleton.h"

using namespace std;

enum Logs//!< Enumeration of the type of logs to store.
{
	kLog,//!< Standard log that is overwritten when the program starts.
	kLongLong,//!< Log that is not overwritten, but rather appended to until manually deleted.
	kBoth//!< Use both logs.
};

/*! Log inherits from the singleton base and is used for storing logs of runtime performance */
class Log final : public Singleton<Log>
{
	friend class Singleton<Log>;
public:
	 void Append(string);//!< Appends a string to the log[s].
	 void AppendNoReturn(string);//!< Appends a string to the log[s], but without a cartrage return.
	 bool DestroyLog(Logs = Logs::kLog);//!< Destroys the log[s].
	 void SetLog(const char *logFile = "log.txt"){ this->logFile = logFile; isUsingLongLog = false; }//!< Sets the log to read and write to, it will create the log if non existant.
	 void SetLog_SetLongLog(const char *logFile = "log.txt", const char *longLogFile = "long_log.txt") { this->logFile = logFile; this->longLogFile = longLogFile; isUsingLongLog = true;}//!< Sets the long log to read and write to, it will create the log if non existant.

private:
	bool isUsingLongLog;//!< Private boolean flag to toggle long log usage.
	const char *logFile;//!< The location of the log file.
	const char *longLogFile;//!< The location of the long log file.
};

#endif /* SRC_CONFIG_LOG_H_ */
