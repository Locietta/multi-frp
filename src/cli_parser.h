#pragma once

#include "util/str.hpp"
#include <filesystem>

enum class ParseResult : unsigned char {
    SUCCESS,
    GRACEFUL_EXIT,
    ERR,
};

struct CliParser final {
    ParseResult parse(this auto &self, int argc, native_cstr argv[]);

    std::filesystem::path config_file_path;
};