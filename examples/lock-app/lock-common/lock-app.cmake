#
#   Copyright (c) 2023 Project CHIP Authors
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

cmake_minimum_required(VERSION 3.21)

set(CHIP_LOCK_COMMON_BASE_DIR ${CMAKE_CURRENT_LIST_DIR})


# Add common lock sources to the specific target
# [Args]:
#   target - target name
# Available options are:
#   SCOPE   sources scope for the target, PRIVATE as default  
macro(chip_add_lock_app_common target)
    set(SCOPE PRIVATE)
    cmake_parse_arguments(ARG "" "SCOPE" "" ${ARGN})
    if (ARG_SCOPE)
        set(SCOPE ${ARG_SCOPE})
    endif()

    target_include_directories(${target} 
        ${SCOPE}
            ${CHIP_LOCK_COMMON_BASE_DIR}/include
    )

    target_sources(${target}
        ${SCOPE}
            ${CHIP_LOCK_COMMON_BASE_DIR}/src/ZCLDoorLockCallbacks.cpp
            ${CHIP_LOCK_COMMON_BASE_DIR}/src/LockManager.cpp
            ${CHIP_LOCK_COMMON_BASE_DIR}/src/LockEndpoint.cpp
    )
endmacro()
