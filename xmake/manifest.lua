-- fix manifest embedding for clang-cl

rule("embed.manifest")
    set_extensions(".manifest")
    on_load(function (target)
        target:add("ldflags", "/manifest:embed", {force = true})
    end)
    on_buildcmd_file(function (target, batchcmds, sourcefile, opt)
        target:add("ldflags", "/manifestinput:" .. sourcefile, {force = true})

        local target_file = target:targetfile()

        -- add depend for manifest file, so that target will be rebuilt when manifest changed
        batchcmds:add_depfiles(sourcefile)
        batchcmds:set_depmtime(os.mtime(target_file))
        batchcmds:set_depcache(target:dependfile(target_file))
    end)
rule_end()