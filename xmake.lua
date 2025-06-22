set_project("multi-frp")
set_description("Simple wrapper for frp to connect to multiple servers")

add_rules("mode.release", "mode.debug", "mode.releasedbg")
set_languages("cxx23")

if is_os("windows") then
    set_toolchains("clang-cl")
    add_defines("_CRT_SECURE_NO_WARNINGS")
    add_defines("WIN32_LEAN_AND_MEAN", "UNICODE", "_UNICODE", "NOMINMAX", "_WINDOWS")
end

add_requires("fmt", "nlohmann_json", "reproc")

target("multi-frp")
    set_kind("binary")
    add_files("src/*.cpp")
    add_includedirs("src", {public = true})
    add_packages("fmt", "nlohmann_json", "filesystem", "reproc")
