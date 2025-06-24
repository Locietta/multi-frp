// windows_process_manager.cpp
#ifdef _WIN32

#include "process/process.h"

#include <fmt/format.h>
#include <windows.h>

struct Process::Impl {
    HANDLE job_handle_ = nullptr;
    HANDLE process_handle_ = nullptr;
    DWORD process_id_ = 0;

    Impl() {
        // Create a job object for automatic cleanup
        job_handle_ = CreateJobObject(nullptr, nullptr);
        if (job_handle_) {
            // Configure job to kill all processes when parent dies
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION job_info = {};
            job_info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
            SetInformationJobObject(job_handle_, JobObjectExtendedLimitInformation,
                                    &job_info, sizeof(job_info));
        }
    }

    ~Impl() {
        if (process_handle_) {
            CloseHandle(process_handle_);
        }
        if (job_handle_) {
            CloseHandle(job_handle_);
        }
    }

    bool start(const std::vector<std::string> &args) {
        if (args.empty()) return false;

        std::string command_line = build_command_line(args);

        STARTUPINFOA startup_info = {};
        startup_info.cb = sizeof(startup_info);

        PROCESS_INFORMATION process_info = {};

        // Create the process
        BOOL result = CreateProcessA(
            nullptr,             // Application name
            command_line.data(), // Command line
            nullptr,             // Process security attributes
            nullptr,             // Thread security attributes
            FALSE,               // Inherit handles
            CREATE_SUSPENDED,    // Creation flags (suspended so we can add to job)
            nullptr,             // Environment
            nullptr,             // Current directory
            &startup_info,       // Startup info
            &process_info        // Process info
        );

        if (!result) {
            return false;
        }

        // Add process to job object for automatic cleanup
        if (job_handle_) {
            AssignProcessToJobObject(job_handle_, process_info.hProcess);
        }

        // Resume the process
        ResumeThread(process_info.hThread);
        CloseHandle(process_info.hThread);

        process_handle_ = process_info.hProcess;
        process_id_ = process_info.dwProcessId;

        return true;
    }

    bool stop(int timeout_ms) {
        if (!process_handle_) return true;

        // Try graceful termination first
        if (!GenerateConsoleCtrlEvent(CTRL_C_EVENT, process_id_)) {
            // If that fails, try TerminateProcess
            TerminateProcess(process_handle_, 1);
        }

        // Wait for process to exit
        DWORD wait_result = WaitForSingleObject(process_handle_, timeout_ms);

        if (wait_result == WAIT_TIMEOUT) {
            // Force kill if timeout
            TerminateProcess(process_handle_, 1);
            WaitForSingleObject(process_handle_, INFINITE);
        }

        return true;
    }

    bool is_running() const {
        if (!process_handle_) return false;

        DWORD exit_code;
        if (!GetExitCodeProcess(process_handle_, &exit_code)) {
            return false;
        }

        return exit_code == STILL_ACTIVE;
    }

    int wait() {
        if (!process_handle_) return -1;

        WaitForSingleObject(process_handle_, INFINITE);

        DWORD exit_code;
        if (GetExitCodeProcess(process_handle_, &exit_code)) {
            return static_cast<int>(exit_code);
        }

        return -1;
    }

    bool is_valid() const {
        return process_handle_ != nullptr;
    }

    // std::unique_ptr<Impl> clone() const { return std::make_unique<Impl>(*this); }

private:
    std::string build_command_line(const std::vector<std::string> &args) {
        std::string result;
        result.reserve(256); // Pre-allocate reasonable size

        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) result.append(" ");
            result.append("\"");
            result.append(args[i]);
            result.append("\"");
        }
        return result;
    }
};

Process::Process() {}

Process::~Process() = default;

bool Process::start(const std::vector<std::string> &args) {
    return impl<Process::Impl>()->start(args);
}

bool Process::stop(int timeout_ms) {
    return impl<Process::Impl>()->stop(timeout_ms);
}

bool Process::is_running() const {
    return impl<Process::Impl>()->is_running();
}

int Process::wait() {
    return impl<Process::Impl>()->wait();
}

bool Process::is_valid() const {
    return impl<Process::Impl>()->is_valid();
}

#endif // _WIN32