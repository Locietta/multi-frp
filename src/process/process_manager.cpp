#include "process/process_manager.h"

#include <stdexcept>
#include <fmt/base.h>

void ProcessManager::add_process(std::span<std::string const> args) {
    Process process;
    if (process.start(args)) {
        processes_.push_back(std::move(process));
    } else {
        throw std::runtime_error("Failed to start process");
    }
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