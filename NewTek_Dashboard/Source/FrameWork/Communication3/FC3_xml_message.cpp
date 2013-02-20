#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::xml;

message::message( const FrameWork::xml::tree &xml_tree, const DWORD extra_data_size )
	:	m_ref( 1 ), FrameWork::Communication3::implementation::message( ( extra_data_size ? ( extra_data_size + sizeof(header) ) : 0 ) +  
																		(DWORD)( ( xml_tree.output_length() + 1 ) * sizeof( wchar_t ) ) )
{	// Copy the data
	if ( !FrameWork::Communication3::implementation::message::error() )
	{	if ( extra_data_size )
		{	// Set the type
			type() = message_type_xml2;

			// Setup the debugging string
			m_p_header = (header*)ptr();
			m_p_header->m_extra_data_size     = extra_data_size;
			m_p_header->m_extra_data_size_max = extra_data_size;
			m_p_header->m_extra_data_fourcc = 0;
			m_p_extra_data = (BYTE*)( 1 + m_p_header );
			m_p_data_A = (char*)( m_p_extra_data + m_p_header->m_extra_data_size_max );
		
			// Setup
			xml_tree.output( m_p_data_W );
		}
		else
		{	// Set the type
			type() = message_type_xml;

			// Setup the debugging string
			m_p_header	   = NULL;
			m_p_extra_data = NULL;
			m_p_data_A	   = (char*)ptr();

			// Copy the string
			xml_tree.output( m_p_data_W );
		}
	}
}

message::message( const FrameWork::xml::node2 &xml_tree, const DWORD extra_data_size )
	:	m_ref( 1 ), FrameWork::Communication3::implementation::message( ( extra_data_size ? ( extra_data_size + sizeof(header) ) : 0 ) +  
																		(DWORD)( ( xml_tree.output_length() ) * sizeof( wchar_t ) ) )
{	// Copy the data
	if ( !FrameWork::Communication3::implementation::message::error() )
	{	if ( extra_data_size )
		{	// Set the type
			type() = message_type_xml2;

			// Setup the debugging string
			m_p_header = (header*)ptr();
			m_p_header->m_extra_data_size     = extra_data_size;
			m_p_header->m_extra_data_size_max = extra_data_size;
			m_p_header->m_extra_data_fourcc = 0;
			m_p_extra_data = (BYTE*)( 1 + m_p_header );
			m_p_data_A = (char*)( m_p_extra_data + m_p_header->m_extra_data_size_max );
		
			// Fill out the data
			xml_tree.output( m_p_data_W );
		}
		else
		{	// Set the type
			type() = message_type_xml;

			// Setup the debugging string
			m_p_header	   = NULL;
			m_p_extra_data = NULL;
			m_p_data_A	   = (char*)ptr();

			// Copy the string
			xml_tree.output( m_p_data_W );
		}
	}
}

// Constructor
message::message( const wchar_t xml_data[], const DWORD extra_data_size )
	:	m_ref( 1 ), FrameWork::Communication3::implementation::message( ( extra_data_size ? ( extra_data_size + sizeof(header) ) : 0 ) +  
																		(DWORD)( ( ::wcslen( xml_data ) + 1 ) * sizeof( wchar_t ) ) )
{	// Copy the data
	if ( !FrameWork::Communication3::implementation::message::error() )
	{	if ( extra_data_size )
		{	// Set the type
			type() = message_type_xml2;

			// Setup the debugging string
			m_p_header = (header*)ptr();
			m_p_header->m_extra_data_size     = extra_data_size;
			m_p_header->m_extra_data_size_max = extra_data_size;
			m_p_header->m_extra_data_fourcc = 0;
			m_p_extra_data = (BYTE*)( 1 + m_p_header );
			m_p_data_A = (char*)( m_p_extra_data + m_p_header->m_extra_data_size_max );

			// Fill out the string
			::wcscpy( m_p_data_W, xml_data );
		}
		else
		{	// Set the type
			type() = message_type_xml;

			// Setup the debugging string
			m_p_header	   = NULL;
			m_p_extra_data = NULL;
			m_p_data_A	   = (char*)ptr();

			// Copy the string
			::wcscpy( m_p_data_W, xml_data );
		}
	}
}

message::message( const char xml_data[], const DWORD extra_data_size )
	:	m_ref( 1 ), FrameWork::Communication3::implementation::message( ( extra_data_size ? ( extra_data_size + sizeof(header) ) : 0 ) + 
																		(DWORD)( ( ::strlen( xml_data ) + 1 ) * sizeof( char ) ) )
{	// Copy the data
	if ( !FrameWork::Communication3::implementation::message::error() )
	{	if ( extra_data_size )
		{	// Set the type
			type() = message_type_xml2;
		
			// Setup the debugging string
			m_p_header = (header*)ptr();
			m_p_header->m_extra_data_size     = extra_data_size;
			m_p_header->m_extra_data_size_max = extra_data_size;
			m_p_header->m_extra_data_fourcc = 0;
			m_p_extra_data = (BYTE*)( 1 + m_p_header );
			m_p_data_A = (char*)( m_p_extra_data + m_p_header->m_extra_data_size_max );

			// Copy the string
			::strcpy( m_p_data_A, xml_data );
		}
		else
		{	// Set the type
			type() = message_type_xml;

			// Setup the debugging string
			m_p_header	   = NULL;
			m_p_extra_data = NULL;
			m_p_data_A	   = (char*)ptr();

			// Copy the string
			::strcpy( m_p_data_A, xml_data );
		}
	}
}

message::message( const DWORD size_in_bytes )
	:	m_ref( 1 ), FrameWork::Communication3::implementation::message( size_in_bytes )
{	// Set the type
	if ( !FrameWork::Communication3::implementation::message::error() )
	{	// Set the type
		type() = message_type_xml;

		// Setup the debugging string
		m_p_header	   = NULL;
		m_p_extra_data = NULL;
		m_p_data_A	   = (char*)ptr();
	}
}

// Internal use only :)
message::message( const DWORD block_id, const DWORD addr )
	:	m_ref( 1 ), FrameWork::Communication3::implementation::message( block_id, addr )
{	// Setup the debugging string
	switch( type() )
	{	case message_type_xml:
			// The data is the whole message
			m_p_data_A = (char*)ptr();
			m_p_header = NULL;
			break;

		case message_type_xml2:
			// We have a header
			// Setup the debugging string
			m_p_header = (header*)ptr();
			m_p_extra_data = (BYTE*)( 1 + m_p_header );
			m_p_data_A = (char*)( m_p_extra_data + m_p_header->m_extra_data_size_max );
			break;

		default:
			__assume( false ); 
			assert( false );
			break;
	}
}

message::~message( void )
{	// Help debug people ussing messages wrong
	if ( m_p_data_A[0] )
			assert( ::strlen( m_p_data_A )*sizeof(char)	   < size() );
	else	assert( ::wcslen( m_p_data_W )*sizeof(wchar_t) < size() );
}

// Is there an error in this message, most likely caused by a failed allocation or transmission
bool message::error( void ) const
{	if ( FrameWork::Communication3::implementation::message::error() ) return true;
	return ( ( type() != message_type_xml ) && ( type() != message_type_xml2 ) );
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
	return m_p_data_A;
}

message::operator char* ( void )
{	assert( !error() );
	return m_p_data_A;
}

message::operator const wchar_t* ( void ) const
{	assert( !error() );
	return m_p_data_W;
}

message::operator wchar_t* ( void )
{	assert( !error() );
	return m_p_data_W;
}

// Parse this message
const bool message::parse( FrameWork::xml::node *p_node ) const
{	// Parse the string.
	return FrameWork::xml::load_from_string( (const char*)m_p_data_A, p_node );
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
{	// Get the total message size
	DWORD ret = __super::size();

	// We support two versions of XML messages
	if ( type() == message_type_xml2 ) 
	{	assert( m_p_header );
		ret -= sizeof(header);
		ret -= m_p_header->m_extra_data_size_max;
	}
	return ret;
}

// This will ficegive you access to the extra data that can be allocated along with a frame.
// Obviously this requires the sender and receiver to have a common understanding of what is
// actually stored in the data itself.
const void* message::extra_data( void ) const
{	return m_p_extra_data;
}

void* message::extra_data( void )
{	return m_p_extra_data;
}

// This allows you to reduce the extra data size. Treat this with caution.
void message::set_extra_data_size( const int new_data_size )
{	assert( m_p_header );
	assert( !error() );
	assert( new_data_size <= m_p_header->m_extra_data_size_max );
	m_p_header->m_extra_data_size = new_data_size;
}

// Get the current extra data size
const int message::extra_data_size( void ) const
{	assert( !error() );
	return m_p_header ? m_p_header->m_extra_data_size : 0;
}

// Get access to the unique code that identifies the "extrea" data attached to this frame.
const DWORD	message::extra_data_fourCC( void ) const
{	assert( !error() );
	return m_p_header ? m_p_header->m_extra_data_fourcc : 0;
}

DWORD &message::extra_data_fourCC( void )
{	static DWORD dummy;
	assert( !error() );
	if ( !m_p_header ) { assert( false ); return dummy; }
	return m_p_header->m_extra_data_fourcc;
}