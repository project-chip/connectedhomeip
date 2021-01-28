# Copyright (c) 2021 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19.0)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/connectedhomeip REALPATH)
get_filename_component(APP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/.. REALPATH)

set(MBED_PATH $ENV{MBED_OS_PATH} CACHE INTERNAL "")
set(MBED_CONFIG_PATH ${CMAKE_CURRENT_SOURCE_DIR}/.mbedbuild CACHE INTERNAL "")
set(APP_TARGET chip-mbed-shell-example)

add_subdirectory(${CHIP_ROOT}/config/mbed)

include(${MBED_PATH}/tools/cmake/app.cmake)

add_subdirectory(${MBED_PATH})

add_executable(${APP_TARGET}
    main.cpp
)

mbed_configure_app_target(${APP_TARGET})

mbed_set_mbed_target_linker_script(${APP_TARGET})

project(${APP_TARGET})

target_link_libraries(${APP_TARGET} mbed-os)
target_link_libraries(${APP_TARGET} PUBLIC chip)

mbed_set_post_build(${APP_TARGET})

option(VERBOSE_BUILD "Have a verbose build process")
if(VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()