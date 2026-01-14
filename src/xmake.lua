add_includedirs(".")

includes("process")

add_requires("fmt", "nlohmann_json", "argparse")

target("multi-frp")
    add_rules("module.program")
    add_files("*.cpp")
    add_packages("fmt", "nlohmann_json", "argparse")
    add_headerfiles("*.h", "util/*.hpp", { install = false })
    add_deps("process")
