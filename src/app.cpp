#include "app.h"
#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

#include "cli_parser.h"

#ifndef _WIN32
#include <signal.h>
#include <pthread.h>

namespace {

sigset_t make_sigset(std::initializer_list<int> signals) {
    sigset_t sigset;
    sigemptyset(&sigset);
    for (int sig : signals) {
        sigaddset(&sigset, sig);
    }
    return sigset;
}

const char *signal_to_str(int signal) {
    switch (signal) {
        case SIGINT: return "SIGINT";
        case SIGTERM: return "SIGTERM";
        case SIGABRT: return "SIGABRT";
        case SIGSEGV: return "SIGSEGV";
        default: return "UNKNOWN SIGNAL";
    }
}

#define std_to_str(str) (str)

} // namespace

#else

#include <windows.h>
#include <fcntl.h>

namespace {

BOOL WINAPI console_ctrl_handler(DWORD signal) {
    switch (signal) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            // ignore all these events, let child handle them first
            return TRUE;
        default:
            return FALSE;
    }
}

str std_to_str(std::string_view utf8) {
    if (utf8.empty()) return {};
    const int needed = MultiByteToWideChar(CP_UTF8, 0, utf8.data(),
                                           static_cast<int>(utf8.size()),
                                           nullptr, 0);
    str result(needed, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(),
                        static_cast<int>(utf8.size()),
                        result.data(), needed);
    return result;
}

template <std::ranges::range T>
    requires std::convertible_to<std::ranges::range_value_t<T>, std::string_view>
auto std_to_str(T &&range) {
    std::vector<str> result;
    if constexpr (std::ranges::sized_range<T>) {
        result.reserve(std::ranges::size(range));
    }

    for (const auto &item : range) {
        result.push_back(std_to_str(item));
    }
    return result;
}

} // namespace
#endif

int App::run(int argc, cstr argv[]) {

#ifndef _WIN32
    const auto signals = make_sigset({SIGINT, SIGTERM, SIGABRT, SIGSEGV});
    pthread_sigmask(SIG_BLOCK, &signals, nullptr);
#else
    if (!SetConsoleCtrlHandler(console_ctrl_handler, TRUE)) {
        fmt::println(STR("Error: Could not set control handler"));
        return 1;
    }
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);
#endif

    CliParser parser;
    const auto parse_result = parser.parse(argc, argv);
    if (parse_result == ParseResult::GRACEFUL_EXIT) {
        return 0;
    } else if (parse_result == ParseResult::ERR) {
        return 1;
    }

    const auto config_file_path = parser.config_file_path;
    const auto config_file_path_str = config_file_path.native();

    if (!std::filesystem::exists(config_file_path)) {
        fmt::println(STR("Config file does not exist: {}"), config_file_path_str);
        return 1;
    }

    // Read the JSON configuration file
    nlohmann::json config;
    std::ifstream config_file(config_file_path);
    if (!config_file.is_open()) {
        fmt::println(STR("Failed to open config file: {}"), config_file_path_str);
        return 1;
    }

    try {
        config_file >> config;
    } catch (const std::exception &e) {
        fmt::println(STR("Error reading config file: {}"), std_to_str(e.what()));
        return 1;
    }

    /// Example JSON structure:
    /*
    {
        "frpc": "path/to/frpc",
        "configs": [
            "path/to/config1.toml",
            "path/to/config2.toml"
        ]
    }
    */

    if (!config.contains("frpc") || !config.contains("configs")) {
        fmt::println(STR("Invalid config file format. Expected 'frpc' and 'configs' keys."));
        return 1;
    }

    const auto frpc_path = std_to_str(config["frpc"].get<std::string>());
    const auto config_files = std_to_str(config["configs"].get<std::vector<std::string>>());

    // Check if all config files exist
    for (const auto &config_file : config_files) {
        if (!std::filesystem::exists(config_file)) {
            fmt::println(STR("Config file does not exist: {}"), config_file);
            return 1;
        }
    }

    // Print the frpc binary path and config files
    fmt::println(STR("frpc binary: {}"), frpc_path);
    fmt::println(STR("Config files:"));
    for (const auto &config_file : config_files) {
        fmt::println(STR(" - {}"), config_file);
    }

    // Execute multiple frpc all at background
    for (const auto &config_file : config_files) {
        const auto args = std::array{frpc_path.c_str(), STR("-c"), config_file.c_str()};
        if (!process_manager_.add_process(args)) {
            fmt::println(STR("Failed to start frpc with config: {}"), config_file);
            return 1;
        }
        fmt::println(STR("Started frpc with config: {}"), config_file);
    }

#ifndef _WIN32
    int last_signal;
    sigwait(&signals, &last_signal);
    fmt::println(STR("Received termination signal: {}"), signal_to_str(last_signal));
    process_manager_.terminate_all();
#endif

    process_manager_.wait_all();
    fmt::println(STR("All frpc instances have been executed."));

    return 0;
}