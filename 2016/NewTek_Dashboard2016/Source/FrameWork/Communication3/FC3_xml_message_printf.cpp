#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FC3::xml;

FC3::xml::message* FC3::xml::printf( const wchar_t* p_format, ... )
{	// Get the arguments
	va_list args;
	va_start( args, p_format );

	// Get the length of the message
	const DWORD msg_length = 1 + (DWORD)::_vscwprintf( p_format, args );

	// Allocate a message
	FC3::xml::message* p_msg = new FC3::xml::message( msg_length*sizeof( wchar_t ) );

	// Create the string
#pragma warning( push )
#pragma warning( disable : 4996 ) // Depreciated, but it should not be !
	wchar_t *p_dst = (*p_msg);
	::_vsnwprintf( p_dst, msg_length, p_format, args );
#pragma warning( pop )

	// Return the result
	return p_msg;
}

FC3::xml::message* FC3::xml::printf( const char* p_format, ... )
{	// Get the arguments
	va_list args;
	va_start( args, p_format );

	// Get the length of the message
	const DWORD msg_length = 1 + (DWORD)::_vscprintf( p_format, args );

	// Allocate a message
	FC3::xml::message* p_msg = new FC3::xml::message( msg_length*sizeof( char ) );

	// Create the string
#pragma warning( push )
#pragma warning( disable : 4996 ) // Depreciated, but it should not be !
	char *p_dst = (*p_msg);
	::_vsnprintf( p_dst, msg_length, p_format, args );
#pragma warning( pop )

	// Return the result
	return p_msg;
}