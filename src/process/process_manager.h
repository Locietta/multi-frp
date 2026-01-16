#pragma once

#include "util/trait.hpp"
#include "process/process.h"
#include <vector>
#include <span>

struct ProcessManager : Unique {
    bool add_process(std::span<const char *const> args);
    void terminate_all();
    void wait_all();

private:
    std::vector<Process> processes_;
};
