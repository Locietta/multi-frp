#include "signal_handler.h"

#include <csignal>
#include <fmt/base.h>
#include "process/process_manager.h"

extern ProcessManager process_manager;

void signal_handler(int signal) {
    const char *signal_str;
    switch (signal) {
        case SIGINT: signal_str = "SIGINT"; break;
        case SIGTERM: signal_str = "SIGTERM"; break;
        case SIGABRT: signal_str = "SIGABRT"; break;
#ifdef _WIN32
        case SIGBREAK: signal_str = "SIGBREAK"; break;
#endif
        default: signal_str = "UNKNOWN SIGNAL"; break;
    }

    fmt::println("Received signal: {}", signal_str);
    process_manager.terminate_all();
    exit(signal);
}

#ifdef _WIN32
BOOL WINAPI console_ctrl_handler(DWORD ctrl_type) {
    const char *signal_str;
    switch (ctrl_type) {
        case CTRL_C_EVENT: signal_str = "CTRL_C_EVENT"; break;
        case CTRL_BREAK_EVENT: signal_str = "CTRL_BREAK_EVENT"; break;
        case CTRL_CLOSE_EVENT: signal_str = "CTRL_CLOSE_EVENT"; break;
        case CTRL_LOGOFF_EVENT: signal_str = "CTRL_LOGOFF_EVENT"; break;
        case CTRL_SHUTDOWN_EVENT: signal_str = "CTRL_SHUTDOWN_EVENT"; break;
        default: signal_str = "UNKNOWN_CTRL_EVENT"; break;
    }

    fmt::println("Received Windows control event: {}", signal_str);
    process_manager.terminate_all();

    // Return TRUE to indicate we handled the event
    // This gives us time to clean up before the process is forcibly terminated
    return TRUE;
}
#endif

void register_signal_handlers() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGABRT, signal_handler);

#ifdef _WIN32
    if (!SetConsoleCtrlHandler(console_ctrl_handler, TRUE)) {
        fmt::println("Warning: Failed to set console control handler!");
    }
#endif
}