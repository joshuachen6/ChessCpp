add_rules("mode.debug", "mode.release")

add_requires("sfml 2.6.1", {configs = {graphics = true, window = true, system = true}})

if is_plat("linux", "macosx") then
    add_requires("tbb")
end

target("Chess")
    set_kind("binary")
    set_languages("c++23")
    add_files("src/*.cpp")
    add_files("src/imgui/*.cpp")
    add_includedirs("include")
    add_includedirs("src/imgui")
    add_packages("sfml")
    
    if is_plat("linux", "macosx") then
        add_packages("tbb")
    end

    after_build(function (target)
        os.cp("$(projectdir)/res", target:targetdir())
    end)
