#include "process/process_manager.h"

#include <charconv>
#include "util/print.hpp"

bool ProcessManager::add_process(std::span<const char *const> args) {
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
        char buffer[11]; // enough for 32-bit int
        if (auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), exit_code); ec == std::errc()) {
            *ptr = '\0';
        } else {
            std::strcpy(buffer, "unknown");
        }

        print("Process exited with code: ", buffer, "\n");
    }
}