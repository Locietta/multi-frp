add_includedirs(".")

includes("process")

target("multi-frp")
    set_kind("binary")
    add_files("*.cpp")
    add_packages("fmt", "nlohmann_json")
    add_deps("process")
