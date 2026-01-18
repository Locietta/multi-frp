#pragma once

#include <string_view>
#include <cstdio>
#include <cstring>

// print a bunch of strings at once
// first gather all the length of the strings to allocate a buffer
// then call write to print them all at once
template <std::convertible_to<std::string_view>... Args>
constexpr void fprint(FILE *file, Args &&...args) {
    const auto total_size = (std::string_view(args).size() + ...);
    char *buffer = new char[total_size + 1];
    char *ptr = buffer;
    for (const auto &str : {std::string_view(args)...}) {
        std::memcpy(ptr, str.data(), str.size());
        ptr += str.size();
    }
    buffer[total_size] = '\0';
    std::fwrite(buffer, 1, total_size, file);
    delete[] buffer;
}

template <std::convertible_to<std::string_view>... Args>
constexpr void print(Args &&...args) {
    fprint(stdout, std::forward<Args>(args)...);
}
