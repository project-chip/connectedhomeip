add_library(gnu17 INTERFACE)
target_compile_options(gnu17
    INTERFACE 
        $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++17>
        -D_DEFAULT_SOURCE)
target_link_libraries(app PRIVATE gnu17)
