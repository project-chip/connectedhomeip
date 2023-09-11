#
# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
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
