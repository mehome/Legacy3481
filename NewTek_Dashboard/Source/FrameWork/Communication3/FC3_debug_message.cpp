#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::debug;

message::message( const std::pair< DWORD, DWORD > sizes )
	:	m_ref( 1 ), m_p_category( NULL ), m_p_message( NULL ),
		FrameWork::Communication3::implementation::message( header_size + ( sizes.first + sizes.second + 2 ) * sizeof( wchar_t ) )
{	// Error
	if ( !FrameWork::Communication3::implementation::message::error() )
	{	// Set the size
		type() = message_type_debug;

		// Get the header pointer
		m_p_header   = (header*)ptr();

		// Setup the header
		m_p_header->m_category_size = sizes.first;
		m_p_header->m_message_size  = sizes.second;

		// Setup the string pointers
		m_p_category = (wchar_t*)ptr( header_size );
		m_p_message  = m_p_category + sizes.first + 1;
	}
}

// Internal use only :)
message::message( const DWORD block_id, const DWORD addr )
	:	m_ref( 1 ), m_p_category( NULL ), m_p_message( NULL ),
		FrameWork::Communication3::implementation::message( block_id, addr )
{	// Check the size and the type
	if ( !FrameWork::Communication3::implementation::message::error() )
	{	// If the type is correct, set it up.
		if ( type() == message_type_debug )
		{	// Get the header pointer
			m_p_header = (header*)ptr();

			// Setup the string pointers
			m_p_category = (wchar_t*)ptr( header_size );
			m_p_message  = m_p_category + m_p_header->m_category_size + 1;
		}
	}
}

message::~message( void )
{
}

// Is there an error in this message, most likely caused by a failed allocation or transmission
bool message::error( void ) const
{	if ( FrameWork::Communication3::implementation::message::error() ) return true;
	return ( type() != message_type_debug );
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

// Parse this message
const bool message::parse( FrameWork::xml::node *p_node ) const
{	// Parse the string.
	return FrameWork::xml::load_from_string( (const char*)ptr(), p_node );
}

const wchar_t* message::category( void ) const
{	assert( !error() );
	return m_p_category;
}

wchar_t* message::category( void )
{	assert( !error() );
	return m_p_category;
}

const wchar_t* message::content( void ) const
{	assert( !error() );
	return m_p_message;
}

wchar_t* message::content( void )
{	assert( !error() );
	return m_p_message;
}

// This ensures that people can delete the read_with_info structs returned above correctly
void* message::operator new ( const size_t size )
{	return ::malloc( size );
}

void  message::operator delete ( void* ptr )
{	::free( ptr );
}

void* message::operator new [] ( const size_t size )
{	return ::malloc( size );
}

void  message::operator delete [] ( void* ptr )
{	::free( ptr );
}