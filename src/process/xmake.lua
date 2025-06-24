

target("process")
    set_kind("static")
    add_files("*.cpp")
    add_packages("fmt")
    if is_os("windows") then
        add_syslinks("kernel32", "user32", "shell32")
    end