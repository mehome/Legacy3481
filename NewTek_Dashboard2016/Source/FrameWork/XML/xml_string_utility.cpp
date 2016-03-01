#include "stdafx.h"
#include "FrameWork.XML.h"

namespace FrameWork
{
	namespace xml
	{
		namespace utilities
		{

//for ratio strings of the format "x:y" with positive nonzero values, returns -1:1 if the string is invalid in any way
std::pair<int,int> FRAMEWORKXML_API get_ratio( wchar_t const *l_p_wsz_original_ratio )
{
	// (cull out garbage....)
	//if the ptr is null or the string is < 3 chars ("1:1"), just return default 16:9
	if( !l_p_wsz_original_ratio || ::wcslen( l_p_wsz_original_ratio ) < 3 )
		return std::pair< int, int >( -1, 1 );

	//the string and ptrs into it
	wchar_t l_wsz_ratio[MAX_PATH];
	wchar_t *l_p_wsz_width = &l_wsz_ratio[0];
	wchar_t *l_p_wsz_height = NULL;
				
	//int of aspect width and height, for wtoi
	int l_width = 0;
	int l_height = 0;
			
	//cpy the string since we will be modifying it
	::wcscpy_s( l_wsz_ratio, MAX_PATH, l_p_wsz_original_ratio );
				
	//token parse for ":" char, it will be replaced, set the ptr for the second value string
	::wcstok_s( l_wsz_ratio, L":", &l_p_wsz_height );

	//convert strings into value s
	l_width	= ::_wtoi( l_p_wsz_width );
	l_height= ::_wtoi( l_p_wsz_height );
	
	// (cull out garbage....)
	//if they were both valid (not zero and positive), return the pair
	if( l_width > 0 && l_height > 0 )
		return std::pair< int, int > ( l_width, l_height );
	else
	//if one or both of them were zero or negative
		return std::pair< int, int >( -1, 1 );

}

		}
	}
}