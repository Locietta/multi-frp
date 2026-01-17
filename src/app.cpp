#include "app.h"
#include <array>
#include <filesystem>
#include <string>

#include "util/print.hpp"

#include "cli_parser.h"
#include "config.hpp"

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

#define arg_end ((const char *) (nullptr))

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

} // namespace

#define arg_end

#endif

int App::run(int argc, char *argv[]) {
#ifndef _WIN32
    const auto signals = make_sigset({SIGINT, SIGTERM, SIGABRT, SIGSEGV});
    pthread_sigmask(SIG_BLOCK, &signals, nullptr);
#else
    if (!SetConsoleCtrlHandler(console_ctrl_handler, TRUE)) {
        print("Error: Could not set control handler\n");
        return 1;
    }
#endif

    CliParser parser;
    const auto parse_result = parser.parse(argc, argv);
    if (parse_result == ParseResult::GRACEFUL_EXIT) {
        return 0;
    } else if (parse_result == ParseResult::ERR) {
        return 1;
    }

    const auto &config_file_path = parser.config_file_path;
    const auto config_file_path_str = config_file_path.string();

    if (!std::filesystem::exists(config_file_path)) {
        print("Config file does not exist: ", config_file_path_str, "\n");
        return 1;
    }

    // Read the JSON configuration file

    std::string file_content;
    {
        const auto file_deleter = [](FILE *file) static { if (file) {fclose(file);} };
        std::unique_ptr<FILE, decltype(file_deleter)> file_ptr(fopen(config_file_path_str.c_str(), "r"), file_deleter);
        if (!file_ptr) {
            print("Failed to open config file: ", config_file_path_str, "\n");
            return 1;
        }

        const auto file_size = std::filesystem::file_size(config_file_path);
        file_content.resize(file_size);

        fread(file_content.data(), 1, file_content.size(), file_ptr.get());
    }

    /// Parse JSON
    Config config;
    try {
        config = daw::json::from_json<Config>(file_content);
    } catch (const daw::json::json_exception &e) {
        print("Error parsing config file: ", e.what(), "\n");
        return 1;
    } catch (const std::exception &e) {
        print("Error parsing config file: ", e.what(), "\n");
        return 1;
    } catch (...) {
        print("Unknown error parsing config file.\n");
        return 1;
    }

    // Check if all config files exist
    for (const auto &config_file : config.configs) {
        const auto config_path = std::filesystem::path(config_file);
        if (!std::filesystem::exists(config_path)) {
            print("Config file does not exist: ", config_file, "\n");
            return 1;
        }
    }

    // Print the frpc binary path and config files
    print("frpc binary: ", config.frpc, "\n");
    print("Config files:\n");
    for (const auto &config_file : config.configs) {
        print(" - ", config_file, "\n");
    }

    // Execute multiple frpc all at background
    for (const auto &config_file : config.configs) {
        const auto args = std::array{config.frpc.c_str(), "-c", config_file.c_str(), arg_end};
        if (!process_manager_.add_process(args)) {
            print("Failed to start frpc with config: ", config_file, "\n");
            return 1;
        }
        print("Started frpc with config: ", config_file, "\n");
    }

#ifndef _WIN32
    int last_signal;
    sigwait(&signals, &last_signal);
    print("Received termination signal: ", signal_to_str(last_signal), "\n");
    process_manager_.terminate_all();
#endif

    process_manager_.wait_all();
    print("All frpc instances have been executed.\n");

    return 0;
}