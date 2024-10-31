target("bench-cpu")
    set_kind("binary")
    add_includedirs("$(projectdir)/include")
    add_files("cpu/main.cpp")
    add_packages("benchmark", "glm")
    add_deps("ppl")
    if is_plat("android") then on_run(run_on_android) end
