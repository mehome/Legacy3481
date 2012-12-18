#pragma once

struct FRAMEWORKXML_API parser
{			// Constructor
			parser( node *p_parent_node );

			// Destructor
			~parser( void );
	
			// Push characters into the parser
			bool operator() ( const char*	 p_data, const size_t length );
			bool operator() ( const wchar_t* p_data, const size_t length );

			// Has there been any kind of error
			bool error( void ) const;

private:	// The parser object (opaque)
			void *m_p_parser;

			// Has there been an error
			bool m_error;

			// The xml_creation
			struct xml_creator : public std::stack<node*>
			{		// Constructor
					xml_creator( node* p_parent_node );

					// Implementations
					static void StartElementHandler( void *userData, const wchar_t *name, const wchar_t **atts );
					static void EndElementHandler( void *userData, const wchar_t *name );
					static void CharacterDataHandler( void *userData, const wchar_t *s, int len );

			} m_xml_op;
};