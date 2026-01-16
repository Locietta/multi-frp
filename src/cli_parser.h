#pragma once

#include <filesystem>

enum class ParseResult : unsigned char {
    SUCCESS,
    GRACEFUL_EXIT,
    ERR,
};

struct CliParser final {
    ParseResult parse(this CliParser &self, int argc, char *argv[]);

    std::filesystem::path config_file_path;
};