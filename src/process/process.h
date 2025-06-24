#pragma once

#include <string>
#include <vector>

#include "util/pimpl.hpp"

struct Process : Pimpl<Process> {
    struct Impl;

    Process();
    ~Process();

    Process(Process &&) noexcept = default;
    Process &operator=(Process &&) noexcept = default;

    bool start(const std::vector<std::string> &args);
    bool stop(int timeout_ms = 5000);
    bool is_running() const;
    int wait();
    bool is_valid() const;
};
