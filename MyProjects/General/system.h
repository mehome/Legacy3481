#ifndef SYSTEM_H
#define SYSTEM_H

#if defined (WIN32)
	#define IS_WIN32 TRUE
#else
	#define IS_WIN32 FALSE
#endif
#define IS_NT      IS_WIN32 && (BOOL)(GetVersion() < 0x80000000)
#define IS_WIN32S  IS_WIN32 && (BOOL)(!(IS_NT) && (LOBYTE(LOWORD(GetVersion()))<4))
#define IS_WIN95   (BOOL)(!(IS_NT) && !(IS_WIN32S)) && IS_WIN32
//#define _WIN32_WINNT 0x0500

#define _INC_VFW //ensure not to include windows version of vfw
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h> //added to compile for NT
#include <commctrl.h>
#include <commdlg.h>
#include <tchar.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dbt.h>
#include <io.h>

// DirectX includes
#include <dsound.h>
#include <ddraw.h>
#include <dvp.h>
// DirectX media for DV cam
#define __EDEVDEFS__    //don't include edevdefs.h

#include <mmreg.h>
#include <streams.h>
#include <initguid.h>
#include <xprtdefs.h>   //include this instead of edevdefs
#include <msacm.h>
// other multimedia
#undef _INC_VFW
#include "myVFW.h"
//#include <vfw.h>

#include "CBasic.h"
#include "GBasic.h"

#endif /* SYSTEM_H */
