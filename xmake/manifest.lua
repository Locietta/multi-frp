-- fix manifest embedding for clang-cl

rule("embed.manifest")
    set_extensions(".manifest")
    on_load(function (target)
        target:add("ldflags", "/manifest:embed", {force = true})
    end)
    on_buildcmd_file(function (target, batchcmds, sourcefile, opt)
        target:add("ldflags", "/manifestinput:" .. sourcefile, {force = true})
    end)
