#include "app.h"
#include <filesystem>
#include <fstream>

#include <fmt/base.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

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

} // namespace

#endif

int App::run(int argc, char *argv[]) {
    if (argc < 2) {
        fmt::println("Usage: {} <config.json>", argv[0]);
        return 1;
    }

    const std::string config_file_path = argv[1];
    if (!std::filesystem::exists(config_file_path)) {
        fmt::println("Config file does not exist: {}", config_file_path);
        return 1;
    }

    // Read the JSON configuration file
    nlohmann::json config;
    std::ifstream config_file(config_file_path);
    if (!config_file.is_open()) {
        fmt::println("Failed to open config file: {}", config_file_path);
        return 1;
    }

    try {
        config_file >> config;
    } catch (const std::exception &e) {
        fmt::println("Error reading config file: {}", e.what());
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
        fmt::println("Invalid config file format. Expected 'frpc' and 'configs' keys.");
        return 1;
    }

    const auto frpc_path = config["frpc"].get<std::string>();
    const auto config_files = config["configs"].get<std::vector<std::string>>();

    // Check if all config files exist
    for (const auto &config_file : config_files) {
        if (!std::filesystem::exists(config_file)) {
            fmt::println("Config file does not exist: {}", config_file);
            return 1;
        }
    }

    // Print the frpc binary path and config files
    fmt::println("frpc binary: {}", frpc_path);
    fmt::println("Config files:");
    for (const auto &config_file : config_files) {
        fmt::println(" - {}", config_file);
    }

    // Execute multiple frpc all at background
    for (const auto &config_file : config_files) {
        const auto args = std::array{frpc_path, std::string("-c"), config_file};
        if (!process_manager_.add_process(args)) {
            fmt::println("Failed to start frpc with config: {}", config_file);
            return 1;
        }
        fmt::println("Started frpc with config: {}", config_file);
    }

#ifndef _WIN32
    const auto signals = make_sigset({SIGINT, SIGTERM, SIGABRT, SIGSEGV});
    pthread_sigmask(SIG_BLOCK, &signals, nullptr);
    int last_signal;
    sigwait(&signals, &last_signal);
    fmt::println("Received termination signal: {}", signal_to_str(last_signal));
    process_manager_.terminate_all();
#endif

    process_manager_.wait_all();
    fmt::println("All frpc instances have been executed.");

    return 0;
}