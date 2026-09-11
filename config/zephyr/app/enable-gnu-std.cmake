add_library(gnu17 INTERFACE)
target_compile_options(gnu17
    INTERFACE 
        $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++17>
        -D_DEFAULT_SOURCE)
if(NOT DEFINED GNU_STD_TARGET)
    set(GNU_STD_TARGET app)
endif()
get_target_property(GNU_STD_TARGET_TYPE ${GNU_STD_TARGET} TYPE)
if(GNU_STD_TARGET_TYPE STREQUAL "INTERFACE_LIBRARY")
    target_link_libraries(${GNU_STD_TARGET} INTERFACE gnu17)
else()
    target_link_libraries(${GNU_STD_TARGET} PRIVATE gnu17)
endif()