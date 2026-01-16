#pragma once

#include <span>

#include "util/pimpl.hpp"
#include "util/str.hpp"

struct Process : Pimpl<Process> {
    struct Impl;

    Process();
    ~Process();

    Process(Process &&) noexcept = default;
    Process &operator=(Process &&) noexcept = default;

    bool start(std::span<const_cstr const> args);
    bool stop(int timeout_ms = 5000);
    bool is_running() const;
    int wait();
    bool is_valid() const;
};
