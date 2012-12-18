#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FrameWork::Communication3::implementation;

// Does a force mapping of pages.
volatile BYTE mem_reset_do_not_optimize_out;
void mem_reset( const BYTE *p_start, const BYTE *p_end )
{	// Just read in every page.
	static const size_t page_size = 4096;
	BYTE sum = 0;
	for( const BYTE *p_scan = p_start; p_scan < p_end; p_scan += page_size )
		sum += p_scan[ 0 ];

	// By having a global volatile this ensures that this function is not totally optimized out.
	mem_reset_do_not_optimize_out += sum;
}

bool mapped_file::setup( const wchar_t filename[], const DWORD size, const bool force_page_mapped, const bool must_exist )
{	// Check we are being called correctly
	assert( !m_mapping_handle );
	assert( !m_p_memory );
	assert( !m_p_memory_end );
	
	// Just in case
	m_mapping_handle = ::CreateFileMappingW( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (DWORD)size, filename );
	if ( !m_mapping_handle ) 
		return false;

	// Did I create this item
	const bool created = ( ::GetLastError() != ERROR_ALREADY_EXISTS );

	// Check that it must exist first
	if ( ( must_exist ) && ( created ) )
	{	// We are trying to access something that must exist
		return false;
	}
	else
	{	// Map the view of the file
		m_p_memory = (BYTE*)::MapViewOfFile( m_mapping_handle, FILE_MAP_ALL_ACCESS, 0, 0, size );
		if ( !m_p_memory ) 
			return false;

		// Set the end pointer
		m_p_memory_end = m_p_memory + size;

		// This can be used to reduce page faults at run-time, although I currently do not
		// believe it makes much difference.
		//mem_reset( m_p_memory, m_p_memory_end );
	}

	// Success
	return true;
}

mapped_file::mapped_file( void )
	:	m_mapping_handle( NULL ), m_p_memory( NULL ), m_p_memory_end( NULL )
{	
}

// Constructor
mapped_file::mapped_file( const wchar_t filename[], const DWORD size, const bool force_page_mapped, const bool must_exist )
	:	m_mapping_handle( NULL ), m_p_memory( NULL ), m_p_memory_end( NULL )
{	// Setup
	setup( filename, size, force_page_mapped, must_exist );
}

// Destructor
mapped_file::~mapped_file( void )
{	// Unmap the view of the file
	if ( m_p_memory )
		::UnmapViewOfFile( m_p_memory );

	// Close the handle.
	if ( m_mapping_handle )
		::CloseHandle( m_mapping_handle );
}

// Get the size (0 means error)
const DWORD mapped_file::size( void ) const
{	// Get the size
	return (DWORD)( m_p_memory_end - m_p_memory );
}

// Was there an error
const bool mapped_file::error( void ) const
{	return m_p_memory_end ? false : true;
}

// Get a pointer with index
BYTE* mapped_file::ptr( const DWORD addr )
{	// Return the pointer
	if ( !m_p_memory ) return NULL;
	BYTE *p_ret = m_p_memory + addr;
	assert( ( p_ret >= m_p_memory ) && ( p_ret < m_p_memory_end ) );
	return p_ret;
}

const BYTE* mapped_file::ptr( const DWORD addr ) const
{	// Return the pointer
	if ( !m_p_memory ) return NULL;
	const BYTE *p_ret = m_p_memory + addr;
	assert( ( p_ret >= m_p_memory ) && ( p_ret < m_p_memory_end ) );
	return p_ret;
}

// Get a pointer
BYTE* mapped_file::ptr( void )
{	// Return the pointer
	return m_p_memory;
}

const BYTE* mapped_file::ptr( void ) const
{	// Return the pointer
	return m_p_memory;
}

DWORD mapped_file::addr( const BYTE* ptr )
{	assert( ( ptr >= m_p_memory ) && ( ptr < m_p_memory_end ) );
	return (DWORD)( ptr - m_p_memory );
}

const DWORD mapped_file::addr( const BYTE* ptr ) const
{	assert( ( ptr >= m_p_memory ) && ( ptr < m_p_memory_end ) );
	return (DWORD)( ptr - m_p_memory );
}