
target("demo-core-info")
    set_kind("binary")
    add_files("core-info/main.cpp")
    add_includedirs("$(projectdir)/include")
    if is_plat("android") then
        on_run(run_on_android)
    end
target_end()

target("demo-cpu")
    set_kind("binary")
    add_files("cpu/*.cpp")
    add_includedirs("$(projectdir)/include")
    add_packages("glm")
    add_deps("ppl")
    if is_plat("android") then
        on_run(run_on_android)
    end
target_end()

target("demo-vulkan")
    set_kind("binary")
    add_includedirs("$(projectdir)/include")
    add_files("vulkan/main.cpp")
    add_packages("volk")
    if is_plat("android") then on_run(run_on_android) end
target_end()

target("demo-zheyuan-vulkan")
    set_kind("binary")
    add_files("zheyuan-vulkan/main.cpp")
    add_includedirs("$(projectdir)/include")
    add_packages("glm", "volk", "vulkan-headers", "vulkan-validationlayers")
    -- add_deps("ppl-vulkan")
    if is_plat("android") then on_run(run_on_android) end
target_end()

add_requires("vulkan-validationlayers")
add_requires("vulkan-headers")
-- add_requires("vulkansdk")

target("demo-yanwen-vulkan")
    set_kind("binary")
    add_files("yanwen-vulkan/*.cpp")
    add_includedirs("$(projectdir)/include")
    add_packages("glm", "spdlog", "volk", "vulkan-validationlayers")
    if is_plat("android") then on_run(run_on_android) end
target_end()

-- target("demo-vulkan-hpp")
--     set_kind("binary")
--     add_files("vulkan-hpp/*.cpp")
--     add_packages("spdlog", "vulkansdk")
--     -- add '-lvulkan' to link flags
--     add_ldflags("-lvulkan")
--     -- add_cxxflags("-lvulkan")
--     if is_plat("android") then on_run(run_on_android) end
-- target_end()

-- add_requires("vk-bootstrap")
-- vk-bootstrap

-- target("demo-vkb")
--     set_kind("binary")
--     add_files("vulkan-vkb/*.cpp")
--     add_packages("spdlog", "vk-bootstrap")
--     if is_plat("android") then on_run(run_on_android) end
-- target_end()