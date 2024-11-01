-- ---------------------------------------------------------------------
-- Foundations
-- ---------------------------------------------------------------------

target("test-glm")
    set_kind("binary")
    add_files("foundations/test-glm.cpp")
    add_packages("gtest", "glm")
    if is_plat("android") then on_run(run_on_android) end
target_end()

target("test-vulkan")
    set_kind("binary")
    add_includedirs("$(projectdir)/include")
    add_files("foundations/test-vulkan.cpp")
    add_packages("gtest", "vulkan-headers", "volk")
    if is_plat("android") then on_run(run_on_android) end
target_end()

if not is_plat("android") then
target("test-cuda")
    set_kind("binary")
    add_files("foundations/test-cuda.cu")
    add_packages("gtest")
    add_cugencodes("native")
target_end()
end

target("test-threadpool")
    set_kind("binary")
    add_includedirs("$(projectdir)/include")
    add_files("foundations/test-threadpool.cpp")
    add_packages("gtest")
    if is_plat("android") then on_run(run_on_android) end
target_end()

-- ---------------------------------------------------------------------
-- Thread Pinning
-- ---------------------------------------------------------------------

target("test-thread-pinning")
    set_kind("binary")
    add_files("thread_pinning.cpp")
    add_packages("gtest")
    if is_plat("android") then on_run(run_on_android) end
target_end()
