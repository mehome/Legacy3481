#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::implementation;

#define MS_VC_EXCEPTION 0x406D1388

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{	DWORD dwType;		// Must be 0x1000.
	LPCSTR szName;		// Pointer to name (in user addr space).
	DWORD dwThreadID;	// Thread ID (-1=caller thread).
	DWORD dwFlags;		// Reserved for future use, must be zero.

}	THREADNAME_INFO;

#pragma pack(pop)

bool FrameWork::Communication3::implementation::set_thread_name( const char *p_name )
{
	return set_thread_name( ::GetCurrentThreadId(), p_name );
}

bool FrameWork::Communication3::implementation::set_thread_name( const DWORD dwThreadID, const char *p_name )
{	// Set the information
	THREADNAME_INFO info = { 0x1000, p_name, dwThreadID, 0 };

	// Raise the exception
	__try { ::RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info ); }
   __except( EXCEPTION_EXECUTE_HANDLER ) {}

   // Success
	return true;
}