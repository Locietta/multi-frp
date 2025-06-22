#include <cstdlib>
#include <filesystem>
#include <fstream>

#include <future>

#include <fmt/base.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

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
            "path/to/config1.ini",
            "path/to/config2.ini"
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
    std::vector<std::future<int>> futures;
    for (const auto &config_file : config_files) {
        futures.emplace_back(std::async(std::launch::async, [frpc_path, config_file]() {
            std::string command = fmt::format("{} -c {}", frpc_path, config_file);
            fmt::print("Executing command: {}\n", command);
            int result = std::system(command.c_str());
            if (result != 0) {
                fmt::print("Command failed with exit code: {}\n", result);
            }
            return result;
        }));
    }

    // Wait for all futures to complete
    for (auto &future : futures) {
        future.get();
    }
    fmt::print("All frpc instances have been executed.\n");

    return 0;
}