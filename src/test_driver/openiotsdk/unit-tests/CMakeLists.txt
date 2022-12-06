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
get_filename_component(OPEN_IOT_SDK_CONFIG ${CHIP_ROOT}/config/openiotsdk REALPATH)
get_filename_component(OPEN_IOT_SDK_EXAMPLE_COMMON ${CHIP_ROOT}/examples/platform/openiotsdk REALPATH)

list(APPEND CMAKE_MODULE_PATH ${OPEN_IOT_SDK_CONFIG}/cmake)

# Toolchain files need to exist before first call to project
include(toolchain)

project(chip-open-iot-sdk-unit-tests LANGUAGES C CXX ASM)

include(sdk)
include(linker)

# LwIP configuration
if(TARGET lwip-cmsis-port)
    # lwip requires user_lwipopts.h, we use the custom settings
    target_include_directories(lwipopts
        INTERFACE
            lwip-config
    )
endif()

# Application CHIP build configuration 
set(CONFIG_CHIP_LIB_TESTS YES)
set(CONFIG_CHIP_DETAIL_LOGGING NO)
set(CONFIG_CHIP_PROGRESS_LOGGING NO)
set(CONFIG_CHIP_AUTOMATION_LOGGING YES)
set(CONFIG_CHIP_ERROR_LOGGING NO)

include(chip)

add_subdirectory(${OPEN_IOT_SDK_EXAMPLE_COMMON}/app ./app_build)

file(STRINGS testnames.txt TEST_NAMES_FROM_FILE)
STRING(REGEX REPLACE "\n" ";" TEST_NAMES_FROM_FILE "${TEST_NAMES_FROM_FILE}")

foreach(TEST_NAME IN LISTS TEST_NAMES_FROM_FILE)
    add_executable(${TEST_NAME})
    target_include_directories(${TEST_NAME}
        PRIVATE
            main/include
            ${CHIP_ROOT}/third_party/nlunit-test/repo/src
    )

    target_sources(${TEST_NAME}
        PRIVATE
            main/main.cpp
    )

    target_link_libraries(${TEST_NAME} 
        openiotsdk-app
    )

    # Link the *whole-archives* to keep the static test objects.
    target_link_options(${TEST_NAME}
        PUBLIC
            -Wl,--whole-archive "${CMAKE_CURRENT_BINARY_DIR}/chip_build/lib/lib${TEST_NAME}.a"
            -Wl,--no-whole-archive "${CMAKE_CURRENT_BINARY_DIR}/chip_build/lib/libCHIP_tests.a"
            -Wl,--no-whole-archive)

    # set_target_link requires APP_TARGET to be defined
    set(APP_TARGET ${TEST_NAME})
    set_target_link(${TEST_NAME})
endforeach()
