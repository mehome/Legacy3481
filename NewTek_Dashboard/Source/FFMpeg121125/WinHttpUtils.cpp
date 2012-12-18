#include "stdafx.h"

static void init_url_components(URL_COMPONENTS& components)
{
	memset(&components, 0, sizeof(components));

	components.dwStructSize      = sizeof(components);
	components.dwHostNameLength  = static_cast<DWORD>(-1);
	components.dwUrlPathLength   = static_cast<DWORD>(-1);
	components.dwExtraInfoLength = static_cast<DWORD>(-1);
	components.dwUserNameLength  = static_cast<DWORD>(-1);
	components.dwPasswordLength  = static_cast<DWORD>(-1);
}

bool crack_url(const std::wstring url, std::wstring& hostname, std::wstring& resource, int& port, std::wstring* p_username, std::wstring* p_password)
{
	URL_COMPONENTS components;
	init_url_components(components);

	if (WinHttpCrackUrl(url.c_str(), 0, 0, &components) == FALSE)
		return false;

	hostname.assign(components.lpszHostName,  components.dwHostNameLength);
	resource.assign(components.lpszUrlPath,   components.dwUrlPathLength);
	resource.append(components.lpszExtraInfo, components.dwExtraInfoLength);

	if (resource.length() == 0)
		resource = L"/";

	if (p_username)
		p_username->assign(components.lpszUserName, components.dwUserNameLength);

	if (p_password)
		p_password->assign(components.lpszPassword, components.dwPasswordLength);

	port = components.nPort;
	return true;
}

bool get_http_header(HINTERNET request, const DWORD header_id, std::wstring& header)
{
	DWORD buffer_length;
	if (WinHttpQueryHeaders(request, header_id, WINHTTP_HEADER_NAME_BY_INDEX, WINHTTP_NO_OUTPUT_BUFFER, &buffer_length, WINHTTP_NO_HEADER_INDEX) != FALSE)
		return false;

	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		return false;

	std::vector<BYTE> buffer(buffer_length);
	if (WinHttpQueryHeaders(request, header_id, WINHTTP_HEADER_NAME_BY_INDEX, &buffer[0], &buffer_length, WINHTTP_NO_HEADER_INDEX) == FALSE)
		return false;

	header = reinterpret_cast<wchar_t*>(&buffer[0]);
	return true;
}

bool get_http_header(HINTERNET request, const DWORD header_id, DWORD& header)
{
	DWORD buffer_length = sizeof(DWORD);
	return WinHttpQueryHeaders(request, header_id | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &header, &buffer_length, WINHTTP_NO_HEADER_INDEX) != FALSE;
}

bool get_http_header(HINTERNET request, const DWORD header_id, SYSTEMTIME& header)
{
	DWORD buffer_length = sizeof(SYSTEMTIME);
	return WinHttpQueryHeaders(request, header_id | WINHTTP_QUERY_FLAG_SYSTEMTIME, WINHTTP_HEADER_NAME_BY_INDEX, &header, &buffer_length, WINHTTP_NO_HEADER_INDEX) != FALSE;
}
