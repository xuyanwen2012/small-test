

target("bench-micro")
    set_kind("binary")
    add_includedirs("$(projectdir)/include")
    add_files("main.cpp")
    add_packages("benchmark")
    add_deps("ppl")
