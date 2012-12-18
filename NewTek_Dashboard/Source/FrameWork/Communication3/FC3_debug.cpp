#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::debug;

const bool FrameWork::Communication3::debug::debug_cls( void )
{	// Get the length of the string
	return debug_output( FrameWork::Communication3::debug::config::p_default_control_message_category, FrameWork::Communication3::debug::config::p_default_control_message_cls );
}

// We have to be careful to avoid recursively calling myself due to the fact that the creation of the
// first memory pools also sends the first debug calls that create the first memory pools recursively.
const bool FrameWork::Communication3::debug::debug_output( const wchar_t *p_category, const wchar_t *p_format, va_list args )
{	// This is no longer an option !
	if ( ( !p_category ) || ( p_category[ 0 ] == 0 ) )
	{	assert( false );
		p_category = L"";
	}

	// Get the length of the string
	const DWORD cat_length = (DWORD)::wcslen( p_category );
	const DWORD msg_length = (DWORD)::_vscwprintf( p_format, args );

	// Create the message
	FrameWork::Communication3::debug::message		msg( std::pair<DWORD,DWORD>( cat_length, msg_length ) );

	// Set the category
	::memcpy( (void*)msg.category(), (void*)p_category, sizeof( wchar_t ) * ( cat_length + 1 ) );

		// Create the string
#pragma warning( push )
#pragma warning( disable : 4996 ) // Depreciated, but it should not be !
	wchar_t *p_dst = msg.content();
	::_vsnwprintf( p_dst, msg_length + 1, p_format, args );
#pragma warning( pop )

	// Remove carriage returns
	wchar_t *p_dst_e = msg.content() + msg_length - 1;
	while( ( p_dst_e >= p_dst ) && ( ( p_dst_e[ 0 ] == '\n' ) || ( p_dst_e[ 0 ] == '\r' ) ) )
	{	p_dst_e[ 0 ] = 0;
		p_dst_e--;
	}

	// We first ping the destination
	return FC3::utilities::safe_send_message( msg, config::p_default_debug_server    ) |
		   FC3::utilities::safe_send_message( msg, config::p_default_debug_server_ex );
}

const bool FrameWork::Communication3::debug::debug_output( const wchar_t *p_category, const wchar_t *p_format, ... )
{	// Get the arguments
	va_list args;
	va_start( args, p_format );
	return debug_output( p_category, p_format, args );
}