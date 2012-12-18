#include "StdAfx.h"
#include "FrameWork.XML.h"
#include "rapidxml.hpp"
#include "..\Communication3\FC3i_memory_pool.h"

using namespace FXML;

// e.g.		<DDR> </DDR>
// The node type is DDR
const wchar_t *node2::type( void ) const
{	return m_p_name;
}

// The text on this node
const wchar_t *node2::text( void ) const
{	return m_p_text;
}

// Get access to the parameters
// e.g.		<DDR value="1" another_value="2">
// There are two parameters. "value","1"   "another_value","2"
const int node2::no_parameters( void ) const
{	return m_no_attributes;
}

// If you access a parameter past the end, NULl, NULL is returned.
const std::pair< const wchar_t*, const wchar_t* > node2::parameter( const int idx ) const
{	// Debugging
	assert( idx < m_no_attributes );

	// Get the attributes
	attribute_type* p_attribs = (attribute_type*)( sizeof(node2) + (BYTE*)this );
	return p_attribs[ idx ];
}

// Get parameter by name, NULL if not found.
const wchar_t* node2::parameter( const wchar_t name[], const wchar_t *p_default ) const
{	// Get the attributes
	attribute_type* p_attribs = (attribute_type*)( sizeof(node2) + (BYTE*)this );

	// Cycle over the item
	for( int i=0; i<m_no_attributes; i++ )
	if ( ! ::wcscmp( name, p_attribs[ i ].first ) )
		return p_attribs[ i ].second;

	// Return the default
	return p_default;
}

// Get the number of children
const int node2::no_children( void ) const
{	return m_no_children;
}

// Get the child
node2 &node2::child( const int idx )
{	// Debugging
	assert( idx < m_no_children );
	
	// Get the node pointers
	node2** p_nodes = (node2**)( m_no_attributes + (attribute_type*)( sizeof(node2) + (BYTE*)this ) );

	// Return the node
	return *( p_nodes[ idx ] );
}

const node2 &node2::child( const int idx ) const
{	// Debugging
	assert( idx < m_no_children );
	
	// Get the node pointers
	node2** p_nodes = (node2**)( m_no_attributes + (attribute_type*)( sizeof(node2) + (BYTE*)this ) );

	// Return the node
	return *( p_nodes[ idx ] );
}

// Get the first child of a given name.
// NULL if not found
node2* node2::get_child( const wchar_t name[] )
{	// Get the node pointers
	node2** p_nodes = (node2**)( m_no_attributes + (attribute_type*)( sizeof(node2) + (BYTE*)this ) );

	// Cycle across nodes
	for( int i=0; i<m_no_children; i++ )
	if ( ! ::wcscmp( name, p_nodes[ i ]->m_p_name ) )
		return p_nodes[ i ];

	// Not found
	return NULL;
}

const node2* node2::get_child( const wchar_t name[] ) const
{	// Get the node pointers
	node2** p_nodes = (node2**)( m_no_attributes + (attribute_type*)( sizeof(node2) + (BYTE*)this ) );

	// Cycle across nodes
	for( int i=0; i<m_no_children; i++ )
	if ( ! ::wcscmp( name, p_nodes[ i ]->m_p_name ) )
		return p_nodes[ i ];

	// Not found
	return NULL;
}

// Setup the data
node2* node2::setup( const void* p_data )
{	// Rapid XML
	using namespace rapidxml;
	xml_node<wchar_t> *p_node = (xml_node<wchar_t>*)p_data;

	// This is the base pointer
	BYTE* p_base = sizeof( node2 ) + (BYTE*)this;

	// Get the name
	m_p_name = p_node->name();
	m_p_text = NULL;

	// Compute the number of attributes
	attribute_type* p_attribs = (attribute_type*)p_base;
	for( xml_attribute<wchar_t>* p_attrib = p_node->first_attribute(); p_attrib; p_attrib = p_attrib->next_attribute(), p_attribs++ )
	{	// Store the attributes
		p_attribs->first  = p_attrib->name();
		p_attribs->second = p_attrib->value();
	}

	// Move past the attributes
	m_no_attributes = (int)( p_attribs - (attribute_type*)p_base );
	p_base += sizeof( attribute_type )*m_no_attributes;

	// Count the number of children
	m_no_children = 0;
	for( xml_node<wchar_t>* p_child = p_node->first_node(); p_child; p_child = p_child->next_sibling() )
	if ( p_child->type() == node_element ) m_no_children++;

	// This is the pointer to the next node
	node2**	p_children = (node2**)p_base;
	p_base += sizeof( node2* )*m_no_children;

	// Get the child pointer location	
	for( xml_node<wchar_t>* p_child = p_node->first_node(); p_child; p_child = p_child->next_sibling() )
	{	switch( p_child->type() )
		{	case node_element:
			{	// Setup this item
				node2* p_node2 = (node2*)p_base;
				*(p_children++) = p_node2;
				p_base = (BYTE*)p_node2->setup( p_child );
			} break;

			case node_data:
			{	// We only keep the first text node seen. This is imperfect, but life.
				if ( !m_p_text )
					m_p_text = p_child->value();
			}
		}
	}
	
	// Return the end of this node
	return (node2*)p_base;
}

// Compute the size from a node
const size_t node2::size( const void* p_data )
{	// Rapid XML
	using namespace rapidxml;
	xml_node<wchar_t> *p_node = (xml_node<wchar_t>*)p_data;

	// The return size
	size_t ret = sizeof( node2 );

	// Count the number of attributes
	for( xml_attribute<wchar_t>* p_attrib = p_node->first_attribute(); p_attrib; p_attrib = p_attrib->next_attribute() )
	{	ret += sizeof( attribute_type );
	}

	// We count the number of children
	for( xml_node<wchar_t>* p_child = p_node->first_node(); p_child; p_child = p_child->next_sibling() )
	if ( p_child->type() == node_element ) 
	{	// Handle recursion
		ret += sizeof( node2* );
		ret += size( p_child );
	}

	// Return the results
	return ret;
}

// Generate output
// Get the output length of the XML, *** excluding termination ***
const int node2::output_length( void ) const
{	return output_length( 0 );
}

// Write to output. The return value is the number of characters written, *** including termination ***
void node2::output( wchar_t destination[] ) const
{	output( destination, 0 );
}

// Get the output length of the XML, *** including termination ***
const int node2::output_length( const int depth ) const
{	// The length so far
	int ret = 0;
	
#define _add_str_( a )	{ ret += (int)::wcslen( a ); }
#define _add_chr_( a )	{ ret ++; }

	// <ddr param="">text</ddr>
	_add_chr_( L'<' );							// <
	_add_str_( type() );						// ddr
	
	for( int i=0; i<no_parameters(); i++ )
	{	_add_chr_( L' ' );						//  
		_add_str_( parameter(i).first );		// param
		_add_chr_( L'=' );						// =
		_add_chr_( L'"' );						// "
		_add_str_( parameter(i).second );		// param
		_add_chr_( L'"' );						// "
	}

	_add_chr_( L'>' );							// >

	// Recursion
	for( int i=0; i<no_children(); i++ )		
		ret += child( i ).output_length( depth+1 );

	// Ending
	_add_chr_( L'<' );							// <
	_add_chr_( L'/' );							// /
	_add_str_( type() );						// ddr
	_add_chr_( L'>' );							// >	

	if ( !depth ) _add_chr_( 0 );				// Termination

#undef _add_str_
#undef _add_chr_	

	// Return the length
	return ret;
}

// Write to output. The return value is the number of characters written, *** including termination ***
const int node2::output( wchar_t destination[], const int depth ) const
{
#define _add_str_( a )	{ const int len = (int)::wcslen( a ); ::memcpy( p_destination, a, sizeof(wchar_t)*len ); p_destination += len; }
#define _add_chr_( a )	{ *(p_destination++) = (a); }

	wchar_t *p_destination = destination;

	// <ddr param="">text</ddr>
	_add_chr_( L'<' );							// <
	_add_str_( type() );						// ddr
	
	for( int i=0; i<no_parameters(); i++ )
	{	_add_chr_( L' ' );						//  
		_add_str_( parameter(i).first );		// param
		_add_chr_( L'=' );						// =
		_add_chr_( L'"' );						// "
		_add_str_( parameter(i).second );		// param
		_add_chr_( L'"' );						// "
	}

	_add_chr_( L'>' );							// >

	// Recursion
	for( int i=0; i<no_children(); i++ )		
		p_destination += child( i ).output( p_destination, depth+1 );

	// Ending
	_add_chr_( L'<' );							// <
	_add_chr_( L'/' );							// /
	_add_str_( type() );						// ddr
	_add_chr_( L'>' );							// >	

	if ( !depth ) _add_chr_( 0 );				// Termination

	// Get the real length used
	const int ret = (int)( p_destination - destination );

	//const int desired_length = output_length( depth );
	//assert( ret == desired_length );

#undef _add_str_
#undef _add_chr_	

	// Return the length
	return ret;
}


//*******************************************************************************************************************************
// Constructor
tree2::tree2( const char *p_data, const int size_in_bytes )
	: m_used( false )
{	::memset( &m_empty, 0, sizeof( m_empty ) );
	setup( p_data, size_in_bytes );
}

tree2::tree2( const wchar_t *p_data, const int size_in_bytes )
	: m_used( false )
{	::memset( &m_empty, 0, sizeof( m_empty ) );
	setup( p_data, size_in_bytes );
}

tree2::tree2( void )
	: m_used( false )
{	::memset( &m_empty, 0, sizeof( m_empty ) );
}

// Assign and parse a new string
void tree2::operator= ( const char* p_str )
{	setup( p_str );
}

void tree2::operator= ( const wchar_t* p_str )
{	setup( p_str );
}

tree2::~tree2( void )
{		
}

// Free all memory on this node (and all children)
void tree2::clear( void )
{	m_used = false;
}

// Load from a string
bool tree2::read_from_string( const char text[], const int length )
{	setup( text, length );
	return type() ? true : false;
}

bool tree2::read_from_string( const wchar_t text[], const int length )
{	setup( text, length );
	return type() ? true : false;
}

// Load from a resource
bool tree2::read_from_resource( HANDLE hModule, const DWORD resource_ID, const wchar_t* resource_type )
{	// Not yet set
	const char *p_data = NULL;
	size_t size = 0;
	
	// Look for the resource
	HRSRC rsrc = ::FindResourceW( (HMODULE)hModule, MAKEINTRESOURCE( resource_ID ), resource_type );
    if ( !rsrc ) return false;

	// Load in the resource
	HGLOBAL glbl_rsrc = ::LoadResource( (HMODULE)hModule, rsrc );
	if ( !glbl_rsrc ) return false;

	// Get the resource pointer
	p_data = (char*)::LockResource( glbl_rsrc );
	if ( !p_data ) return false;

	size = ::SizeofResource( (HMODULE)hModule, rsrc );
	if ( !size ) return false;

	// Hack to NULL terminate the string
	char* p_tmp = new char [ size + 1 ];
	::memcpy( p_tmp, p_data, size );
	p_tmp[ size ] = 0;	

	// Try to read the mesh file.
	setup( p_tmp, (int)size );

	// Free
	delete [] p_tmp;

	// Success
	return type() ? true : false;
}

// e.g.		<DDR> </DDR>
// The node type is DDR
const wchar_t *tree2::type( void ) const
{	return ( m_used ? ( (node2*)&m_nodes[ 0 ] ) : &m_empty ) -> type();
}

const wchar_t *tree2::text( void ) const
{	return ( m_used ? ( (node2*)&m_nodes[ 0 ] ) : &m_empty ) -> text();
}

// Get access to the parameters
// e.g.		<DDR value="1" another_value="2">
// There are two parameters. "value","1"   "another_value","2"
const int tree2::no_parameters( void ) const
{	return ( m_used ? ( (node2*)&m_nodes[ 0 ] ) : &m_empty ) -> no_parameters();
}

// If you access a parameter past the end, NULl, NULL is returned.
const std::pair< const wchar_t*, const wchar_t* > tree2::parameter( const int idx ) const
{	return ( m_used ? ( (node2*)&m_nodes[ 0 ] ) : &m_empty ) -> parameter( idx );
}

// Get parameter by name, NULL if not found.
const wchar_t* tree2::parameter( const wchar_t name[], const wchar_t *p_default ) const
{	return ( m_used ? ( (node2*)&m_nodes[ 0 ] ) : &m_empty ) -> parameter( name, p_default );
}

// Get the number of children
const int tree2::no_children( void ) const
{	return ( m_used ? ( (node2*)&m_nodes[ 0 ] ) : &m_empty ) -> no_children();
}

// Get the child
node2 &tree2::child( const int idx )
{	return ( m_used ? ( (node2*)&m_nodes[ 0 ] ) : &m_empty ) -> child( idx );
}

const node2 &tree2::child( const int idx ) const
{	return ( m_used ? ( (node2*)&m_nodes[ 0 ] ) : &m_empty ) -> child( idx );
}

// Get the first child of a given name.
// NULL if not found
node2* tree2::get_child( const wchar_t name[] )
{	return ( m_used ? ( (node2*)&m_nodes[ 0 ] ) : &m_empty ) -> get_child( name );
}

const node2* tree2::get_child( const wchar_t name[] ) const
{	return ( m_used ? ( (node2*)&m_nodes[ 0 ] ) : &m_empty ) -> get_child( name );
}

// Get this item as a node, which is useful for passing to other functions
tree2::operator		  node2& ( void )
{	return *( m_used ? ( (node2*)&m_nodes[ 0 ] ) : &m_empty );
}

tree2::operator const node2& ( void ) const
{	return *( m_used ? ( (node2*)&m_nodes[ 0 ] ) : &m_empty );
}

// Setup 
void tree2::setup( const void* p_data, const int size_in_bytes )
{	// Rapid XML
	using namespace rapidxml;

	// Not used now
	m_used = false;

	// Get the perser
	xml_document<wchar_t> parser;

	// We need to guess whether the string is unicode
	const wchar_t*	p_dataW = (const wchar_t*)p_data;
	const char*		p_dataA = (const char*)p_data;

	// Note that a document cannot be a single character long
	if ( !p_dataA[ 0 ] ) 
		return;

	if ( !p_dataA[ 1 ] )
	{	// Probably unicode
		if ( size_in_bytes > 0 ) assert( (size_in_bytes&1) == 0 );
		const size_t length = 1 + ( ( size_in_bytes > 0 ) ? ( size_in_bytes / 2 ) : ::wcslen( p_dataW ) );

		// Expand the string pool if needed
		if ( length > m_str.size() ) 
			m_str.resize( length );

		// Copy as fast as possible
		::memcpy( &m_str[ 0 ], p_dataW, length*sizeof(wchar_t) );

		// Terminate the string
		m_str[ length-1 ] = 0;
	}
	else
	{	// Probably ASCII
        // UTF-8?
        if ( ((unsigned char)p_dataA[ 0 ]) == 0xEF && ((unsigned char)p_dataA[ 1 ]) == 0xBB && ((unsigned char)p_dataA[ 2 ]) == 0xBF )
        {	// Skup utf-8 bom
			p_dataA += 3;      
			const size_t length = 1 + ( ( size_in_bytes > 0 ) ? size_in_bytes : ::strlen( p_dataA ) );

			// Expand the string pool if needed
			if ( length > m_str.size() ) m_str.resize( length );

			// Convert the string
			::MultiByteToWideChar( CP_UTF8, 0, p_dataA, -1, &m_str[ 0 ], (int)length );
        }
		else
		{	// Get the length
			const size_t length = 1 + ( ( size_in_bytes > 0 ) ? size_in_bytes : ::mbstowcs( NULL, p_dataA, 0 ) );
		
			// Expand the string pool if needed
			if ( length > m_str.size() ) m_str.resize( length );

			// Convert the string
			::mbstowcs( &m_str[ 0 ], p_dataA, length );

			// Terminate the string
			m_str[ length-1 ] = 0;
		}
	}

	// Parse the object
	try
	{	// Parse it
	parser.parse<0>( &m_str[ 0 ] );

	// We now get the total data size
	const size_t data_size = node2::size( parser.first_node() );

	// Allocate the compressed node structure
	if ( data_size > m_nodes.size() )
		m_nodes.resize( data_size );

	// Build the node tree
	node2* p_node = (node2*)&m_nodes[ 0 ];
	BYTE* p_debug = (BYTE*)p_node->setup( parser.first_node() );
	assert( ( p_debug - (BYTE*)p_node ) == data_size );

	// Store whether a type was found
	m_used = p_node->type() ? true : false;
	}
	catch( ... )
	{
	}

	
}

// Get the output length of the XML, *** excluding termination ***
const int tree2::output_length( void ) const
{	const node2&	this_node = (*this);
	return this_node.output_length( 0 );
}

// Write to output. The return value is the number of characters written, *** including termination ***
void tree2::output( wchar_t destination[] ) const
{	const node2&	this_node = (*this);
	this_node.output( destination, 0 );
}