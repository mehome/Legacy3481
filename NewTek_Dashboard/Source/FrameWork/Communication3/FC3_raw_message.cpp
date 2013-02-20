#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::raw;

message::message( const DWORD size_in_bytes )
	:	m_ref( 1 ), 
		FrameWork::Communication3::implementation::message( size_in_bytes )
{	// Set the type
	// Copy the data
	if ( !FrameWork::Communication3::implementation::message::error() )
		type() = message_type_raw;
}

message::message( const BYTE* p_data, const DWORD size_in_bytes )
	:	m_ref( 1 ), 
		FrameWork::Communication3::implementation::message( size_in_bytes )
{	// Set the type
	// Copy the data
	if ( !FrameWork::Communication3::implementation::message::error() )
		type() = message_type_raw;

	// Copy the data
	::memcpy( ptr(), p_data, size_in_bytes );
}

// Internal use only :)
message::message( const DWORD block_id, const DWORD addr )
	:	m_ref( 1 ), 
		FrameWork::Communication3::implementation::message( block_id, addr )
{	
}

message::~message( void )
{	
}

// Is there an error in this message, most likely caused by a failed allocation or transmission
bool message::error( void ) const
{	if ( FrameWork::Communication3::implementation::message::error() ) return true;
	return ( type() != message_type_raw );
}

// Reference counting
const long message::addref( void ) const
{	// One more person cares about me
	return ::_InterlockedIncrement( &m_ref );
}

const long message::release( void ) const
{	// Delete ?
	const long ret = ::_InterlockedDecrement( &m_ref );
	assert( ret >= 0 );
	if ( !ret ) delete this;
	return ret;
}

const long message::refcount( void ) const
{	return m_ref;
}

// Cast this to a string
message::operator const char* ( void ) const
{	assert( !error() );
	return (const char*)ptr();
}

message::operator char* ( void )
{	assert( !error() );
	return (char*)ptr();
}

const char* message::data( void ) const
{	assert( !error() );
	return (const char*)ptr();
}

char* message::data( void )
{	assert( !error() );
	return (char*)ptr();
}

// This ensures that people can delete the read_with_info structs returned above correctly
static FC3::implementation::memory_pool< sizeof( message ) > g_mem_alloc;

void* message::operator new ( const size_t size )
{	assert( sizeof( message ) == size );
	return g_mem_alloc.malloc();
}

void  message::operator delete ( void* ptr )
{	return g_mem_alloc.free( ptr );
}

void* message::operator new [] ( const size_t size )
{	return ::malloc( size );
}

void  message::operator delete [] ( void* ptr )
{	::free( ptr );
}

// Get the data size of this message
const DWORD message::size( void ) const
{	return FrameWork::Communication3::implementation::message::size();
}