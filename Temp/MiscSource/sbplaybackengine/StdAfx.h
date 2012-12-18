#ifndef __StdAfx__
#define __StdAfx__

#define __SBENGINE_Compiling_Locally__
//#define _DISPLAYINFO

// This is for debug output from the calculate routine
//#define _DISPLAYINFO_2

// THis displays how many fields ahead of video we have computed
//#define _DISPLAYINFO_3

// The DVE stuff
//#define _DISPLAYINFO_DVE

void g_BufferCache_Init(bool Init);

#include "../Controls_SBPlaybackEngineDLL.h"

// This is the HINSTANCE of the DLL
extern HINSTANCE h_SBPB_HINSTANCE;

// This is the global CPU limiter
extern CPU_Limiter GlobalCPULimiter;
extern void DriveItems_Cache_Init(bool Init);

// The member you can use to allocate these things
extern DVECache g_DVECache_Allocate;

#endif