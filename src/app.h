#pragma once

#include "process/process_manager.h"
#include "util/str.hpp"

struct App final {
    int run(int argc, cstr argv[]);

private:
    ProcessManager process_manager_;
};