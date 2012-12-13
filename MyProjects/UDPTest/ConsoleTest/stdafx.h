// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

//#include <windows.h>
#include <iostream>
#include <tchar.h>
#include <math.h>
#include <assert.h>
//#include <ObjBase.h> //GUID converters

#include <vector>
#include <stack>
#include <fstream>
#include <utility>
#include <set>
#include <string>
#include <algorithm>
#include <functional>

// TODO: reference additional headers your program requires here
#define wchar2char(wchar2char_pwchar_source) \
	const size_t wchar2char_Length=wcstombs(NULL,wchar2char_pwchar_source,0)+1; \
	char *wchar2char_pchar = (char *)_alloca(wchar2char_Length);; /* ";;" is needed to fix a compiler bug */ \
	wcstombs(wchar2char_pchar,wchar2char_pwchar_source,wchar2char_Length);

#define char2wchar(char2wchar_pchar_source) \
	const size_t char2wchar_Length=((strlen(char2wchar_pchar_source)+1)*sizeof(wchar_t)); \
	wchar_t *char2wchar_pwchar = (wchar_t *)_alloca(char2wchar_Length);; /* ";;" is needed to fix a compiler bug */ \
	mbstowcs(char2wchar_pwchar,char2wchar_pchar_source,char2wchar_Length);

#define _aligned_alloca( size, alignement ) ( (void*)( ((size_t)((alignement)-1)+(size_t)_alloca( (size)+(alignement) ))&(~((alignement)-1)) ) )
