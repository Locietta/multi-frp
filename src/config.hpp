#pragma once

#include <string>
#include <vector>

#include <daw/json/daw_json_link.h>

/// Example JSON structure:
/*
{
    "frpc": "path/to/frpc",
    "configs": [
        "path/to/config1.toml",
        "path/to/config2.toml"
    ]
}
*/

struct Config final {
    std::string frpc;
    std::vector<std::string> configs;
};

namespace daw::json {

template <>
struct json_data_contract<Config> {
    using type = json_member_list<
        json_string<"frpc">,
        json_array<"configs", std::string>>;
};

} // namespace daw::json