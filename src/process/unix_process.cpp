#ifndef _WIN32

#include "process/process.h"

#include <csignal>
#include <sys/wait.h>
#include <vector>

struct Process::Impl {
    pid_t pid_ = -1;
    int exit_code_ = -1;
    bool has_exited_ = false;
    
    Impl() = default;
    
    ~Impl() {
        if (pid_ > 0 && !has_exited_) {
            stop(5000);
        }
    }
    
    bool start(const std::vector<std::string>& args) {
        if (args.empty()) return false;
        
        // Convert to char* array for execvp
        std::vector<char*> argv;
        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);
        
        pid_ = fork();
        
        if (pid_ == 0) {
            // Child process
            // Create new process group
            setpgid(0, 0);
            
            execvp(argv[0], argv.data());
            
            // If execvp returns, there was an error
            _exit(1);
        } else if (pid_ > 0) {
            // Parent process
            return true;
        } else {
            // Fork failed
            return false;
        }
    }
    
    bool stop(int timeout_ms) {
        if (pid_ <= 0) return true;
        
        // Try graceful termination first
        kill(pid_, SIGTERM);
        
        // Wait for process to exit
        int status;
        for (int i = 0; i < timeout_ms / 10; ++i) {
            pid_t result = waitpid(pid_, &status, WNOHANG);
            if (result == pid_) {
                exit_code_ = WEXITSTATUS(status);
                has_exited_ = true;
                return true;
            }
            usleep(10000); // 10ms
        }
        
        // Force kill if timeout
        kill(pid_, SIGKILL);
        waitpid(pid_, &status, 0);
        exit_code_ = WEXITSTATUS(status);
        has_exited_ = true;
        
        return true;
    }
    
    bool is_running() const {
        if (pid_ <= 0) return false;
        if (has_exited_) return false;
        
        // Check if process still exists
        return kill(pid_, 0) == 0;
    }
    
    int wait() {
        if (pid_ <= 0) return -1;
        if (has_exited_) return exit_code_;
        
        int status;
        waitpid(pid_, &status, 0);
        exit_code_ = WEXITSTATUS(status);
        has_exited_ = true;
        
        return exit_code_;
    }
    
    bool is_valid() const {
        return pid_ > 0;
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

#endif // !_WIN32