#pragma once

void signal_handler(int signal);

#ifdef _WIN32
#include <windows.h>
BOOL WINAPI console_ctrl_handler(DWORD ctrl_type);
#endif

void register_signal_handlers();