#pragma once

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the FRAMEWORKAUDIO2_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// FRAMEWORKAUDIO2_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifndef FRAMEWORKAUDIO2_API
#ifdef FRAMEWORKAUDIO2_EXPORTS
#define FRAMEWORKAUDIO2_API
#else
#define FRAMEWORKAUDIO2_API
#endif
#endif // FRAMEWORKAUDIO2_API

// For the sake of simplicity, this uses bitmaps internally
#include <utility>
#include <complex>
#include "..\FrameWork.h"

namespace FrameWork
{
	namespace Audio2
	{		
		#include "buffer.h"
		#include "buffer-inc.h"
		#include "buffer_types.h"

		#include "fft.h"

		namespace fir
		{	
			#include "fir_filter_types.h"
			#include "fir_filter.h"
		};

		namespace effects
		{
			#include "compressor_limiter.h"
		};

		namespace utils
		{
			#include "interleaved.h"
			#include "dbconversion.h"
			#include "resampler.h"
		};

		#include "dspFilter.h"

		namespace iir
		{	
			#include "iir_filter.h"
		};

		namespace io
		{
			#include "io_wav.h"
		};
	};
};

namespace FAUD = FrameWork::Audio2;