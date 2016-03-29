#include "StdAfx.h"
#include "FrameWork.XML.h"

using namespace FrameWork::xml;


bool FrameWork::xml::load_from_file( const wchar_t *p_filename, node base_node[] )
{	// Open the file for reading
	HANDLE h_file = ::CreateFileW( p_filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if ( h_file == INVALID_HANDLE_VALUE ) return false;
	
	// Error
	bool xml_error = false;

	// Create a parser
	parser	local_parser( base_node );

	// The block file to read from the file
	const DWORD block_size = 4096;

	// Read from the file
	while( true )
	{	// A temporary block
		char	block[ block_size ];

		// The size to read
		DWORD	read_size = block_size;
		::ReadFile( h_file, block, block_size, &read_size, NULL );		

		// Add it to the parser
		xml_error = local_parser( block, read_size );

		// Finished ?
		if ( read_size != block_size ) break;
	}

	// Close the file
	::CloseHandle( h_file );

	// Success
	return !local_parser.error();
}