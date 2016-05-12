#include <string>
#include <stdarg.h>
#include "windows.h"

namespace Framework
{
namespace Base
{

void DebugOutput(const char *format, ... )
{
	va_list marker;
	va_start(marker,format);
		static char Temp[2048];
		vsprintf(Temp,format,marker);
		OutputDebugStringA(Temp);
	va_end(marker); 
}

std::string BuildString(const char *format, ... )
{
	char Temp[2048];
	va_list marker;
	va_start(marker,format);
	vsprintf(Temp,format,marker);
	va_end(marker); 
	std::string ret(Temp);
	return ret;
}
//////////////////////////////////////////////////////////////////////////

char* GetLastSlash(char* fn, char* before)
{
	if (!fn) return NULL;
	char* lastSlash = before ? before-1 : fn+strlen(fn);

	while (lastSlash > fn)
	{
		if ((*lastSlash == '/') || (*lastSlash == '\\'))
			return lastSlash;
		--lastSlash;
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////////

//! Returns false iff c == [ 'f', 'F', 'n', 'N', '0', 0 ]
bool ParseBooleanFromChar(char c)
{
	c = toupper(c);
	if ((c == 'F') || (c == 'N') || (c == '0') || (c == 0))
		return false;
	else
		return true;
}
//////////////////////////////////////////////////////////////////////////

void StripCommentsAndTrailingWhiteSpace(char* line)
{
	for (char* eol = line; ; ++eol)
	{
		if ((eol[0] == '\n') || (eol[0] == '\r') || (eol[0] == '#') || (eol[0] == '\0'))
		{
			eol[0] = '\0';
			--eol;
			while ((eol >= line) && ((eol[0]==' ') || (eol[0]=='\t')))
			{
				eol[0] = '\0';
				--eol;
			}
			return;
		}
	}
}

	}
}