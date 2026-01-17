#include "cli_parser.h"

#include <vector>
#include <cstring>
#include "version.h"
#include "util/print.hpp"

namespace {

void print_usage() {
    // clang-format off
    print(
        "Usage: {} [--help] [--version] --config CONFIG_FILE\n"
        "\n"
        "Optional arguments:\n"
        "  -h, --help                shows this help message and exits\n"
        "  -v, --version             prints version information and exits\n"
        "  -c, --config CONFIG_FILE  Path to the JSON configuration file [required]\n",
        program_name
    );
    // clang-format on
}

} // namespace

ParseResult CliParser::parse(this CliParser &self, int argc, char *argv[]) {
    std::vector<const char *> normalized;
    normalized.reserve(argc);

    for (int i = 1; i < argc; ++i) {
        const auto arg = argv[i];

        if (std::strcmp(arg, "-h") == 0 || std::strcmp(arg, "--help") == 0) {
            print_usage();
            return ParseResult::GRACEFUL_EXIT;
        }

        if (std::strcmp(arg, "-v") == 0 || std::strcmp(arg, "--version") == 0) {
            print(program_name, " ", version, "\n");
            return ParseResult::GRACEFUL_EXIT;
        }

        normalized.push_back(arg);
    }

    bool config_provided = false;
    for (size_t i = 0; i < normalized.size(); ++i) {
        const auto arg = normalized[i];

        if (std::strcmp(arg, "-c") == 0 || std::strcmp(arg, "--config") == 0) {
            if (i + 1 >= normalized.size() || normalized[i + 1][0] == 0) {
                print("Error parsing arguments: -c/--config requires CONFIG_FILE.\n");
                print_usage();
                return ParseResult::ERR;
            }

            i += 1;
            const auto config_value = normalized[i];

            self.config_file_path = std::filesystem::path(config_value);
            config_provided = true;
            continue;
        }

        print("Error parsing arguments: unknown option '", arg, "'.\n");
        return ParseResult::ERR;
    }

    if (!config_provided) {
        print("Error parsing arguments: -c: required.\n");
        print_usage();
        return ParseResult::ERR;
    }

    return ParseResult::SUCCESS;
}