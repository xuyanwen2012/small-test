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


target("demo-vma-debug")
    set_kind("binary")
    add_includedirs("$(projectdir)/include")
    add_files("vma-debug/*.cpp")
    add_deps("ppl-vulkan")
    add_packages("spdlog", "volk", "vulkan-memory-allocator")
    if is_plat("android") then on_run(run_on_android) end
target_end()


function compile_shaders()
    -- Get all the .comp shaders from the source folder
    local shader_files = os.files("$(projectdir)/ppl/vulkan/shaders/*.comp")

    -- Loop over the shader files and compile them
    for _, shader_file in ipairs(shader_files) do
        local output_path = "$(projectdir)/ppl/vulkan/shaders/compiled_shaders/" .. path.basename(shader_file) .. ".spv"
        local command = "glslangValidator -V --target-env spirv1.3 " .. shader_file .. " -o " .. output_path
        os.run(command)
        print(string.format("Compiled %s to %s", shader_file, output_path))
    end
end

target("demo-vulkan")
    set_kind("binary")
    add_includedirs("$(projectdir)/include")
    add_files("vulkan/*.cpp")
    add_deps("ppl-vulkan")
    add_packages("glm", "spdlog", "volk", "vulkan-memory-allocator")
    if is_plat("android") then on_run(run_on_android) end
    after_build(compile_shaders)
target_end()