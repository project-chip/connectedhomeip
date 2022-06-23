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

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../.. REALPATH)
get_filename_component(OPENIOTSDK_COMMON ${CHIP_ROOT}/examples/platform/openiotsdk REALPATH)

set(APP_TARGET chip-openiotsdk-unit-tests)

include(${OPENIOTSDK_COMMON}/cmake/sdk.cmake)

project(${APP_TARGET} LANGUAGES C CXX ASM)
add_executable(${APP_TARGET})

# Application CHIP build configuration 
set(CONFIG_CHIP_LIB_TESTS YES)
include(${OPENIOTSDK_COMMON}/cmake/chip.cmake)

target_include_directories(${APP_TARGET} PRIVATE
                           main/include
)

target_sources(${APP_TARGET} PRIVATE
                main/main.cpp
)

target_link_libraries(${APP_TARGET} 
    cmsis-rtos-implementation
    openiotsdk-chip
)

# Link the *whole-archives* to keep the static test objects.
target_link_options(${APP_TARGET} PUBLIC -Wl,--whole-archive "${CMAKE_CURRENT_BINARY_DIR}/chip_build/lib/libCHIP_tests_custom.a" -Wl,--no-whole-archive)

include(${OPENIOTSDK_COMMON}/cmake/linker.cmake)
set_target_link(${APP_TARGET})
