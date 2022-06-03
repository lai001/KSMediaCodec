set_xmakever("2.6.3")
includes("../KSMediaCodec")
includes("../../Foundation/Foundation")

add_requires("spdlog")

target("Example")
    set_kind("binary")
    set_languages("c++17")
    add_files("main.cpp")
    add_rules("mode.debug", "mode.release")
    add_deps("KSMediaCodec")
    add_deps("Foundation")
    add_packages("spdlog")
    after_build(function (target)
        os.rm(target:targetdir() .. "/Resource")
        os.cp("Resource", target:targetdir())
    end)