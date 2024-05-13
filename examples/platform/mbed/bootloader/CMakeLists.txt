# Copyright (c) 2021 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19.0 FATAL_ERROR)

set(MBED_PATH ${MBED_OS_PATH} CACHE INTERNAL "")
set(MCUBOOT_PATH ${MBED_MCU_BOOT_PATH} CACHE INTERNAL "")
set(MBED_CONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
set(APP_TARGET chip-mbed-bootloader)

include(${MBED_PATH}/tools/cmake/app.cmake)

project(${APP_TARGET})

add_subdirectory(${MBED_PATH} ./mbed_build)
add_subdirectory(${MCUBOOT_PATH}/boot/bootutil/ ./mbed_mcu_boot_util)
add_subdirectory(${MCUBOOT_PATH}/boot/mbed/ ./mbed_mcu_boot)  # Mbed-MCUboot Port

add_executable(${APP_TARGET})

target_sources(${APP_TARGET}
    PUBLIC
        default_bd.cpp
        signing_keys.c
)

target_link_libraries(${APP_TARGET}
    PUBLIC
        bootutil
        mbed-mcuboot
        mbed-storage-spif
        mbed-storage-qspif
        mbed-baremetal
)

mbed_set_post_build(${APP_TARGET})

option(VERBOSE_BUILD "Have a verbose build process")
if(VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
