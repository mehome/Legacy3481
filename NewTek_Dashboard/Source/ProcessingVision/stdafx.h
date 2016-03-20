// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
// for OCV
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <ShellAPI.h>
#include <stdio.h>
#include <Shlobj.h>

#if 0
#define DOUT(...) FrameWork::DebugOutput(__VA_ARGS__);
#else
#define DOUT(...)
#endif

enum histo_mode
{
	h_original,
	h_equalize,
	h_clahe
};

