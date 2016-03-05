/*
 * out.h
 *
 *  Created on: 26 Oct 2015
 *      Author: cooper.ryan
 */

#ifndef SRC_INCLUDE_OUT_H_
#define SRC_INCLUDE_OUT_H_

#include <iostream>
#include <sstream>
#include <string>
#include "Log.h"

class Out final
{
public:
  // for regular output of variables and stuff
  template<typename T> Out& operator<<(const T& out_stream)
  {
	ostringstream convert;
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
