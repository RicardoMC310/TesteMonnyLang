add_rules("mode.debug", "mode.release", "mode.check")

target("monny")
    set_kind("binary")
    add_files("src/**.cpp")
    set_optimize("fastest")
    add_includedirs("./include")
    if is_mode("debug") then
        add_cxflags("-fsanitize=address")
        add_mxflags("-fsanitise=address")
        add_ldflags("-fsanitize=address")
    end

