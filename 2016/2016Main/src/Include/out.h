/****************************** Header ******************************\
Class Name:  Out
Summary: 	 Stream redirector to cout and Log.
Project:     FRC2016
Copyright (c) BroncBotz.
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\***************************************************************************/

#ifndef SRC_INCLUDE_OUT_H_
#define SRC_INCLUDE_OUT_H_

#include <iostream>
#include <sstream>
#include <string>
#include "Log.h"

/*! Out is a stream redirector, it function like cout but also appends to the log.*/
class Out final
{
public:
  // for regular output of variables and stuff
  template<typename T> Out& operator<<(const T& out_stream)
  {
	ostringstream convert;//!< Stream to hold string like data.
    std::cout << out_stream;
    convert << out_stream;
    Log::Instance().AppendNoReturn(convert.str());
    return *this;
  }
  // for manipulators like std::endl
  typedef std::ostream& (*stream_function)(std::ostream&);
  Out& operator<<(stream_function func)
  {
    func(std::cout);
    return *this;
  }
};

#endif /* SRC_INCLUDE_OUT_H_ */
