add_library(gnu17 INTERFACE)
target_compile_options(gnu17
    INTERFACE 
        $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++17>
        -D_SYS__PTHREADTYPES_H_)
target_link_libraries(app PRIVATE gnu17)
