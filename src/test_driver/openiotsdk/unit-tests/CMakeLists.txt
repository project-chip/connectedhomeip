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

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../.. REALPATH)
get_filename_component(OPEN_IOT_SDK_CONFIG ${CHIP_ROOT}/config/openiotsdk REALPATH)
get_filename_component(OPEN_IOT_SDK_EXAMPLE_COMMON ${CHIP_ROOT}/examples/platform/openiotsdk REALPATH)

list(APPEND CMAKE_MODULE_PATH ${OPEN_IOT_SDK_CONFIG}/cmake)

# Application CHIP build configuration
set(CONFIG_CHIP_LIB_TESTS YES)
set(CONFIG_CHIP_DETAIL_LOGGING NO)
set(CONFIG_CHIP_PROGRESS_LOGGING NO)
set(CONFIG_CHIP_AUTOMATION_LOGGING YES)
set(CONFIG_CHIP_ERROR_LOGGING NO)
set(CONFIG_CHIP_FORCE_LOGGING_STDIO YES)

# Toolchain files need to exist before first call to project
include(toolchain)

project(chip-open-iot-sdk-unit-tests LANGUAGES C CXX ASM)


include(sdk)
include(chip)
include(linker)

add_subdirectory(${OPEN_IOT_SDK_EXAMPLE_COMMON}/app ./app_build)

file(STRINGS test_components.txt TEST_NAMES_FROM_FILE)


target_compile_definitions(openiotsdk-startup
    PRIVATE
        IOT_SDK_APP_MAIN_STACK_SIZE=20480
)

foreach(TEST_NAME IN LISTS TEST_NAMES_FROM_FILE)
    set(APP_TARGET ${TEST_NAME}_ns)
    add_executable(${APP_TARGET})
    target_include_directories(${APP_TARGET}
        PRIVATE
        main/include
    )

    target_sources(${APP_TARGET}
        PRIVATE
            main/main_ns.cpp
    )

    target_link_libraries(${APP_TARGET}
        openiotsdk-startup
        openiotsdk-app
    )

    # Link the *whole-archives* to keep the static test objects.
    target_link_options(${APP_TARGET}
        PUBLIC
            -Wl,--whole-archive "${CMAKE_CURRENT_BINARY_DIR}/chip_build/lib/lib${TEST_NAME}.a"
            "${CMAKE_CURRENT_BINARY_DIR}/chip_build/lib/libPWTestsWrapper.a"
            -Wl,--no-whole-archive)

    set_target_link(${APP_TARGET})
    sdk_post_build(${APP_TARGET})
endforeach()
