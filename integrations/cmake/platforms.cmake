#
#
#    Copyright (c) 2020 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

#
#    Description:
#      This file is the CHIP SDK CMake template for defining platform-specific build parameters.
#

set(CHIP_PLATFORM "standalone" CACHE STRING "Target platform chosen by the user at configure time")

# Get a list of the available platforms and output as a list to the 'arg_platforms' argument
function(CHIP_get_platforms arg_platforms)
    set(result "standalone")
    set(platforms_dir "${CHIP_PROJECT_SOURCE_DIR}/integrations/cmake/platforms")
    file(GLOB platforms RELATIVE "${platforms_dir}" "${platforms_dir}/*")
    foreach(platform IN LISTS platforms)
        if(IS_DIRECTORY "${platforms_dir}/${platform}")
            list(APPEND result "${platform}")
        endif()
    endforeach()

    set(${arg_platforms} "${result}" PARENT_SCOPE)
endfunction()

CHIP_get_platforms(CHIP_PLATFORMS)

set_property(CACHE CHIP_PLATFORM PROPERTY STRINGS ${CHIP_PLATFORMS})

if(NOT CHIP_PLATFORM IN_LIST CHIP_PLATFORMS)
    message(FATAL_ERROR "Platform unknown: ${CHIP_PLATFORM}")
endif()

set(CHIP_PLATFORM_CMAKE 
    "${CHIP_PROJECT_SOURCE_DIR}/integrations/cmake/platforms/${CHIP_PLATFORM}"
)
include("${CHIP_PLATFORM_CMAKE}/platform.cmake")
