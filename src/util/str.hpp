#pragma once

#include <string>
#include <fmt/base.h>

#ifdef _WIN32
#include <fmt/xchar.h>
using cstr = wchar_t *;
using const_cstr = wchar_t const *;
using str = std::wstring;

#define CATENATE_DETAIL(x, y) x##y
#define CATENATE(x, y) CATENATE_DETAIL(x, y)
#define STR(str) CATENATE(L, str)

#define STRCMP wcscmp
#else
using cstr = char *;
using const_cstr = char const *;
using str = std::string;
#define STR(str) str
#define STRCMP strcmp
#endif