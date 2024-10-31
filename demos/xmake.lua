
target("demo1")
    set_kind("binary")
    add_files("demo1.cpp")
    add_includedirs("$(projectdir)/include")
    if is_plat("android") then
        on_run(run_on_android)
    end
target_end()


target("demo-cpu")
    set_kind("binary")
    add_files("cpu-only/*.cpp")
    add_includedirs("$(projectdir)/include")
    add_packages("glm")
    if is_plat("android") then
        on_run(run_on_android)
    end
target_end()

