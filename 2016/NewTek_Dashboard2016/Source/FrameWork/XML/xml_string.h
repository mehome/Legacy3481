#pragma once

bool FRAMEWORKXML_API load_from_string( const std::string	&xml_data, node base_node[] );
bool FRAMEWORKXML_API load_from_string( const std::wstring	&xml_data, node base_node[] );

bool FRAMEWORKXML_API load_from_string( const char	  xml_data[], node base_node[], const int size = -1 );
bool FRAMEWORKXML_API load_from_string( const wchar_t xml_data[], node base_node[], const int size = -1 );


//
//  scan a text string looking for the 5 chars that need to be escaped in XML
//
//	if none found,   return NULL      
//	otherwise,		 return pointer to allocated string with escape sequences inserted
//
//	
//
FRAMEWORKXML_API	wchar_t *	escape_text_for_XML( const wchar_t *pRawText );
FRAMEWORKXML_API	void		escape_text_for_XML_free( const wchar_t* p_esc_text );
