#include <filesystem>
#include <fstream>

#include <signal.h>

#include <fmt/base.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include <reproc++/reproc.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

class MultiProcessManager {
private:
    std::vector<reproc::process> processes_;

public:
    void add_process(std::vector<std::string> args) {
        reproc::process process;
        process.start(args);
        processes_.push_back(std::move(process));
    }

    void terminate_all() {
        fmt::println("Terminating {} processes...", processes_.size());
        for (auto &process : processes_) {
            // First try graceful termination
            auto err = process.terminate();
            if (err) {
                fmt::println("Warning: Failed to terminate process gracefully: {}", err.message());
                // Force kill if graceful termination fails
                process.kill();
            }
        }
        fmt::println("All processes terminated.");
    }

    void wait_all() {
        for (auto &process : processes_) {
            auto [exit_code, err] = process.wait(reproc::infinite);
            if (err) {
                fmt::print("Error waiting for process: {}\n", err.message());
            } else {
                fmt::print("Process exited with code: {}\n", exit_code);
            }
        }
    }
};

static MultiProcessManager multi_process_manager;

void signal_handler(int signal) {
    // convert signal to string for logging
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
    multi_process_manager.terminate_all();
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
    multi_process_manager.terminate_all();

    // Return TRUE to indicate we handled the event
    // This gives us time to clean up before the process is forcibly terminated
    return TRUE;
}
#endif

int main(int argc, char **argv) {
    // accept a json file to specify frpc binary location and config files

    if (argc < 2) {
        fmt::print("Usage: {} <config.json>\n", argv[0]);
        return 1;
    }

    std::string config_file_path = argv[1];
    if (!std::filesystem::exists(config_file_path)) {
        fmt::print("Config file does not exist: {}\n", config_file_path);
        return 1;
    }

    // Register signal handlers for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGABRT, signal_handler);

#ifdef _WIN32
    // Register Windows console control handler for Task Manager termination
    if (!SetConsoleCtrlHandler(console_ctrl_handler, TRUE)) {
        fmt::print("Warning: Failed to set console control handler\n");
    }
#endif

    // Read the JSON configuration file
    nlohmann::json config;
    std::ifstream config_file(config_file_path);
    if (!config_file.is_open()) {
        fmt::print("Failed to open config file: {}\n", config_file_path);
        return 1;
    }

    try {
        config_file >> config;
    } catch (const std::exception &e) {
        fmt::print("Error reading config file: {}\n", e.what());
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
        fmt::print("Invalid config file format. Expected 'frpc' and 'configs' keys.\n");
        return 1;
    }

    std::string frpc_path = config["frpc"].get<std::string>();
    std::vector<std::string> config_files = config["configs"].get<std::vector<std::string>>();

    // detect if the paths is absolute or relative
    // if relative, convert to absolute path using the config file path as base
    std::filesystem::path base_path = std::filesystem::path(config_file_path).parent_path();
    if (!std::filesystem::path(frpc_path).is_absolute()) {
        frpc_path = (base_path / frpc_path).string();
    }
    for (auto &config_file : config_files) {
        if (!std::filesystem::path(config_file).is_absolute()) {
            config_file = (base_path / config_file).string();
        }
    }
    // renormalize paths
    frpc_path = std::filesystem::canonical(frpc_path).string();
    for (auto &config_file : config_files) {
        config_file = std::filesystem::canonical(config_file).string();
    }

    // Check if frpc binary exists
    if (!std::filesystem::exists(frpc_path)) {
        fmt::print("frpc binary does not exist: {}\n", frpc_path);
        return 1;
    }
    // Check if all config files exist
    for (const auto &config_file : config_files) {
        if (!std::filesystem::exists(config_file)) {
            fmt::print("Config file does not exist: {}\n", config_file);
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
        std::vector<std::string> args = {frpc_path, "-c", config_file};
        try {
            multi_process_manager.add_process(std::move(args));
            fmt::print("Started frpc with config: {}\n", config_file);
        } catch (const std::exception &e) {
            fmt::print("Error starting frpc with config {}: {}\n", config_file, e.what());
            return 1;
        }
    }

    multi_process_manager.wait_all();
    fmt::print("All frpc instances have been executed.\n");

    return 0;
}