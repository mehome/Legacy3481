/****************************** Header ******************************\
Class Name:  Singleton
Summary: 	 An inherited single system to create singletons in a more
			 Streamlined fashion.
Project:     FRC2016
Copyright (c) BroncBotz.
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/
#ifndef INCLUDE_SINGLETON_H_
#define INCLUDE_SINGLETON_H_

#include <iostream>

using namespace std;

template <typename T>

/*! This is a template for easily creating classes that are to be singletons.*/
class Singleton
{
public:
	Singleton(){}//!< Default constructor.
	static T& Instance()//!< Manages instance for classes that inherit this.
	{
		static T Instance_;
#pragma GCC diagnostic ignored "-Wunused-variable"
		Singleton<T> &t = Instance_;
		return Instance_;
	}
};

#endif /* INCLUDE_SINGLETON_H_ */
