#include "cli_parser.h"

#include <vector>
#include "version.h"

namespace {

void print_usage() {
    // clang-format off
    fmt::print(
        STR("Usage: {} [--help] [--version] --config CONFIG_FILE\n")
        STR("\n")
        STR("Optional arguments:\n")
        STR("  -h, --help                shows this help message and exits\n")
        STR("  -v, --version             prints version information and exits\n")
        STR("  -c, --config CONFIG_FILE  Path to the JSON configuration file [required]\n"),
        program_name
    );
    // clang-format on
}

} // namespace

ParseResult CliParser::parse(this CliParser &self, int argc, cstr argv[]) {
    std::vector<const_cstr> normalized;
    normalized.reserve(argc);

    for (int i = 1; i < argc; ++i) {
        const auto arg = argv[i];

        if (STRCMP(arg, STR("-h")) == 0 || STRCMP(arg, STR("--help")) == 0) {
            print_usage();
            return ParseResult::GRACEFUL_EXIT;
        }

        if (STRCMP(arg, STR("-v")) == 0 || STRCMP(arg, STR("--version")) == 0) {
            fmt::println(STR("{} {}"), program_name, version);
            return ParseResult::GRACEFUL_EXIT;
        }

        normalized.push_back(arg);
    }

    bool config_provided = false;
    for (size_t i = 0; i < normalized.size(); ++i) {
        const auto arg = normalized[i];

        if (STRCMP(arg, STR("-c")) == 0 || STRCMP(arg, STR("--config")) == 0) {
            if (i + 1 >= normalized.size() || normalized[i + 1][0] == 0) {
                fmt::println(STR("Error parsing arguments: -c/--config requires CONFIG_FILE."));
                print_usage();
                return ParseResult::ERR;
            }

            i += 1;
            const auto config_value = normalized[i];

            self.config_file_path = std::filesystem::path(config_value);
            config_provided = true;
            continue;
        }

        fmt::println(STR("Error parsing arguments: unknown option '{}'"), arg);
        return ParseResult::ERR;
    }

    if (!config_provided) {
        fmt::println(STR("Error parsing arguments: -c: required."));
        print_usage();
        return ParseResult::ERR;
    }

    return ParseResult::SUCCESS;
}