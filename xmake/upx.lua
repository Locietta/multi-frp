rule("module.program")
    on_load(function (target)
        target:set("kind", "binary")
    end)

    after_build(function (target)
        local enabled = target:extraconf("rules", "module.program", "upx")

        import("core.project.depend")
        import("lib.detect.find_tool")

        local targetfile = target:targetfile()
        depend.on_changed(function ()
            local file = path.join("build", path.filename(targetfile))
            os.tryrm(file)

            if (not enabled) or (not is_mode("release")) then
                os.cp(targetfile, file)
            else
                local upx = assert(find_tool("upx"), "upx not found!")
                os.vrunv(upx.program, {targetfile, "-o", file})
            end
        end, {files = targetfile})
    end)