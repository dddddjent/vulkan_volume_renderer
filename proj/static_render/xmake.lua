add_rules("mode.release", "mode.debug")

includes("./../../src/engine/xmake.lua")

add_requires("vulkansdk", "glfw 3.4", "glm 1.0.1")
add_requires("glslang 1.3", { configs = { binaryonly = true } })
add_requires("imgui 1.91.1",  {configs = {glfw_vulkan = true}})
add_requires("cuda", {system=true, configs={utils={"cublas","cusparse","cusolver"}}})

set_policy("build.intermediate_directory", false)

target("static_render")
    if is_plat("windows") then
        add_rules("plugin.vsxmake.autoupdate")
        add_cxxflags("/utf-8")
    end
    add_rules("utils.glsl2spv", { outputdir = "build" })

    set_languages("cxx20")
    set_kind("binary")

    add_headerfiles("*.h")
    add_files("*.cpp")
    add_files("*.cu")
    add_includedirs(".",{public=true})

    add_cugencodes("compute_75")
    add_cuflags("--std c++20", "-lineinfo")

    add_deps("engine")

    add_packages("imgui")
    add_packages("vulkansdk", "glfw", "glm")
    add_packages("cuda")

    if is_mode("debug") then
        add_cxxflags("-DDEBUG")
    end
    if is_mode("release") then
        add_cxxflags("-DNDEBUG")
    end
