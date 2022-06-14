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

cmake_minimum_required(VERSION 3.21)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)
get_filename_component(OPENIOTSDK_COMMON ${CHIP_ROOT}/examples/platform/openiotsdk REALPATH)
get_filename_component(SHELL_COMMON ${CHIP_ROOT}/examples/shell/shell_common REALPATH)

set(APP_TARGET chip-openiotsdk-shell-example)

include(${OPENIOTSDK_COMMON}/cmake/sdk.cmake)

project(${APP_TARGET} LANGUAGES C CXX ASM)
add_executable(${APP_TARGET})

# Application CHIP build configuration 
set(CONFIG_CHIP_LIB_SHELL YES)
include(${OPENIOTSDK_COMMON}/cmake/chip.cmake)

target_include_directories(${APP_TARGET} PRIVATE
                           main/include
                           ${SHELL_COMMON}/include
                           freertos-config
)

target_sources(${APP_TARGET} PRIVATE
                main/main.cpp
)

target_link_libraries(${APP_TARGET}
    mcu-driver-hal
    mcu-driver-bootstrap
    mdh-reference-platforms-for-arm

    iotsdk-serial-retarget
    freertos-cmsis-rtos
    freertos-kernel-heap-3

    iotsdk-ip-network-api
    lwipcore
    mbedtls
    
    chip
)

include(${OPENIOTSDK_COMMON}/cmake/linker.cmake)
set_target_link(${APP_TARGET})
