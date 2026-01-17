#pragma once

#include <string_view>
#include <cstdio>
#include <cstring>

// print a bunch of strings at once
// first gather all the length of the strings then allocate a buffer
// then call write to print them all at once
template <typename... Args>
constexpr void print(std::string_view format, Args&&... args) {
    const auto total_size = (format.size() + ... + std::string_view(args).size());
    // debug
    // std::printf("Total size: %zu\n", total_size);
    char *buffer = new char[total_size + 1];
    char *ptr = buffer;
    for (const auto &str : {format, std::string_view(args)...}) {
        std::memcpy(ptr, str.data(), str.size());
        ptr += str.size();
    }
    buffer[total_size] = '\0';
    std::fwrite(buffer, 1, total_size, stdout);
    delete[] buffer;
}