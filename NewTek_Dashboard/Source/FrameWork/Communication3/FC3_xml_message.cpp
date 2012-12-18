#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::xml;

message::message( const FrameWork::xml::tree &xml_tree )
	:	m_ref( 1 ), 
		FrameWork::Communication3::implementation::message( (DWORD)( ( xml_tree.output_length() + 1 ) * sizeof( wchar_t ) ) )
{	// Copy the data
	if ( !FrameWork::Communication3::implementation::message::error() )
	{	// Setup
		xml_tree.output( (wchar_t*)ptr() );

		// Set the type
		type() = message_type_xml;
	}

	// Setup the debugging string
	m_p_debugW = (const wchar_t*)*this;
	assert( ::wcslen( m_p_debugW )*sizeof(wchar_t) < size() );
}

message::message( const FrameWork::xml::node2 &xml_tree )
	:	m_ref( 1 ), 
		FrameWork::Communication3::implementation::message( (DWORD)( ( xml_tree.output_length() ) * sizeof( wchar_t ) ) )
{	// Copy the data
	if ( !FrameWork::Communication3::implementation::message::error() )
	{	// Setup
		xml_tree.output( (wchar_t*)ptr() );

		// Set the type
		type() = message_type_xml;
	}

	// Setup the debugging string
	m_p_debugW = (const wchar_t*)*this;
}

// Constructor
message::message( const wchar_t xml_data[] )
	:	m_ref( 1 ), 
		FrameWork::Communication3::implementation::message( (DWORD)( ( ::wcslen( xml_data ) + 1 ) * sizeof( wchar_t ) ) )
{	// Copy the data
	if ( !FrameWork::Communication3::implementation::message::error() )
	{	// Copy
		::wcscpy( (wchar_t*)ptr(), xml_data );

		// Set the type
		type() = message_type_xml;
	}

	// Setup the debugging string
	m_p_debugW = (const wchar_t*)*this;
	assert( ::wcslen( m_p_debugW )*sizeof(wchar_t) < size() );
}

message::message( const char xml_data[] )
	:	m_ref( 1 ), 
		FrameWork::Communication3::implementation::message( (DWORD)( ( ::strlen( xml_data ) + 1 ) * sizeof( char ) ) )
{	// Copy the data
	if ( !FrameWork::Communication3::implementation::message::error() )
	{	// Copy
		::strcpy( (char*)ptr(), xml_data );

		// Set the type
		type() = message_type_xml;
	}

	// Setup the debugging string
	m_p_debugW = (const wchar_t*)*this;
}

message::message( const DWORD size_in_bytes )
	:	m_ref( 1 ), 
		FrameWork::Communication3::implementation::message( size_in_bytes )
{	// Set the type
	// Copy the data
	if ( !FrameWork::Communication3::implementation::message::error() )
		type() = message_type_xml;

	// Setup the debugging string
	m_p_debugW = (const wchar_t*)*this;
}

// Internal use only :)
message::message( const DWORD block_id, const DWORD addr )
	:	m_ref( 1 ), 
		FrameWork::Communication3::implementation::message( block_id, addr )
{
	// Setup the debugging string
	m_p_debugW = (const wchar_t*)*this;
}

message::~message( void )
{	// Help debug people ussing messages wrong
	assert( ::wcslen( m_p_debugW )*sizeof(wchar_t) < size() );
	assert( ::strlen( m_p_debugA )*sizeof(char)	   < size() );
}

// Is there an error in this message, most likely caused by a failed allocation or transmission
bool message::error( void ) const
{	if ( FrameWork::Communication3::implementation::message::error() ) return true;
	return ( type() != message_type_xml );
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

message::operator const wchar_t* ( void ) const
{	assert( !error() );
	return (const wchar_t*)ptr();
}

message::operator wchar_t* ( void )
{	assert( !error() );
	return (wchar_t*)ptr();
}

// Parse this message
const bool message::parse( FrameWork::xml::node *p_node ) const
{	// Parse the string.
	return FrameWork::xml::load_from_string( (const char*)ptr(), p_node );
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