#pragma once

#include "util/trait.hpp"
#include "process/process.h"
#include <vector>
#include <string>

struct ProcessManager : Unique {
    void add_process(std::vector<std::string> args);
    void terminate_all();
    void wait_all();

private:
    std::vector<Process> processes_;
};
