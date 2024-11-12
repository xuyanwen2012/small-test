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


-- target("demo-vma-debug")
--     set_kind("binary")
--     add_includedirs("$(projectdir)/include")
--     add_files("vma-debug/*.cpp")
--     add_deps("ppl-vulkan")
--     add_packages("spdlog", "volk", "vulkan-memory-allocator")
--     if is_plat("android") then on_run(run_on_android) end
-- target_end()


function compile_shaders()
    local shader_files = os.files("$(projectdir)/ppl/vulkan/shaders/*.comp")

    for _, shader_file in ipairs(shader_files) do
        local output_path = "$(projectdir)/ppl/vulkan/shaders/compiled_shaders/" .. path.basename(shader_file) .. ".spv"
        -- local command = "glslangValidator --target-env vulkan1.2 -e main -o " .. output_path .. " " .. shader_file
        local command = "glslangValidator -V --target-env spirv1.3 " .. shader_file .. " -o " .. output_path
        os.run(command)
        print(string.format("Compiled %s to %s", shader_file, output_path))
    end
end

-- function copy_shaders(target)
--     local shader_dir = "$(projectdir)/ppl/vulkan/shaders/compiled_shaders"
--     -- local remote_path = "/data/local/tmp/shaders"
--     -- local remote_path = -- target's exe
--     os.run("adb push " .. shader_dir .. " " .. remote_path)
-- end

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