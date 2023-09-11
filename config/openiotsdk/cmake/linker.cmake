#
# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

#
#   @file
#     CMake linker configuration for target
#

function(set_target_link target)
    target_link_libraries(${target}
        mdh-arm-corstone-300-startup
    )

    if (NOT LINKER_SCRIPT)        
        set(LINKER_SCRIPT ${OPEN_IOT_SDK_CONFIG}/ld/cs300_gcc.ld)
    endif()
    target_link_options(${target} PRIVATE -T ${LINKER_SCRIPT})
    set_target_properties(${target} PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

    target_link_options(${target}
        PRIVATE
            "-Wl,-Map=${target}.map"
    )
endfunction()

set(CMAKE_EXECUTABLE_SUFFIX_CXX .elf)
