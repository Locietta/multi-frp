add_includedirs(".")

includes("process")

add_requires("fmt", "daw_json_link")

option("version")
    set_showmenu(true)
    set_default("dev")
    set_description("version string to embed in the build")
    add_defines("MULTI_FRP_VERSION=\"$(version)\"")

target("multi-frp")
    add_rules("module.program")
    if is_os("windows") then
        add_rules("embed.manifest")
        add_files("app.manifest")
    end
    add_options("version")
    add_files("*.cpp")
    add_packages("fmt", "daw_json_link")
    add_headerfiles("*.h", "util/*.hpp", { install = false })
    add_deps("process")
