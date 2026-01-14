#pragma once

#include "process/process_manager.h"

struct App final {
    int run(int argc, char *argv[]);

private:
    ProcessManager process_manager_;
};