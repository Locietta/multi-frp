add_includedirs(".")

includes("process")

add_requires("fmt", "nlohmann_json", "argparse")

option("version")
    set_showmenu(true)
    set_default("dev")
    set_description("version string to embed in the build")
    add_defines("MULTI_FRP_VERSION=\"$(version)\"")

target("multi-frp")
    add_rules("module.program")
    add_options("version")
    add_files("*.cpp")
    add_packages("fmt", "nlohmann_json", "argparse")
    add_headerfiles("*.h", "util/*.hpp", { install = false })
    add_deps("process")
