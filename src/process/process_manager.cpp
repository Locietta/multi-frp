#include "process/process_manager.h"

#include <fmt/base.h>

bool ProcessManager::add_process(std::span<const char * const> args) {
    Process process;
    if (!process.start(args)) {
        return false;
    }

    processes_.push_back(std::move(process));
    return true;
}

void ProcessManager::terminate_all() {
    for (auto &process : processes_) {
        process.stop();
    }
}

void ProcessManager::wait_all() {
    for (auto &process : processes_) {
        int exit_code = process.wait();
        fmt::println("Process exited with code: {}", exit_code);
    }
}