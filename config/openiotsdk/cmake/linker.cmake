#
#   Copyright (c) 2022 Project CHIP Authors
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
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
