add_rules("mode.debug", "mode.release", "mode.check")

target("monny")
    set_kind("binary")
    add_files("src/**.cpp")
    set_optimize("fastest")
    add_includedirs("./include", "$(exec node -p \"require('node-addon-api').include\")")
    add_cflags("-fno-exceptions", {apid = "c++"})
    add_cxflags("-fno-exceptions", {apid = "c"}, "-fsanitize=address")
    add_links("napi")
    if is_mode("debug") then
        add_mxflags("-fsanitise=address")
        add_ldflags("-fsanitize=address")       
    end

