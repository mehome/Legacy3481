#include "StdAfx.h"
#include "FrameWork.XML.h"

using namespace FrameWork::xml;

bool FrameWork::xml::load_from_string( const std::string	&xml_data, node base_node[] )
{	// Create a parser
	parser	local_parser( base_node );
	
	// Add it to the parser
	const bool xml_error = local_parser( xml_data.c_str(), xml_data.size() );

	// Return
	return !local_parser.error();
}

bool FrameWork::xml::load_from_string( const std::wstring	&xml_data, node base_node[] )
{	// Create a parser
	parser	local_parser( base_node );
	
	// Add it to the parser
	const bool xml_error = local_parser( xml_data.c_str(), xml_data.size() );

	// Return
	return !local_parser.error();
}


bool FrameWork::xml::load_from_string( const char xml_data[], node base_node[], const int size )
{	// Create a parser
	parser	local_parser( base_node );
	
	// Add it to the parser
	const bool xml_error = local_parser( xml_data, ( size != -1 ) ? size : ::strlen( xml_data ) );

	// Return
	return !local_parser.error();
}

bool FrameWork::xml::load_from_string( const wchar_t xml_data[], node base_node[], const int size )
{	// Create a parser
	parser	local_parser( base_node );
	
	// Add it to the parser
	const bool xml_error = local_parser( xml_data, ( size != -1 ) ? size : ::wcslen( xml_data ) );

	// Return
	return !local_parser.error();
}



//
//  scan a text string looking for the 5 chars that need to be escaped in XML
//
//	if none found,   return NULL      
//	otherwise,		 return pointer to allocated string with escape sequences inserted
//
//
//		(WARNING  assumes text does not already contain escape sequences !) 
//
wchar_t *	FrameWork::xml::escape_text_for_XML (const wchar_t *ptext)
{
 int		k, ln, lnseq;
 wchar_t	*srcP, *dstP, *wstr;
 
	// scan character by character, looking for   &<>'"   in source text.
	// if found, tally additional space needed for escape sequences
	k = ln = lnseq = 0;
	srcP = const_cast<wchar_t *>  (ptext);
	
	while (*srcP != 0)
	{
		if (*srcP == L'&')			lnseq += 4;
		if (*srcP == L'\"')			lnseq += 5;
		if (*srcP == L'\'')			lnseq += 5;
		if (*srcP == L'<')			lnseq += 3;
		if (*srcP == L'>')			lnseq += 3;

		ln++;
		srcP++;
	}


	// if no escapes needed in source string, return NULL
	if (lnseq == 0)			return (NULL);

		
	// need space for a new string that will contain the escape sequences	
	wstr = new (std::nothrow) wchar_t [ln + lnseq + 2];
	if (wstr == NULL)		return (NULL);

	
	// replace any of the 5 special chars with their escape sequences in the copy
	srcP = const_cast<wchar_t *> (ptext);
	dstP = wstr;
	
	while (*srcP != 0)
	{
		if (*srcP == L'&')	
		{
			*dstP = L'&';	dstP++;
			*dstP = L'a';	dstP++;
			*dstP = L'm';	dstP++;
			*dstP = L'p';	dstP++;
			*dstP = L';';	dstP++;
			goto nxt02;
		}
		
		if (*srcP == L'\"')	
		{
			*dstP = L'&';	dstP++;
			*dstP = L'q';	dstP++;
			*dstP = L'u';	dstP++;
			*dstP = L'o';	dstP++;
			*dstP = L't';	dstP++;
			*dstP = L';';	dstP++;
			goto nxt02;
		}
		
		if (*srcP == L'\'')	
		{
			*dstP = L'&';	dstP++;
			*dstP = L'a';	dstP++;
			*dstP = L'p';	dstP++;
			*dstP = L'o';	dstP++;
			*dstP = L's';	dstP++;
			*dstP = L';';	dstP++;
			goto nxt02;
		}
		
		if (*srcP == L'<')	
		{
			*dstP = L'&';	dstP++;
			*dstP = L'l';	dstP++;
			*dstP = L't';	dstP++;
			*dstP = L';';	dstP++;
			goto nxt02;
		}
		
		if (*srcP == L'>')	
		{
			*dstP = L'&';	dstP++;
			*dstP = L'g';	dstP++;
			*dstP = L't';	dstP++;
			*dstP = L';';	dstP++;
			goto nxt02;
		}
		
		// otherwise, just copy char
		*dstP = *srcP;
		dstP++;
		
		
	 nxt02:
		srcP++;								// point to next char in input
	}
	
	*dstP = 0;								// terminate output string
			
	return (wstr);
}

void FrameWork::xml::escape_text_for_XML_free( const wchar_t* p_esc_text )
{	delete [] p_esc_text;
}