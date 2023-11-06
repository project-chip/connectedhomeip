#
#   Copyright (c) 2022-2023 Project CHIP Authors
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
get_filename_component(OPEN_IOT_SDK_CONFIG ${CHIP_ROOT}/config/openiotsdk REALPATH)
get_filename_component(OPEN_IOT_SDK_EXAMPLE_COMMON ${CHIP_ROOT}/examples/platform/openiotsdk REALPATH)
get_filename_component(SHELL_COMMON ${CHIP_ROOT}/examples/shell/shell_common REALPATH)

list(APPEND CMAKE_MODULE_PATH ${OPEN_IOT_SDK_CONFIG}/cmake)

set(APP_TARGET chip-openiotsdk-shell-example_ns)

# Toolchain files need to exist before first call to project
include(toolchain)

project(${APP_TARGET} LANGUAGES C CXX ASM)

include(sdk)

add_executable(${APP_TARGET})

# Application CHIP build configuration
set(CONFIG_CHIP_LIB_SHELL YES)
include(chip)

add_subdirectory(${OPEN_IOT_SDK_EXAMPLE_COMMON}/app ./app_build)

target_compile_definitions(openiotsdk-startup
    PRIVATE
        IOT_SDK_APP_SERIAL_BAUDRATE=9600
)

target_compile_definitions(openiotsdk-startup
    PRIVATE
        IOT_SDK_APP_MAIN_STACK_SIZE=8192
)

target_include_directories(${APP_TARGET}
    PRIVATE
        main/include
        ${SHELL_COMMON}/include
)

target_sources(${APP_TARGET}
    PRIVATE
        main/main_ns.cpp
        ${SHELL_COMMON}/cmd_misc.cpp
        ${SHELL_COMMON}/globals.cpp
)

target_link_libraries(${APP_TARGET}
    openiotsdk-startup
    openiotsdk-app
)

include(linker)
set_target_link(${APP_TARGET})

sdk_post_build(${APP_TARGET})
