#include "StdAfx.h"
#include "FrameWork.XML.h"

using namespace FrameWork::xml;

// Constructor
parser::parser( node *p_parent_node ) : m_p_parser( NULL ), m_error( false ), m_xml_op( p_parent_node )
{	// Create an expat parser
	XML_Parser xml_parser = XML_ParserCreate( NULL );
	m_p_parser = (void*)xml_parser;

	// Set it up with the data
	if ( xml_parser )
	{	// Setup the data
		XML_SetElementHandler( xml_parser,(XML_StartElementHandler) xml_creator::StartElementHandler,(XML_EndElementHandler) xml_creator::EndElementHandler );
		XML_SetCharacterDataHandler( xml_parser,(XML_CharacterDataHandler) xml_creator::CharacterDataHandler );
		XML_SetUserData( xml_parser, (void*)&m_xml_op );
	}
	else 
	{	// Clearly there was an error
		m_error = true;
	}
}

// Has there been any kind of error
bool parser::error( void ) const
{	return m_error;
}

// Destructor
parser::~parser( void )
{	// Nuke the parser
	if ( m_p_parser )
	{	// Set the parser
		XML_Parser xml_parser = (XML_Parser)m_p_parser;

		// Flush the XML parser
		XML_Parse( xml_parser, NULL, 0, true );

		// Close the parser
		XML_ParserFree( xml_parser );
	}
}
	
		// Push characters into the parser
bool parser::operator() ( const char* p_data, const size_t data_size )
{
	// Nuke the parser
	if ( !m_p_parser ) return false;

	// Set the parser
	XML_Parser xml_parser = (XML_Parser)m_p_parser;

	// Process it
	if ( XML_STATUS_ERROR == XML_Parse( xml_parser, p_data, (int)data_size, false ) )
	{	// There was an error !
		m_error = true;
	}

	// Success / error
	return error();
}

bool parser::operator() ( const wchar_t* p_data, const size_t data_size )
{
	return (*this)( (char*)p_data, data_size*2 );
}

// Constructor
parser::xml_creator::xml_creator( node* p_parent_node )
{	// Push this value
	push( p_parent_node );
}

// Implementations
void parser::xml_creator::StartElementHandler( void *userData, const wchar_t *name, const wchar_t **atts )
{	// Get the instance
	xml_creator *p_this = (xml_creator*)userData;
	
	// Get the current node
	node*	p_node = p_this->top();

	// Get the number of atts
	int no_atts = 0;
	while( atts[no_atts*2] ) no_atts++;

	// We now simulate the classes
	node::parameter	*p_params = (node::parameter*)atts;

	// Get a new child node
	node *p_new_node = p_node ? p_node->p_create_child( name, no_atts, p_params ) : NULL;

	// Call the creator
	if (p_new_node) p_new_node->start( name, no_atts, p_params );

	// This is now the active node
	p_this->push( p_new_node );
}

void parser::xml_creator::EndElementHandler( void *userData, const wchar_t *name)
{	// Get the context
	xml_creator *p_this = (xml_creator*)userData;

	// Get the current active node
	node*	p_node = p_this->top();

	// Finish creating the node
	if (p_node) p_node->end( name );

	// Now pop the previous node
	p_this->pop();
	assert( p_this->size() );

	// Get the parent node
	node*	p_node_parent = p_this->top();

	// Finish node creation
	if (p_node_parent) p_node_parent->create_child_end( name, p_node );
}

void parser::xml_creator::CharacterDataHandler( void *userData, const wchar_t *s, int len )
{	// Get the instance
	xml_creator *p_this = (xml_creator*)userData;
	
	// Get the current active node
	node*	p_node = p_this->top();

	// Add the text
	if (p_node) p_node->add_text( s, len );
}