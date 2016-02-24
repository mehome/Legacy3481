/****************************** Header ******************************\
Summary: 	 Preprocessor definitions and pragmas used throughout program
Project:     FRC2016
Copyright (c) BroncBotz.
All rights reserved.

Author(s):	Ryan Cooper
Email:	cooper.ryan@centaurisoft.org
\*********************************************************************/
#ifndef INCLUDE_PREPROC_H_
#define INCLUDE_PREPROC_H_

/*! This file just contains preprocessor definitions, nothing program related,
 * only compile-time related stuff.*/

#pragma GCC diagnostic error "-Wdeprecated-declarations"

#define PTR_TO_MEMBER(object,ptrToMember)  ((object).*(ptrToMember))

#define UNBOUNDED "WARNING: This calls an unbounded loop, you must call in separate thread. \
Use #pragma GCC diagnostic ignored \"-Wdeprecated-declarations\" to disable this error"

#define LEGACY "WARNING: This function is only useful for legacy cRio systems, it is not \
needed on the roboRio"

#define DEPRICATED "WARNING: This function is depricated, see header for details"

#endif /* INCLUDE_PREPROC_H_ */
