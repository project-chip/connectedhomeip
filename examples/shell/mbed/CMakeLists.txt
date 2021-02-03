# Copyright (c) 2021 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19.0)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)
get_filename_component(APP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/.. REALPATH)

set(MBED_PATH $ENV{MBED_OS_PATH} CACHE INTERNAL "")
set(MBED_CONFIG_PATH ${CMAKE_CURRENT_SOURCE_DIR}/.mbedbuild CACHE INTERNAL "")
set(CONFIG_CHIP_LIB_SHELL 1)
set(APP_TARGET chip-mbed-shell-example)

include(${MBED_PATH}/tools/cmake/app.cmake)
add_subdirectory(${MBED_PATH} ./mbed_build)

add_subdirectory(${CHIP_ROOT}/config/mbed ./chip_build)


add_executable(${APP_TARGET})

target_include_directories(${APP_TARGET} PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${APP_ROOT}/shell_common/include)

# FIXME: These includes path should be propagated by chip 
target_include_directories(${APP_TARGET} PRIVATE 
    ${CHIP_ROOT}/third_party/lwip/repo/lwip/src/include
    ${CHIP_ROOT}/src/lwip/mbed
)

# FIXME: These defines should be propagated by chip 
target_compile_definitions(${APP_TARGET} PRIVATE
    "LWIP_NO_STDINT_H=1" 
    "IN_ADDR_T_DEFINED=1"
)

target_sources(${APP_TARGET} PRIVATE
    ${APP_ROOT}/shell_common/cmd_base64.cpp
    ${APP_ROOT}/shell_common/cmd_device.cpp
    ${APP_ROOT}/shell_common/cmd_misc.cpp
    ${APP_ROOT}/shell_common/cmd_otcli.cpp
    ${APP_ROOT}/shell_common/cmd_btp.cpp
    ${APP_ROOT}/standalone/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/arch.c
)

mbed_configure_app_target(${APP_TARGET})

mbed_set_mbed_target_linker_script(${APP_TARGET})

project(${APP_TARGET})

target_link_libraries(${APP_TARGET} mbed-os chip)

mbed_set_post_build(${APP_TARGET})

option(VERBOSE_BUILD "Have a verbose build process")
if(VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()