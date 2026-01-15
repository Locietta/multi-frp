#pragma once

#include <string>

#ifdef _WIN32
using native_cstr = wchar_t *;
using native_str = std::wstring;
#define STR(str) L##str
#define STRCMP wcscmp
#else
using native_cstr = char *;
using native_str = std::string;
#define STR(str) str
#define STRCMP strcmp
#endif