#pragma once

#include "util/trait.hpp"
#include "process/process.h"
#include <vector>
#include <span>
#include <string>

struct ProcessManager : Unique {
    bool add_process(std::span<std::string const> args);
    void terminate_all();
    void wait_all();

private:
    std::vector<Process> processes_;
};
