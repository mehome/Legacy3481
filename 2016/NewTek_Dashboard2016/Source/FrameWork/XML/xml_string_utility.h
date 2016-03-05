#pragma once

//for ratio strings of the format "x:y" with positive nonzero values, returns -1:1 if the string is invalid in any way
std::pair<int,int> FRAMEWORKXML_API get_ratio( wchar_t const *p_str );
