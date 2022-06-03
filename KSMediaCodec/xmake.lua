set_xmakever("2.6.3")
includes("../../Foundation/Foundation")

rule("KSMediaCodec.deps")
    on_load(function (target)
        local previous = os.cd(target:scriptdir())
        if os.exists("Vendor/ffmpeg") == false then
            local oldir = os.cd("Vendor")
            import("net.http")
            import("utils.archive")
            if os.exists("ffmpeg.zip") == false then
                http.download("https://github.com/BtbN/FFmpeg-Builds/releases/download/autobuild-2021-02-28-12-32/ffmpeg-n4.3.2-160-gfbb9368226-win64-gpl-shared-4.3.zip", "ffmpeg.zip")
            end
            archive.extract("ffmpeg.zip")
            os.trymv("ffmpeg-n4.3.2-160-gfbb9368226-win64-gpl-shared-4.3", "ffmpeg")
            os.cd(oldir)            
        end    

        target:add("includedirs", target:scriptdir() .. "/Vendor/ffmpeg/include", {public = true})
        target:add("linkdirs", target:scriptdir() .. "/Vendor/ffmpeg/lib")
        target:add("links", "avcodec", "avformat", "avutil", "swresample", "swscale")

        if os.exists("../../Foundation") == false then
            git.clone("https://github.com/lai001/Foundation.git", {branch = "main", outputdir = "../../Foundation"})
        end
        target:add("deps", "Foundation")
        os.cd(previous)
    end)
    after_build(function (target)
        local previous = os.cd(target:scriptdir())
        os.cp("Vendor/ffmpeg/bin/*.dll", target:targetdir())
        os.cd(previous)
    end)

target("KSMediaCodec")
    set_kind("$(kind)")
    set_languages("c++17")
    add_files("src/**.cpp")
    add_headerfiles("include/**.hpp", "include/**.h")
    add_includedirs("include/KSMediaCodec")
    add_includedirs("include", {interface = true})
    add_rules("mode.debug", "mode.release", "KSMediaCodec.deps")
    if is_kind("shared") and is_plat("windows") then
        add_defines("KSMediaCodec_BUILD_DLL_EXPORT")
    end
    on_config(function (target)
        import("core.project.project")
        for _targetname, _target in pairs(project.targets()) do
            local depsType = type(_target:get("deps"))
            local deps = nil;
            if depsType == "table" then
                deps = _target:get("deps")
            elseif depsType == "string" then
                deps = {_target:get("deps")}
            end
            if deps and table.contains(deps, "KSMediaCodec") and target:kind() == "shared" and target:is_plat("windows") then
                _target:add("defines", "KSMediaCodec_DLL")
            end
        end
    end)

target("Foundation")