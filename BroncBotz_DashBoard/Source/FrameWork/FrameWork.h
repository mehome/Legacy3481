#pragma once

// C++ includes
#include <cassert>
#include <vector>
#include <list>
#include <queue>

// The namespace
namespace FrameWork
{
#include "critical_section.h"
#include "thread.h"
#include "event.h"
#include "Time_Type.h"

	namespace Bitmaps
	{
		#include "Bitmap.h"
		#include "bitmap_422-inc.h"
	}

#include "Misc.h"

};	// namespace FrameWork

namespace FTHRDS   = FrameWork;
namespace FTHREADS = FrameWork;