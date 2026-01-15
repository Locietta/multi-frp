#pragma once
#ifndef MULTI_FRP_VERSION
#define MULTI_FRP_VERSION "dev"
#endif

#include "util/str.hpp"

inline const_cstr version = STR(MULTI_FRP_VERSION);
inline const_cstr program_name = STR("multi-frp");