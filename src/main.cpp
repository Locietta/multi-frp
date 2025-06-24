#include <filesystem>
#include <fstream>

#include <fmt/base.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "process/process_manager.h"
#include "signal_handler.h"

ProcessManager process_manager;

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

    register_signal_handlers();

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
            process_manager.add_process(std::move(args));
            fmt::print("Started frpc with config: {}\n", config_file);
        } catch (const std::exception &e) {
            fmt::print("Error starting frpc with config {}: {}\n", config_file, e.what());
            return 1;
        }
    }

    process_manager.wait_all();
    fmt::print("All frpc instances have been executed.\n");

    return 0;
}