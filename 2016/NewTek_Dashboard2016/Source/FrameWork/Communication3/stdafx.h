// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>

// TODO: reference additional headers your program requires here
#include "..\compiler_settings.h"

// Important includes
#include <cassert>
#include <algorithm>
#include <intrin.h>

// WinSockets
#include <winsock2.h>
#pragma comment( lib, "ws2_32.lib" )

// The Multimedia system
#include <Mmsystem.h>
#pragma comment( lib, "Winmm.lib" )

// Pretend I am the standard library
namespace std
{
	template< class iterator, class eqcmp > __forceinline
	iterator reverse_find( iterator first, iterator last, const eqcmp& value )
	{	iterator i = last;
		while( i != first ) { --i; if ( *i == value ) return i; }
		return last;
	}
};

#define FDEBUG	FrameWork::Debug