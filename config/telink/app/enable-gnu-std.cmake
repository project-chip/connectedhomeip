add_library(gnu20 INTERFACE)
target_compile_options(gnu20
    INTERFACE
        $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++20>
        -D_DEFAULT_SOURCE)
target_link_libraries(app PRIVATE gnu20)
