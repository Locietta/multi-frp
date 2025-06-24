add_includedirs(".")

includes("process")

target("multi-frp")
    add_rules("module.program", {upx = true})
    add_files("*.cpp")
    add_packages("fmt", "nlohmann_json")
    add_headerfiles("*.h", "util/*.hpp", { install = false })
    add_deps("process")
