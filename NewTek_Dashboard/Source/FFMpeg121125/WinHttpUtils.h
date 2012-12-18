#pragma once

bool crack_url(const std::wstring url, std::wstring& hostname, std::wstring& resource, int& port, std::wstring* p_username=NULL, std::wstring* p_password=NULL);

bool get_http_header(HINTERNET request, const DWORD header_id, std::wstring& header);
bool get_http_header(HINTERNET request, const DWORD header_id, DWORD& header);
bool get_http_header(HINTERNET request, const DWORD header_id, SYSTEMTIME& header);
