target("ppl")
    set_kind("static")
    add_files("host/*.cpp")
    add_includedirs("$(projectdir)/include")
    add_packages("glm")
