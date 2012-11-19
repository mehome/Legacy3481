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
		// The bitmap template
		#include "bitmap.h"
		#include "bitmap-inc.h"

		#include "bitmap_422.h"
		#include "bitmap_422-inc.h"

		// Basic pixel implementations
		#include "pixel_types.h"
		#include "bitmap_types.h"

		// Color conversions
		#include "color_conversions.h"		
	}

#include "Misc.h"

typedef int work_flags;
static const work_flags do_not_wait   = 1;
static const work_flags long_function = 2;

#include "wait_item.h"
#include "wait.h"

//#define framework_work_fcn_name_c				fcncall
//#define framework_work_fcn_name_c_ex			fcncall_ex
//#define framework_work_fcn_name_c_ret			fcncall
//#define framework_work_fcn_name_c_ret_ex		fcncall_ex
#define framework_work_fcn_name_cpp				fcncall
#define framework_work_fcn_name_cpp_ex			fcncall_ex
#define framework_work_fcn_name_cpp_ret			fcncall
#define framework_work_fcn_name_cpp_ret_ex		fcncall_ex

		//namespace c
		//{	
		//	#include "fcncall_c.h"		
		//};

		namespace cpp
		{
			#include "fcncall_cpp.h"
		};

//#undef	framework_work_fcn_name_c				// fcncall_c
//#undef	framework_work_fcn_name_c_ex			// fcncall_c_ex
//#undef	framework_work_fcn_name_c_ret			// fcncall_c_ret
//#undef	framework_work_fcn_name_c_ret_ex		// fcncall_c_ret_ex
#undef	framework_work_fcn_name_cpp				// fcncall_cpp
#undef	framework_work_fcn_name_cpp_ex			// fcncall_cpp_ex
#undef	framework_work_fcn_name_cpp_ret			// fcncall_cpp_ret
#undef	framework_work_fcn_name_cpp_ret_ex		// fcncall_cpp_ret_ex

#define	framework_work_use_thread_param
#define	framework_work_use_apc					h_thread
//#define framework_work_fcn_name_c				threadcall
//#define framework_work_fcn_name_c_ex			threadcall_ex
//#define framework_work_fcn_name_c_ret			threadcall
//#define framework_work_fcn_name_c_ret_ex		threadcall_ex
#define framework_work_fcn_name_cpp				threadcall
#define framework_work_fcn_name_cpp_ex			threadcall_ex
#define framework_work_fcn_name_cpp_ret			threadcall
#define framework_work_fcn_name_cpp_ret_ex		threadcall_ex

		//namespace c
		//{	
		//	#include "fcncall_c.h"		
		//};

		namespace cpp
		{
			#include "fcncall_cpp.h"
		};

//#undef	framework_work_fcn_name_c				// fcncall_c
//#undef	framework_work_fcn_name_c_ex			// fcncall_c_ex
//#undef	framework_work_fcn_name_c_ret			// fcncall_c_ret
//#undef	framework_work_fcn_name_c_ret_ex		// fcncall_c_ret_ex
#undef	framework_work_fcn_name_cpp				// fcncall_cpp
#undef	framework_work_fcn_name_cpp_ex			// fcncall_cpp_ex
#undef	framework_work_fcn_name_cpp_ret			// fcncall_cpp_ret
#undef	framework_work_fcn_name_cpp_ret_ex		// fcncall_cpp_ret_ex
#undef	framework_work_use_apc
#undef	framework_work_use_thread_param		

};	// namespace FrameWork

namespace FTHRDS   = FrameWork;
namespace FTHREADS = FrameWork;