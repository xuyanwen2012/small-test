add_requires("spdlog")

target("demo-core-info")
    set_kind("binary")
    add_files("core-info/main.cpp")
    add_includedirs("$(projectdir)/include")
    add_packages("spdlog")
    if is_plat("android") then
        on_run(run_on_android)
    end
target_end()

target("demo-cpu")
    set_kind("binary")
    add_files("cpu/*.cpp")
    add_includedirs("$(projectdir)/include")
    add_packages("glm", "spdlog")
    add_deps("ppl")
    if is_plat("android") then
        on_run(run_on_android)
    end
target_end()

target("demo-vulkan-info")
    set_kind("binary")
    add_includedirs("$(projectdir)/include")
    add_files("vulkan-info/main.cpp")
    add_packages("volk", "spdlog")
    if is_plat("android") then on_run(run_on_android) end
target_end()

-- target("demo-zheyuan-vulkan")
--     set_kind("binary")
--     add_files("zheyuan-vulkan/main.cpp")
--     add_includedirs("$(projectdir)/include")
--     add_packages("glm", "volk", "vulkan-headers", "vulkan-validationlayers", "spdlog")
--     -- add_deps("ppl-vulkan")
--     if is_plat("android") then on_run(run_on_android) end
-- target_end()

-- target("demo-vma")
--     set_kind("binary")
--     add_files("vma/main.cpp")
--     add_includedirs("$(projectdir)/include")
--     add_packages("volk", "vulkan-memory-allocator", "spdlog")
--     if is_plat("android") then on_run(run_on_android) end
-- target_end()

target("demo-vma-debug")
    set_kind("binary")
    add_includedirs("$(projectdir)/include")
    add_files("vma-debug/*.cpp")
    add_deps("ppl-vulkan")
    add_packages("spdlog", "volk", "vulkan-memory-allocator")
    if is_plat("android") then on_run(run_on_android) end
target_end()