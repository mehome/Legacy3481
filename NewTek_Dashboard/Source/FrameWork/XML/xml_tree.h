#pragma once

struct FRAMEWORKXML_API tree 
{			
			// Constructor
			tree( void );

			// Parse from a string
			tree( const char	text[], const int size = -1 );
			tree( const wchar_t text[], const int size = -1 );

			// Destructor
			~tree( void );

			// General management
			// ******************
				// Free all memory on this node (and all children)
				void clear( void );

				// Load from a file
				bool read_from_file( const wchar_t filename[] );

				// Load from a string
				bool read_from_string( const char	 text[], const int size = -1 );
				bool read_from_string( const wchar_t text[], const int size = -1 );

				// Load from a resource
				bool read_from_resource( HANDLE hModule, const DWORD resource_ID, const wchar_t* resource_type );

			// Get tree data
			// *************
				// Get access to the node type 
				// e.g.		<DDR> </DDR>
				// The node type is DDR
				const wchar_t *type( void ) const;

				// Get access to the parameters
				// e.g.		<DDR value="1" another_value="2">
				// There are two parameters. "value","1"   "another_value","2"
				const int no_parameters( void ) const;

				// If you access a parameter past the end, NULl, NULL is returned.
				const std::pair< const wchar_t*, const wchar_t* > parameter( const int idx ) const;

				// Get parameter by name, NULL if not found.
				const wchar_t* parameter( const wchar_t name[], const wchar_t *p_default = NULL ) const;

				// Get the text on this node
				// e.g.		<DDR>hello world</DDR>
				// The text is "hello world"
				const wchar_t *text( void ) const;

				// Get the number of children
				const int no_children( void ) const;

				// Get the child
					  tree &child( const int idx );
				const tree &child( const int idx ) const;

				// Get the first child of a given name.
				// NULL if not found
					  tree* get_child( const wchar_t name[] );
				const tree* get_child( const wchar_t name[] ) const;

				// Get the parent (might be NULL if this is the root)
					  tree* parent( void );
				const tree* parent( void ) const;

			// Set tree data
			// *************
				// We allow changes to be tracked in the XML tree so that only items that have not changed are actually
				// put into an XML update when it occurs. 
				void track_changes( const bool start_tracking = true );

				// Set the data type
				void set_type( const wchar_t name[] );

				// Set the text
				void set_text( const wchar_t text[] );
				
				// Remove all parameters
				void remove_all_parameters( void );

				// Add a parameter
				void add_parameter( const wchar_t name[], const wchar_t value[] );

				// Set a parameter				
				void set_parameter( const int idx, const wchar_t name[], const wchar_t value[] );
				
				// Set a parameter if it exists, *** case sensitive ***
				// Adds it if it does not exist
				void set_parameter( const wchar_t name[], const wchar_t value[] );

				// Set a parameter, but first check if any of the characters need to be escaped, and use the escaped string if necessary
				void safe_set_parameter( const wchar_t name[], const wchar_t value[] );

				// Remove all children
				void remove_all_children( void );

				// Add a child
				void add_child( tree* p_new_child );

				// Set a child
				void set_child( const int idx, tree* p_new_child );

			// Execute tree data
			// *****************
				bool execute( node *p_node ) const;

			// Generate output
			// ***************
				// Get the output length of the XML, *** excluding termination ***
				int output_length( void ) const;

				// Write to output. The return value is the number of characters written, *** including termination ***
				void output( wchar_t destination[] ) const; 

private:	// The current implementation is based on xml::node. 
			// This is hidden from the "user" so that the new RapidXML implementation
			// can avoid this dependancy.
			struct internal_node : public node
			{	// Constructor
				internal_node( tree* p_parent );
				
				// Create a child.
				virtual node *p_create_child( const wchar_t type[], const int no_parameters, const parameter *p_parameters );

				// This item is starting to be created
				virtual void start( const wchar_t type[], const int no_parameters, const parameter *p_parameters );

				// The current text
				virtual void add_text( const wchar_t text[], const int no_chars );

				// The parent
				tree* m_p_parent;

			}	m_internal_node;

			// The tree parent node
			tree* m_p_parent;

			// A simple string implementation
			//typedef std::pair< int, wchar_t* > simple_string;
			struct simple_string
			{	int			first;
				wchar_t*	second;
			};

			// Very simple string handling
			static const bool ss_init( simple_string& str );
			static const bool ss_free( simple_string& str );
			static const bool ss_set ( simple_string& str, const wchar_t text[] );
			static const bool ss_add ( simple_string& str, const wchar_t text[] );
			static const bool ss_set ( simple_string& str, const wchar_t text[], const int no_chars );						
			static const bool ss_add ( simple_string& str, const wchar_t text[], const int no_chars, const bool terminate = false );

			// We keep track of whether this item has had any changes made to it.
			mutable bool m_changes_made;

			// This will consolidate the changes made so that if any children changed then the
			// parent is also changed.
			void process_changes( void ) const;

			// The node type
			simple_string m_type;

			// The text at this level
			simple_string m_text;

			// The children
			std::vector< tree* > m_children;

			// The parameters at this level
			std::vector< std::pair< simple_string, simple_string > > m_parameters;

			// Get the output length of the XML, *** including termination ***
			int recurse_output_length( const int depth ) const;

			// Write to output. The return value is the number of characters written, *** including termination ***
			int recurse_output( wchar_t destination[], const int depth ) const; 
};