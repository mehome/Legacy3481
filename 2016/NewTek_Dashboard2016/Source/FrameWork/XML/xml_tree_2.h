#pragma once

struct tree2;

struct FRAMEWORKXML_API node2
{			// e.g.		<DDR> </DDR>
			// The node type is DDR
			const wchar_t *type( void ) const;

			// The text on this node
			const wchar_t *text( void ) const;

			// Get access to the parameters
			// e.g.		<DDR value="1" another_value="2">
			// There are two parameters. "value","1"   "another_value","2"
			const int no_parameters( void ) const;

			// If you access a parameter past the end, NULl, NULL is returned.
			const std::pair< const wchar_t*, const wchar_t* > parameter( const int idx ) const;

			// Get parameter by name, NULL if not found.
			const wchar_t* const parameter( const wchar_t name[], wchar_t* const default_value = 0 ) const;

			// Get parameters by type using template specialization.
			template<class T> T parameter( const wchar_t name[], const T default_value = 0 ) const;

			// Get the number of children
			const int no_children( void ) const;

			// Get the child
				  node2 &child( const int idx );
			const node2 &child( const int idx ) const;

			// Get the first child of a given name.
			// NULL if not found
				  node2* get_child( const wchar_t name[] );
			const node2* get_child( const wchar_t name[] ) const;

			// Generate output
			// Get the output length of the XML, *** including termination ***
			const int output_length( void ) const;

			// Write to output. The return value is the number of characters written, *** including termination ***
			void output( wchar_t destination[] ) const; 

private:	// Convert a string to a boolean value.
			static bool convert_to_bool( wchar_t const * const value );
			
			// The sizes
			int	m_no_children;
			int m_no_attributes;

			// The name of this element
			wchar_t* m_p_name;

			// This is text field
			wchar_t* m_p_text;

			// Attribute descriptor
			typedef std::pair<const wchar_t*,const wchar_t*> attribute_type;

			// Compute the size from a node
			static const size_t size( const void* p_data );

			// Setup the data
			node2* setup( const void* p_data );

			// Get the output length of the XML, *** excluding termination ***
			const int output_length( const int depth ) const;

			// Write to output. The return value is the number of characters written, *** including termination ***
			const int output( wchar_t destination[], const int depth ) const; 

			// A friend
			friend tree2;
};

// Default node2 conversion to an unknown type.
template<class T> __inline T node2::parameter( const wchar_t name[], const T default_value ) const
{	const wchar_t* const p_param = parameter( name );
	return p_param ? T( p_param ) : default_value;
}

// Specialized node2 conversion to wchar_t*.
template<> __inline const wchar_t* node2::parameter( const wchar_t name[], const wchar_t* default_value ) const
{	const wchar_t* const p_param = parameter( name );
	return p_param ? p_param : default_value;
}

// Specialized node2 conversion to int.
template<> __inline int node2::parameter( const wchar_t name[], const int default_value ) const
{	const wchar_t* const p_param = parameter( name );
	return p_param ? ::_wtoi( p_param ) : default_value;
}

// Specialized node2 conversion to int64.
template<> __inline __int64 node2::parameter( const wchar_t name[], const __int64 default_value ) const
{	const wchar_t* const p_param = parameter( name );
	return p_param ? ::_wtoi64( p_param ) : default_value;
}

// Specialized node2 conversion to unsigned int.
template<> __inline unsigned int node2::parameter( const wchar_t name[], const unsigned int default_value ) const
{	const wchar_t* const p_param = parameter( name );
	unsigned int ret = ( p_param != 0 && ::swscanf( p_param, L"%u", &ret ) != 0 ) ? ret : default_value;
	return ret;
}

// Specialized node2 conversion to bool.
template<> __inline bool node2::parameter( const wchar_t name[], const bool default_value ) const
{	const wchar_t* const p_param = parameter( name );
	return p_param ? convert_to_bool( p_param ) : default_value;
}

// Specialized node2 conversion to float.
template<> __inline float node2::parameter( const wchar_t name[], const float default_value ) const
{	const wchar_t* const p_param = parameter( name );
	return p_param ? static_cast<float>( ::_wtof( p_param ) ) : default_value;
}

// Specialized node2 conversion to double.
template<> __inline double node2::parameter( const wchar_t name[], const double default_value ) const
{	const wchar_t* const p_param = parameter( name );
	return p_param ? ::_wtof( p_param ) : default_value;
}

struct FRAMEWORKXML_API tree2
{			// Constructor
			tree2( void );
			tree2( const char* p_str, const int size_in_bytes = -1 );
			tree2( const wchar_t* p_str, const int size_in_bytes = -1 );

			// Destructor
			~tree2( void );

			// Free all memory on this node (and all children)
			void clear( void );

			// Load from a file
			bool read_from_file( const wchar_t filename[] );

			// Load from a string
			bool read_from_string( const char	 text[], const int size_in_bytes = -1 );
			bool read_from_string( const wchar_t text[], const int size_in_bytes = -1 );

			// Load from a resource
			bool read_from_resource( HANDLE hModule, const DWORD resource_ID, const wchar_t* resource_type );

			// Assign and parse a new string
			void operator= ( const char* p_str );
			void operator= ( const wchar_t* p_str );

			// e.g.		<DDR> </DDR>
			// The node type is DDR
			const wchar_t *type( void ) const;

			// Get the text entry on this node
			const wchar_t *text( void ) const;

			// Get access to the parameters
			// e.g.		<DDR value="1" another_value="2">
			// There are two parameters. "value","1"   "another_value","2"
			const int no_parameters( void ) const;

			// If you access a parameter past the end, NULl, NULL is returned.
			const std::pair< const wchar_t*, const wchar_t* > parameter( const int idx ) const;

			// Get parameter by name, NULL if not found.
			const wchar_t* parameter( const wchar_t name[], const wchar_t *p_default = NULL ) const;

			// Get the number of children
			const int no_children( void ) const;

			// Get the child
				  node2 &child( const int idx );
			const node2 &child( const int idx ) const;

			// Get the first child of a given name.
			// NULL if not found
				  node2* get_child( const wchar_t name[] );
			const node2* get_child( const wchar_t name[] ) const;

			// Get this item as a node, which is useful for passing to other functions
			operator	   node2& ( void );
			operator const node2& ( void ) const;

			// Generate output
			// Get the output length of the XML, *** including termination ***
			const int output_length( void ) const;

			// Write to output. The return value is the number of characters written, *** including termination ***
			void output( wchar_t destination[] ) const; 

private:	// Setup 
			void setup( const void* p_data, const int size_in_bytes = -1 );

			// Is it used
			bool	m_used;

			// This is an empty node
			node2	m_empty;
	
			// The string to use
			std::vector< wchar_t >	m_str;
			std::vector< BYTE >		m_nodes;
};


