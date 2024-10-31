
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

-- target("demo-memory")
--     set_kind("binary")
--     add_files("memory/main.cpp")
--     add_includedirs("$(projectdir)/include")
--     if is_plat("android") then
--         on_run(run_on_android)
--     end
-- target_end()

-- if not is_plat("android") then
-- target("demo-memory-cu")
--     set_kind("binary")
--     add_files("memory/main.cu")
--     add_includedirs("$(projectdir)/include")
--     add_cugencodes("native")
-- target_end()
-- end

