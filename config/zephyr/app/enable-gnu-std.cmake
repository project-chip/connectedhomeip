function(chip_enable_gnu_std target)
    if(NOT TARGET gnu17)
        add_library(gnu17 INTERFACE)
        target_compile_options(gnu17
            INTERFACE
                $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++17>
                -D_DEFAULT_SOURCE)
    endif()

    get_target_property(target_type ${target} TYPE)
    if(target_type STREQUAL "INTERFACE_LIBRARY")
        target_link_libraries(${target} INTERFACE gnu17)
    else()
        target_link_libraries(${target} PRIVATE gnu17)
    endif()
endfunction()