target("bench-cpu-unpinned")
    set_kind("binary")
    add_includedirs("$(projectdir)/include")
    add_files("cpu/unpinned.cpp")
    add_packages("benchmark", "glm")
    add_deps("ppl")
    if is_plat("android") then on_run(run_on_android) end

target("bench-cpu-pinned")
    set_kind("binary")
    add_includedirs("$(projectdir)/include")
    add_files("cpu/pinned.cpp")
    add_packages("benchmark", "glm")
    add_deps("ppl")
    if is_plat("android") then on_run(run_on_android) end
