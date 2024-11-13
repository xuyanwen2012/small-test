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

function compile_shaders()
    local shader_files = os.files("$(projectdir)/ppl/vulkan/shaders/*.comp")

    for _, shader_file in ipairs(shader_files) do
        local output_path = "$(projectdir)/ppl/vulkan/shaders/compiled_shaders/" .. path.basename(shader_file) .. ".spv"
        -- local command = "glslangValidator --target-env vulkan1.2 -e main -o " .. output_path .. " " .. shader_file
        local command = "glslangValidator -V --target-env spirv1.3 " .. shader_file .. " -o " .. output_path
        print(command)
        os.run(command)
    end
end

target("demo-vulkan")
    set_kind("binary")
    add_includedirs("$(projectdir)/include")
    add_files("vulkan/*.cpp")
    add_deps("ppl-vulkan")
    add_packages("glm", "spdlog", "volk", "vulkan-memory-allocator")
    if is_plat("android") then on_run(run_on_android) end
    -- after_build(compile_shaders)
target_end()

target("demo-vulkan-pipe")
    set_kind("binary")
    add_includedirs("$(projectdir)/include")
    add_files("vulkan-pipe/*.cpp")
    add_deps("ppl-vulkan")
    add_packages("glm", "spdlog", "volk", "vulkan-memory-allocator")
    if is_plat("android") then on_run(run_on_android) end
    after_build(compile_shaders)
target_end()

-- Vulkan kernels --------------------------------------------------------------

target("demo-vulkan-kernel-prefix-sum")
    set_kind("binary")
    add_files("vulkan-kernels/prefix_sum.cpp")
    add_includedirs("$(projectdir)/include")
    add_deps("ppl-vulkan")
    add_packages("glm", "spdlog", "volk", "vulkan-memory-allocator")
    if is_plat("android") then on_run(run_on_android) end
target_end()

target("demo-vulkan-kernel-sort")
    set_kind("binary")
    add_files("vulkan-kernels/sort.cpp")
    add_includedirs("$(projectdir)/include")
    add_deps("ppl-vulkan")
    add_packages("glm", "spdlog", "volk", "vulkan-memory-allocator")
    if is_plat("android") then on_run(run_on_android) end
target_end()


target("demo-vulkan-reflect")
    set_kind("binary")
    add_files("vulkan-reflect/main.cpp")
    add_includedirs("$(projectdir)/include")
    add_packages("spdlog", "volk", "spirv-reflect")
    add_deps("ppl-vulkan")
    if is_plat("android") then on_run(run_on_android) end
target_end()
