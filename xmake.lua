add_rules("mode.debug", "mode.release")

add_requires("sfml 2.6.x", {configs = {graphics = true, window = true, system = true}})

if is_plat("linux", "macosx") then
    add_requires("tbb")
end

target("Chess")
    set_kind("binary")
    set_languages("c++23")
    add_files("src/*.cpp")
    add_includedirs("include")
    add_packages("sfml")
    
    if is_plat("linux", "macosx") then
        add_packages("tbb")
    end

    if is_plat("windows") then
        after_build(function (target)
            import("core.base.option")
            local bin_dir = target:targetdir()
            -- Note: xmake handles most DLL copies via `xmake run`, 
            -- but manual copies can be added here if necessary.
        end)
    end
